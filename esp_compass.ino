#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClientSecure.h>
#include <ArduinoJson.h>
#include <SoftwareSerial.h>
#include <TinyGPS++.h>
#include <Wire.h>
#include <QMC5883LCompass.h>
#include <Servo.h>



Servo arrowServo;

// --- USER CONFIGURATION ---
const char* ssid = "YOUR_WIFI_SSID";
const char* password = "YOUR_WIFI_PASSWORD";
String GOOGLE_API_KEY = "YOUR_GOOGLE_PLACES_API_KEY";

// --- OBJECTS ---
TinyGPSPlus gps;
SoftwareSerial ss(D3, D4); // GPS RX=D3, TX=D4
QMC5883LCompass compass;

// --- TARGET MEMORY ---
double targetLat = 0.0;
double targetLon = 0.0;
bool targetFound = false;
String targetName = "";

void setup() {
  Serial.begin(115200);
  ss.begin(9600);

  // 1. Initialize and CALIBRATE Compass
  Wire.begin(D2, D1); 
  compass.init();
  // Here are your exact calibration numbers!
  compass.setCalibrationOffsets(-744.50, 232.50, 2026.00);
  compass.setCalibrationScales(0.95, 0.83, 1.30);
  Serial.println("\nCompass Calibrated!");

  // Initialize Servo on pin D5
  arrowServo.attach(D5, 500, 2500);
  arrowServo.write(0); // Point straight ahead to start
  Serial.println("Servo Attached!");

  // 2. Connect to Wi-Fi
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWi-Fi Connected!");
}

void loop() {
  // 1. Read GPS Data constantly
  while (ss.available() > 0) {
    gps.encode(ss.read());
  }

  // 2. Find the Pub (Runs once when GPS gets a lock)
  if (gps.location.isValid() && !targetFound) {
    float myLat = gps.location.lat();
    float myLon = gps.location.lng();
    Serial.print("GPS Locked! My Loc: "); Serial.print(myLat, 6); Serial.print(", "); Serial.println(myLon, 6);
    
    findNearestBar(myLat, myLon);
  }

  // 3. The Navigation Magic! (Runs continuously once target is found)
  if (targetFound && gps.location.isValid()) {
    
    // A. Read current location
    float myLat = gps.location.lat();
    float myLon = gps.location.lng();

    // B. Read which way we are physically facing
    compass.read();
    int myHeading = compass.getAzimuth(); 
    if (myHeading < 0) myHeading += 360; // Fix -180/180 scale to 0-360

    // C. Calculate the exact direction to the pub
    int targetHeading = gps.courseTo(myLat, myLon, targetLat, targetLon);
    
    // D. Calculate Distance
    double distance = gps.distanceBetween(myLat, myLon, targetLat, targetLon);

    // --- DISPLAY OUTPUT ---
    Serial.println("----------------------------------------");
    Serial.print("TARGET: "); Serial.println(targetName);
    Serial.print("DISTANCE: "); Serial.print(distance); Serial.println(" meters");
    
    Serial.print("PUB IS AT: "); Serial.print(targetHeading); Serial.println(" degrees");
    Serial.print("IM FACING: "); Serial.print(myHeading); Serial.println(" degrees");
    
    // E. Tell the user which way to turn
    int difference = targetHeading - myHeading;
    int servoAngle = 0;

    

    if (difference > 180) {
      difference -= 360; 
    } else if (difference < -180) {
      difference += 360;
    }

    

    if (abs(difference) < 90){
        servoAngle = 90 + difference;
    }
    else if (difference < -90)
        servoAngle = 0;
    else if (difference > 90)
        servoAngle = 180;

    arrowServo.write(servoAngle); 
    
    
    delay(1000); // Update once a second
  }
}

// --- GOOGLE API FUNCTION ---
void findNearestBar(float lat, float lon) {
  if (WiFi.status() == WL_CONNECTED) {
    WiFiClientSecure client;
    client.setInsecure();
    HTTPClient http;

    String url = "https://places.googleapis.com/v1/places:searchNearby";
    http.begin(client, url);
    
    http.addHeader("Content-Type", "application/json");
    http.addHeader("X-Goog-Api-Key", GOOGLE_API_KEY);
    http.addHeader("X-Goog-FieldMask", "places.displayName,places.location");

    String jsonPayload = "{";
    jsonPayload += "\"includedTypes\": [\"restaurant\"],"; 
    jsonPayload += "\"maxResultCount\": 1,";
    jsonPayload += "\"rankPreference\": \"DISTANCE\","; 
    jsonPayload += "\"locationRestriction\": {";
    jsonPayload += "  \"circle\": {";
    jsonPayload += "    \"center\": {";
    jsonPayload += "      \"latitude\": " + String(lat, 6) + ",";
    jsonPayload += "      \"longitude\": " + String(lon, 6);
    jsonPayload += "    },";
    jsonPayload += "    \"radius\": 5000.0";
    jsonPayload += "  }";
    jsonPayload += "}";
    jsonPayload += "}";

    Serial.println("Asking Google...");
    int httpCode = http.POST(jsonPayload); 

    if (httpCode > 0) {
      String payload = http.getString();
      DynamicJsonDocument doc(4096); 
      deserializeJson(doc, payload);

      if (doc.containsKey("places")) {
         targetName = doc["places"][0]["displayName"]["text"].as<String>();
         targetLat = doc["places"][0]["location"]["latitude"];
         targetLon = doc["places"][0]["location"]["longitude"];
         targetFound = true; 
         
         Serial.println("-----------------------------");
         Serial.print("TARGET ACQUIRED: "); Serial.println(targetName);
         Serial.println("-----------------------------");
      } else {
         Serial.println("No places found.");
      }
    }
    http.end();
  }
}
