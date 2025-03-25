#include <WiFi.h>
#include <WiFiClient.h>
#include <ESPAsyncWebServer.h>
#include "esp_camera.h"
#include <ArduinoHttpClient.h>
#include <mbedtls/base64.h>

#define RESPONSE_PIN 12   // GPIO to notify success

const char* ssid = "AndroidAP";
const char* password = "2020not2021";
const char* serverAddress = "192.168.43.179";  // Flask server IP
const int serverPort = 5000;

int imageCount = 0;
unsigned long lastCaptureTime = 0;
const int captureInterval = 3000;  // 3 seconds

AsyncWebServer server(80);
String latestImageBase64;

// Camera Configuration
void startCamera() {
    camera_config_t config;
    config.ledc_channel = LEDC_CHANNEL_0;
    config.ledc_timer = LEDC_TIMER_0;
    config.pin_d0 = 5;
    config.pin_d1 = 18;
    config.pin_d2 = 19;
    config.pin_d3 = 21;
    config.pin_d4 = 36;
    config.pin_d5 = 39;
    config.pin_d6 = 34;
    config.pin_d7 = 35;
    config.pin_xclk = 0;
    config.pin_pclk = 22;
    config.pin_vsync = 25;
    config.pin_href = 23;
    config.pin_sscb_sda = 26;
    config.pin_sscb_scl = 27;
    config.pin_pwdn = -1;
    config.pin_reset = -1;
    config.xclk_freq_hz = 20000000;
    config.pixel_format = PIXFORMAT_JPEG;

    if (psramFound()) {
        config.frame_size = FRAMESIZE_QVGA;
        config.jpeg_quality = 12;  
        config.fb_count = 2;
    } else {
        config.frame_size = FRAMESIZE_CIF;
        config.jpeg_quality = 15;
        config.fb_count = 1;
    }

    esp_err_t err = esp_camera_init(&config);
    if (err != ESP_OK) {
        Serial.println("Camera init failed");
        return;
    }
}

// Function to take a picture and convert it to base64
String captureImage() {
    camera_fb_t* fb = esp_camera_fb_get();
    if (!fb) {
        Serial.println("Camera capture failed");
        return "";
    }

    String imageData = encodeToBase64(fb->buf, fb->len);

    esp_camera_fb_return(fb);
    return imageData;
}

// Function to send image to Flask server
bool sendImage(String base64Image) {
    WiFiClient client;
    HttpClient http(client, serverAddress, serverPort);

    http.beginRequest();
    http.post("/upload");
    http.sendHeader("Content-Type", "application/json");
    http.sendHeader("Content-Length", base64Image.length());
    http.beginBody();
    http.print("{\"image\": \"" + base64Image + "\"}");
    http.endRequest();

    int statusCode = http.responseStatusCode();
    String response = http.responseBody();
    
    if (statusCode == 200 && response.indexOf("\"success\": true") > 0) {
        Serial.println("Plate matched!");
        digitalWrite(RESPONSE_PIN, HIGH);
        return true;
    }

    digitalWrite(RESPONSE_PIN, LOW);
    return false;
}

// Web page to display the latest image
const char* htmlPage = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
    <title>ESP32-CAM Live</title>
    <meta http-equiv="refresh" content="3"> 
</head>
<body>
    <h2>Latest Image</h2>
    <img src="data:image/jpeg;base64,%IMAGE%" width="320"/>
</body>
</html>
)rawliteral";

// Replaces placeholder in web page with latest base64 image
String processor(const String& var) {
    if (var == "IMAGE") {
        return latestImageBase64;
    }
    return "";
}

void setup() {
    Serial.begin(115200);
    WiFi.begin(ssid, password);
    pinMode(RESPONSE_PIN, OUTPUT);
    digitalWrite(RESPONSE_PIN, LOW);

    while (WiFi.status() != WL_CONNECTED) {
        delay(1000);
        Serial.println("Connecting to WiFi...");
    }

    startCamera();

    // Web server to serve the latest image
    server.on("/", HTTP_GET, [](AsyncWebServerRequest* request) {
        String page = htmlPage;
        page.replace("%IMAGE%", latestImageBase64);
        request->send(200, "text/html", page);
    });

    server.begin();
}

void loop() {
    unsigned long currentMillis = millis();

    if (currentMillis - lastCaptureTime >= captureInterval) {
        lastCaptureTime = currentMillis;

        String imageBase64 = captureImage();
        if (!imageBase64.isEmpty()) {
            latestImageBase64 = imageBase64;
            if (sendImage(imageBase64)) {
                imageCount++;
            }
        }
    }

    delay(10);  // Prevents watchdog timer reset
}


String encodeToBase64(uint8_t *data, size_t len) {
    size_t encodedLen = 0;
    mbedtls_base64_encode(NULL, 0, &encodedLen, data, len);  // Get required length

    uint8_t *encodedData = (uint8_t *)malloc(encodedLen);
    if (!encodedData) {
        Serial.println("Memory allocation failed for Base64 encoding");
        return "";
    }

    if (mbedtls_base64_encode(encodedData, encodedLen, &encodedLen, data, len) != 0) {
        Serial.println("Base64 encoding failed");
        free(encodedData);
        return "";
    }

    String encodedStr = (char *)encodedData;
    free(encodedData);
    return encodedStr;
}



