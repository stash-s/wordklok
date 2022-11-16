#include <Arduino.h>
#include <ArduinoOTA.h>
#include <ezTime.h>

#include <ESPAsyncWebServer.h> //Local WebServer used to serve the configuration portal
#include <ESPAsyncWiFiManager.h> //https://github.com/tzapu/WiFiManager WiFi Configuration Magic

#include "display.h"
#include "light_sensor.h"

AsyncWebServer server(80);
DNSServer dns;

Timezone poland;

WordKlokDisplay display;

LightSensor lightSensor;

bool downloadInProgress = false;

void setup() {

    //ezt::setDebug(DEBUG);

    pinMode(LED_BUILTIN, OUTPUT);
    digitalWrite(LED_BUILTIN, LOW);

    display.init();

    // wifiManager.resetSettings();
    Serial.begin(9600);


    display.startAnimation();

    AsyncWiFiManager wifiManager(&server, &dns);

    //wifiManager.setAPCallback(
    //    [](WiFiManager *mgr) { });
    wifiManager.setConfigPortalTimeout(180);

    // exit after config instead of connecting
    wifiManager.setBreakAfterConfig(true);

    Serial.println("connecting...");
    if (wifiManager.autoConnect("AutoconnectAP")) {
        Serial.println("connected ... yay!");
    } else {
        Serial.println("connection failed, rebooting");
        ESP.restart();
    }

    display.endAnimation();

    if (!poland.setCache(0)) {
        poland.setLocation("Europe/Warsaw");
    }

    ezt::onNtpUpdateStart([]() {
        Serial.println("NTP Update started ....");
        display.startAnimation();
    });
    ezt::onNtpUpdateEnd([](bool result) {
        Serial.println("NTP update finished.");
        display.endAnimation();
    });

    ezt::waitForSync();

    ArduinoOTA.onStart([]() {
        String type;
        if (ArduinoOTA.getCommand() == U_FLASH) {
            type = "sketch";
        } else { // U_SPIFFS
            type = "filesystem";
        }

        // NOTE: if updating SPIFFS this would be the place to unmount
        // SPIFFS using SPIFFS.end()
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


    if (!downloadInProgress) {
        ezt::events();
        display.showTime(poland.hour(), poland.minute());
    }

    static auto last_call = 0;

    auto current_time = millis();
    if (current_time - last_call >= 10000) {
        last_call = current_time;

        Serial.print("curent time: ");
        Serial.println(poland.dateTime());
    }

    lightSensor.handle();
}