#include <Arduino.h>
#include <WiFi.h>
#include <WireGuard-ESP32.h>

#include "secrets.h"

WiFiServer server(80);
static WireGuard wg;
IPAddress localIp(10, 200, 200, 2);
unsigned long lastTime = 0;
unsigned long blinkStart = 0;

unsigned long visitors = 0;
unsigned long illuminations = 0;

void setup() {
    Serial.begin(115200);

    pinMode(A2, OUTPUT);
    digitalWrite(A2, HIGH);

    delay(3000);

    digitalWrite(A2, LOW);

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

    digitalWrite(A2, millis() - blinkStart < 250 ? HIGH : LOW);

    if (client) {
        Serial.println("New client connected");

        String header = "";
        String currentLine = "";
        unsigned long currentTime = millis();

        lastTime = currentTime;

        while (client.connected() && currentTime - lastTime <= 2000) {
            currentTime = millis();

            if (client.available()) {
                char c = client.read();

                header += c;

                if (c == '\n') {
                    if (currentLine.length() == 0) {
                        Serial.println(header);

                        if (header.indexOf("dotheilluminationthingy") >= 0) {
                            Serial.println("Blink initiated");

                            blinkStart = millis();
                            illuminations++;
                        } else {
                            visitors++;
                        }

                        Serial.println("Received header; sending response");

                        client.println("HTTP/1.1 200 OK");
                        client.println("Content-Type: text/html");
                        client.println("Connection: close");
                        client.println();

                        client.printf(
                            "<!DOCTYPE html>"
                            "<h1>Hello, world!</h1>"
                            "<p>If you can read this, you have connected to a random ESP32 somewhere on the internet!</p>"
                            "<p>Please mail all correspondence (greetings; praise; complaints; death threats) to:</p>"
                            "<address>James Livesey<br>Null Island<br>Gulf of Guinea</address>"
                            "<p>NO JUNK MAIL</p>"
                            "<hr>"
                            "<p>Alternatively, you may send greetings, praise, complaints and death threats using Morse code, which will illuminate an LED on Mr Livesey's desk. Visual confirmation is provided below.</p>"
                            "<iframe name='thisissillywhydoyouhavetodoitlikethis' id='thisissillywhydoyouhavetodoitlikethis' style='display: none;'></iframe>"
                            "<form action='/dotheilluminationthingy' method='POST' target='thisissillywhydoyouhavetodoitlikethis'>"
                                "<button>ILLUMINATE</button>"
                            "</form>"
                            "<br>"
                            "<iframe width='560' height='315' src='https://www.youtube.com/embed/bf5_kjDBlzA?si=b6wUdYfvAR3rjC6A' title='YouTube video player' frameborder='0' allow='accelerometer; autoplay; clipboard-write; encrypted-media; gyroscope; picture-in-picture; web-share' referrerpolicy='strict-origin-when-cross-origin' allowfullscreen></iframe>"
                            "<p><em>You are visitor number %06d. The button has been clicked %d times before you even bothered visiting this site.</em></p>",
                            visitors, illuminations
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