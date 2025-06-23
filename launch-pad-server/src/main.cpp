#include <Arduino.h>

#include <pulsar/callback.hpp>
#include <pulsar/protocol.hpp>
#include <pulsar/sync.hpp>
#if defined(ESP32)
#include <HTTPClient.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#endif

#include <pulsar/config.hpp>

#include <pocketbase.hpp>
PocketbaseArduino pb("https://deathstar.cyp.sh/");

void wifi_callback(WiFiEvent_t event)
{
  switch (event)
  {
  case SYSTEM_EVENT_STA_GOT_IP:
    Serial.printf("[Wifi] Got IP: %s\n", WiFi.localIP().toString().c_str());
    break;
  case SYSTEM_EVENT_STA_DISCONNECTED:
    Serial.println("[Wifi] Disconnected from WiFi");
    break;
  case SYSTEM_EVENT_STA_CONNECTED:
    Serial.println("[Wifi] Connected to WiFi");
    break;
  case SYSTEM_EVENT_AP_START:
    Serial.println("[Wifi] Access Point started");
    break;
  case SYSTEM_EVENT_AP_STOP:
    Serial.println("[Wifi] Access Point stopped");
    break;
  default:
    Serial.println("[Wifi] Unhandled WiFi event: " + String(event));

    break;
  }
}


void update_online()
{
  auto& rec = pb.getConnectionRecord(); 
  rec["record"]["online"] = true;
  rec["record"]["last_ping_at"] = current_iso_time();
  String body = rec["record"].as<String>();
  Serial.println("Updating online status: " + body);
  pb.update("launchers", rec["record"]["id"].as<String>(), body);
}

long online_ping = 0;
void setup()
{
  Serial.begin(9600);

  //CONFIG_LWIP_MAX_SOCKETS
  
  setup_communication_slave(21, 22, SERVER_S_ADDRESS); // SDA=21, SCL=22 for ESP32
  Wire.begin(SERVER_S_ADDRESS);
  Serial.print("I2C Slave initialized at address: 0x");
  Serial.println(SERVER_S_ADDRESS, HEX);

  // list available wifi
  Serial.println("Available WiFi networks:");
  int n = WiFi.scanNetworks();
  for (int i = 0; i < n; ++i)
  {
    Serial.printf("%d: %s (%d dBm)\n", i + 1, WiFi.SSID(i).c_str(), WiFi.RSSI(i));
  }
  if (n == 0)
  {
    Serial.println("No networks found");
  }
  else
  {
    Serial.printf("%d networks found\n", n);
  }
  // Connect to a WiFi network
  // yeah I leak my wifi password, and it is weird, it's a private joke
  // sorry for this beautiful showcase of professionalism.
  for (size_t i = 0; i < WiFiEvent_t::ARDUINO_EVENT_MAX; i++)
  {

    WiFi.onEvent(wifi_callback, static_cast<arduino_event_id_t>(i));
  }
  // WiFi.begin("iPhone Emma", "biup8vd4ht8w1");
  WiFi.begin("Van FBI", "bosspicasso");
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nConnected to WiFi");

  time_sync_init();

  Serial.println("Time synchronized");
  Serial.println("Current time: " + String(current_iso_time()));
  
  pb.login_passwd(USER_NAME, USER_PASSWORD, "launchers");
  update_online(); 
  //String request =
  //    String("(should_load=true&&loaded_at=\"\"&&launcher=\"") + pb.getConnectionRecord()["record"]["id"].as<String>() + "\")";
  //String result = pb.collection("launches").getList("1",            // page
  //                                                  "20",           // perPage
  //                                                  nullptr,        // sort
  //                                                  request.c_str() // filter
  //);
//
  //if (result.isEmpty())
  //{
  //  Serial.println("No launches found or error fetching launches.");
  //}
//
  //Serial.println("Result: ");
  //Serial.println(result);

  //  pb.subscribe("launches", "*", [](String event, String record, void *ctx) {
  //    Serial.printf("Event: %s, Record: %s\n", event.c_str(), record.c_str());
  //  });
  pb.subscribe("launches", "*", [](SubscriptionEvent &ev, void *ctx)
    {
      Serial.printf("Event: %s, Record: %s\n", ev.event.c_str(), ev.data.c_str());
      JsonDocument doc;
      DeserializationError error = deserializeJson(doc, ev.data);
      if (error)
      {
        Serial.printf("Failed to deserialize JSON: %s\n", error.c_str());
        return;
      }

      if (doc["record"]["launcher"] == "n31794h75vdw95m")
      {
        if (doc["record"]["should_load"] == true && doc["record"]["loaded_at"] == "")
        {
        
        Serial.println("Loading launch...");
        doc["record"]["should_load"] = false;
         // do an update        
        
         String body = doc["record"].as<String>();        
        pb.update(
             "launches",
             doc["record"]["id"].as<String>(),
             body);

        ProtocolMessage message;
        message.version = PROTOCOL_VER;
        message.message_kind = MESSAGE_KIND_PREPARE;
        message.prepare_message.pressure = doc["record"]["pressure"].as<uint8_t>();
        message.prepare_message.water_fill = doc["record"]["water_volumic_percentage"].as<uint8_t>();
        send_message_to_master(&message);
  
  
      }
    } 
  });
  Serial.println("finished loading.");

  online_ping = millis() + 5000; // 5 seconds from now
}




void loop()
{
  // Serial.println("Updating subscription...");
  // auto free_mem  = ESP.getFreeHeap();

  // Serial.printf("Free memory: %d bytes\n", free_mem);
  pb.update_subscription();
  if (millis() > online_ping)
  {
    update_online();
    online_ping = millis() + 5000; // 5 seconds from now
  }
}
