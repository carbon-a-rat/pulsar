#include <Arduino.h>

#include <pulsar/callback.hpp>
#include <pulsar/protocol.hpp>
#if defined(ESP32)
#include <HTTPClient.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#endif

#include <pulsar/config.hpp>

#include <pocketbase.hpp>
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
  Serial.println("Public IP: ");
  HTTPClient http;
  http.begin("http://api.ipify.org");
  int httpCode = http.GET();
  if (httpCode > 0) {
    String payload = http.getString();
    Serial.println(payload);
  } else {
    Serial.printf("Error getting public IP: %s\n", http.errorToString(httpCode).c_str());
  }
  http.end();

  // Test connectivity to Pocketbase server
  Serial.println("Testing connectivity to Pocketbase server...");
  HTTPClient pbTest;
  WiFiClientSecure client;
  client.setInsecure();
  pbTest.begin(client, "https://deathstar.cyp.sh/api/health");
  pbTest.setTimeout(10000);
  int pbHttpCode = pbTest.GET();
  Serial.printf("Pocketbase health check response: %d\n", pbHttpCode);
  if (pbHttpCode > 0) {
    String pbResponse = pbTest.getString();
    Serial.println("Pocketbase response: " + pbResponse);
  } else {
    Serial.printf("Error connecting to Pocketbase: %s\n", pbTest.errorToString(pbHttpCode).c_str());
  }
  pbTest.end();
  client.stop();

  pb.login_passwd(USER_NAME, USER_PASSWORD, "launchers");


  String request = 
    String("(should_load=true&&loaded_at=\"\"&&launcher=\"") + pb.getConnectionRecord()["record"]["id"].as<String>() + "\")";
  String result = pb.collection("launches").getList(
    "1", // page
    "20", // perPage
    nullptr, // sort 
    request.c_str() // filter
  );  
  
  if (result.isEmpty()) {
    Serial.println("No launches found or error fetching launches.");
  }
  
  Serial.println("Fetched launches: ");
  Serial.println(result);

  

  Serial.println("Result: ");
  Serial.println(result);

//  pb.subscribe("launches", "*", [](String event, String record, void *ctx) {
//    Serial.printf("Event: %s, Record: %s\n", event.c_str(), record.c_str());
//  });
  pb.subscribe("launches", "*", [](String event, String record, void *ctx) {
    Serial.printf("Event: %s, Record: %s\n", event.c_str(), record.c_str());
  });
  Serial.println("finished loading.");
}

void loop() {
 // Serial.println("Updating subscription...");
  pb.update_subscription();
}
