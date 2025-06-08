#include <Arduino.h>

#include <pulsar/callback.hpp>
#include <pulsar/protocol.hpp>
#if defined(ESP32)
#include <HTTPClient.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#endif

#include <pulsar/config.hpp>

#include <pulsar/pocketbase.hpp>

PocketbaseArduino pb("https://deathstar.cyp.sh/");

void setup(){
  Serial.begin(9600);
  // list available wifi 
  Serial.println("Available WiFi networks:");
  int n = WiFi.scanNetworks();
  for (int i = 0; i < n; ++i) {
    Serial.printf("%d: %s (%d dBm)\n", i + 1, WiFi.SSID(i).c_str(), WiFi.RSSI(i));
  }
  if (n == 0) {
    Serial.println("No networks found");
  } else {
    Serial.printf("%d networks found\n", n);
  }
  // Connect to a WiFi network
  // yeah I leak my wifi password, and it is weird, it's a private joke 
  // sorry for this beautiful showcase of professionalism.

  WiFi.begin("Van FBI", "bosspicasso");
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }




  Serial.println("\nConnected to WiFi");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  pb.login_passwd(USER_NAME, USER_PASSWORD);


  String result = pb.collection("manufacturers").getList();

  Serial.println("Result: ");
  Serial.println(result);


  pb.subscribe("manufacturers", "*", [](String event, String record, void *ctx) {
    Serial.printf("Event: %s, Record: %s\n", event.c_str(), record.c_str());
  });
  Serial.println("finished loading.");
}

void loop() {
  pb.update_subscription();
}
