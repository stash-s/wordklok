#include <Arduino.h>
#include <ArduinoOTA.h>
#include <NTPClient.h>
#include <WiFiManager.h>

#include "display.h"
#include "light_sensor.h"

WiFiManager wifiManager;

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "europe.pool.ntp.org", 2 * 3600, 236 * 1000);
String ntpPoolServerName;

WordKlokDisplay display;

LightSensor lightSensor;

bool downloadInProgress = false;

void setup() {

    pinMode(LED_BUILTIN, OUTPUT);
    digitalWrite(LED_BUILTIN, LOW);

    display.init();

    // wifiManager.resetSettings();
    Serial.begin(9600);


    display.startAnimation();

    wifiManager.setAPCallback(
        [](WiFiManager *mgr) { });
    wifiManager.setConfigPortalTimeout(180);

    Serial.println("connecting...");
    if (wifiManager.autoConnect("AutoconnectAP")) {
        Serial.println("connected ... yay!");
        ntpPoolServerName = WiFi.gatewayIP().toString();
        timeClient.setPoolServerName(ntpPoolServerName.c_str());
    } else {
        Serial.println("connection failed, rebooting");
        ESP.restart();
    }

    display.endAnimation();

    timeClient.onStartUpdate([]() {
        Serial.println("NTP Update started ....");
        display.startAnimation();
    });
    timeClient.onEndUpdate([]() {
        Serial.println("NTP update finished.");
        display.endAnimation();
    });
    timeClient.begin();

    ArduinoOTA.onStart([]() {
        String type;
        if (ArduinoOTA.getCommand() == U_FLASH) {
            type = "sketch";
        } else { // U_SPIFFS
            type = "filesystem";
        }

        // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS
        // using SPIFFS.end()
        Serial.println("Start updating " + type);
    });

    ArduinoOTA.onStart([]() { downloadInProgress = true; });
    ArduinoOTA.onEnd([]() { display.stop(); });
    ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
        auto progressBar = map(progress, 0, total, 0, 7);
        Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
        display.setScrollBar(progressBar);
    });
    ArduinoOTA.onError([](ota_error_t error) {
        Serial.printf("Error[%u]: ", error);
        if (error == OTA_AUTH_ERROR) {
            Serial.println("Auth Failed");
        } else if (error == OTA_BEGIN_ERROR) {
            Serial.println("Begin Failed");
        } else if (error == OTA_CONNECT_ERROR) {
            Serial.println("Connect Failed");
        } else if (error == OTA_RECEIVE_ERROR) {
            Serial.println("Receive Failed");
        } else if (error == OTA_END_ERROR) {
            Serial.println("End Failed");
        }
    });
    ArduinoOTA.begin();

    lightSensor.setFrequency(1);
    lightSensor.onLevelSet([](int level) {
        Serial.printf("Light level set to %d\n", level);
        display.setLightLevel(level);
    });

    // lightSensor.onReading([](int reading) {
    //    Serial.printf("Light level reading %d\n", reading);
    //});

    digitalWrite(LED_BUILTIN, HIGH);
}

void loop() {
    ArduinoOTA.handle();

    static int ntpFailCount = 0;

    if (!downloadInProgress) {

        if (timeClient.update()) {
            ntpFailCount = 0;

            display.showTime(timeClient.getHours(), timeClient.getMinutes());

        } else {
            ++ntpFailCount;
            Serial.println("NTP update failed");

            if (ntpFailCount > 50) {
                Serial.println("Number of failures exceeded limit, rebooting");
                ESP.restart();
            }
        }
    }

    lightSensor.handle();
}