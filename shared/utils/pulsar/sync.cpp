#ifdef ARDUINO_ARCH_ESP32

#include "sync.hpp"


#include <NTPClient.h>


#include <WiFiUdp.h>
#include <Arduino.h>

static WiFiUDP ntpUDP;
static NTPClient timeClient(ntpUDP, "pool.ntp.org", 0, 60000);

static long last_update = 0;


// shamelessly stolen from @Clemorange22 endurence code.
String epoch_to_iso(time_t epochTime) {
  tm *timeinfo = gmtime(&epochTime);
  char buffer[50];
  strftime(buffer, sizeof(buffer), "%Y-%m-%dT%H:%M:%SZ", timeinfo);
  return String(buffer);
}

void time_sync_init() {
    
    timeClient.begin();
    
    // Wait for the first sync
    while (!timeClient.update()) {
        timeClient.forceUpdate();
    }
    
    Serial.println("Time synchronized with NTP server.");
    
    // Print the current time
    Serial.print("Current time: ");
    Serial.println(timeClient.getFormattedTime());

    Serial.print("Epoch time: ");
    Serial.println(timeClient.getEpochTime());

    last_update = millis();
}

void time_sync_loop() {
    if (millis() - last_update > 60000) { // Update every 60 seconds
        if (timeClient.update()) {
            Serial.print("Updated time: ");
            Serial.println(timeClient.getFormattedTime());
            
        } else {
            Serial.println("Failed to update time from NTP server.");
        }
        last_update = millis();
    }

}

String current_iso_time()
{
    return epoch_to_iso(current_epoch_time());
}
time_t current_epoch_time()
{
    return timeClient.getEpochTime();
}

#endif