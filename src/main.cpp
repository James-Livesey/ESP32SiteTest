#include <Arduino.h>
#include <WiFi.h>
#include <WireGuard-ESP32.h>

#include "secrets.h"

WiFiServer server(80);
static WireGuard wg;

IPAddress localIp(10, 200, 200, 2);

void setup() {
    Serial.begin(115200);

    delay(3000);

    esp_log_level_set("*", ESP_LOG_INFO);

    Serial.println("Hello, world!");
    Serial.printf("Connecting to %s...\r\n", WIFI_SSID);

    log_i("Log level set correctly!");

    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

    while (WiFi.status() != WL_CONNECTED) {
        delay(1000);
        Serial.println("Still connecting...");
    }

    Serial.println("Wi-Fi connected successfully!");
    Serial.printf("IP address: %s\r\n", WiFi.localIP().toString());

    configTime(9 * 60 * 60, 0, "0.uk.pool.ntp.org", "1.uk.pool.ntp.org", "2.uk.pool.ntp.org");

    Serial.println("Configured NTP servers");

    if (wg.begin(
        localIp,
        PRIVATE_KEY,
        ENDPOINT_ADDRESS,
        PUBLIC_KEY,
        ENDPOINT_PORT
    )) {
        Serial.println("Connected via WireGuard");
    } else {
        Serial.println("Failed to connect via WireGuard");
    }

    server.begin();

    Serial.println("Server started");
}

void loop() {
    WiFiClient client = server.available();

    if (client) {
        Serial.println("New client connected");

        while (client.connected()) {
            String header = "";
            String currentLine = "";

            if (client.available()) {
                char c = client.read();

                header += c;

                if (c == '\n') {
                    if (currentLine.length() == 0) {
                        Serial.println("Received header; sending response");

                        client.println("HTTP/1.1 200 OK");
                        client.println("Content-Type: text/html");
                        client.println("Connection: close");
                        client.println();

                        client.println(
                            "<!DOCTYPE html>"
                            "<h1>Hello, world!</h1>"
                            "<p>If you can read this, you have connected to a random ESP32 somewhere on the internet!</p>"
                            "<p>Please mail all correspondence (greetings; praise; complaints; death threats) to:</p>"
                            "<address>James Livesey<br>Null Island<br>Gulf of Guinea</address>"
                            "<p>NO JUNK MAIL</p>"
                        );

                        client.println();

                        Serial.println("Sent response");

                        break;
                    }

                    currentLine = "";
                } else if (c != '\r') {
                    currentLine += c;
                }
            }
        }

        client.stop();

        Serial.println("Client disconnected");
    }
}