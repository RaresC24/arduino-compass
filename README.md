<img width="2040" height="1524" alt="Gemini_Generated_Image_jc04k0jc04k0jc04" src="https://github.com/user-attachments/assets/da7af608-ff35-47de-9dd4-1f433b8d4f54" />

![WhatsApp Image 2026-04-14 at 1 56 46 PM](https://github.com/user-attachments/assets/ebac69a5-e603-4a77-8494-deb66d14242a)

# ESP Compass

An ESP8266-based project that acts as an automated compass pointing you to the nearest restaurant or pub.

It uses a GPS module to obtain your current location, queries the **Google Places API** to find the closest restaurant, and then calculates the distance and heading. Using a **QMC5883L Compass** module to determine your current orientation, a **Servo Motor** then rotates to physically point in the direction of the destination!

## Features
- **GPS tracking** to determine your current coordinates.
- **REST API Call** to the Google Places API to find the nearest "restaurant" (within 5km).
- **QMC5883L compass reading** to figure out which way you are currently facing.
- **Servo-controlled arrow** indicating the real-world direction to turn.

## Hardware Used
- **ESP8266** (NodeMCU or similar)
- **GPS Module** (connected via SoftwareSerial, RX=D3, TX=D4)
- **QMC5883L** Compass Module (connected via I2C, SDA=D2, SCL=D1)
- **Servo Motor** (signal on D5)

## Dependencies
You will need to install the following libraries in the Arduino IDE:
- [ArduinoJson](https://arduinojson.org/)
- [TinyGPS++](https://github.com/mikalhart/TinyGPSPlus)
- [QMC5883LCompass](https://github.com/mprograms/QMC5883LCompass)
- *Built-in or standard libraries:* `ESP8266WiFi`, `ESP8266HTTPClient`, `WiFiClientSecure`, `SoftwareSerial`, `Wire`, `Servo`

## Setup & Configuration
1. Open the `.ino` file in the Arduino IDE.
2. Under the `// --- USER CONFIGURATION ---` section, update the placeholder values:
   ```cpp
   const char* ssid = "YOUR_WIFI_SSID";
   const char* password = "YOUR_WIFI_PASSWORD";
   String GOOGLE_API_KEY = "YOUR_GOOGLE_PLACES_API_KEY";
   ```
   **Note:** You must generate a Google API Key with the **Places API (New)** enabled to get nearby locations.
3. Verify that your I2C pins (`D2`, `D1`), SoftwareSerial pins (`D3`, `D4`), and Servo pin (`D5`) match your hardware setup. By default, the Sketch is configured as:
   - GPS RX -> `D3`, TX -> `D4`
   - Compass SDA -> `D2`, SCL -> `D1`
   - Servo -> `D5`
4. If your compass behaves incorrectly, you may need to update the calibration values in the `setup()` block.

## How It Works
- Upon boot, it calibrates the compass and connects to your specified WiFi.
- It constantly listens to the GPS module until a lock is obtained.
- It then makes an HTTPS request to `https://places.googleapis.com/v1/places:searchNearby` with your exact GPS coordinates.
- Once a target is found, it calculates the heading towards it, compares it with your current compass reading, and moves the servo to point ahead, left, right, or behind!
