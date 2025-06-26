#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <espnow.h>
#include <pulsar/addrconfig.hpp>
#include <pulsar/protocol.hpp>
#include "sensors.hpp"


AccGyroSensor accGyroSensor;
BarometerSensor barometerSensor;
void macAddressToByteArray(const char *macAddress, uint8_t *byteArray) {
  int i = 0;
  char *macAddressCopy = strdup(macAddress);
  char *token = strtok(macAddressCopy, ":");
  while (token != NULL && i < 6) {
    byteArray[i] = (uint8_t)strtol(token, NULL, 16);
    token = strtok(NULL, ":");
    i++;
  }
  free(macAddressCopy);
}

void printMacAddresses() {
  Serial.print("Rocket MAC Address: ");
  Serial.println(WiFi.macAddress());
  Serial.print("Gateway MAC Address: ");
  Serial.println(GATEWAY_ADDR);
}

unsigned long lastTime = 0;  
unsigned long timerDelay = 300;  // send readings timer

// Callback when data is sent
void OnDataSent(uint8_t *mac_addr, uint8_t sendStatus) {
  Serial.print("Last Packet Send Status: ");
  if (sendStatus == 0){
    Serial.println("Delivery success");
  }
  else{
    Serial.println("Delivery fail");
  }
}

// Callback when data is received
void OnDataRecv(uint8_t *mac_addr, uint8_t *incomingData, uint8_t len) {
  if (len == sizeof(ProtocolMessage)) {
    ProtocolMessage message;
    memcpy(&message, incomingData, sizeof(ProtocolMessage));
    
    Serial.print("Received message from launchpad, kind: ");
    Serial.println(message.message_kind);
    
    if (message.message_kind == MESSAGE_KIND_LOG) {
      Serial.print("Launchpad says: ");
      for (int i = 0; i < message.log_message.log_length; i++) {
        Serial.print((char)message.log_message.log[i]);
      }
      Serial.println();
    }
    else if (message.message_kind == MESSAGE_KIND_LAUNCH) {
      Serial.print("LAUNCH COMMAND RECEIVED! Launch ID: ");
      Serial.println(message.launch_message.launch_id);
      // TODO: Implement launch sequence
    }
  }
}

void sendStatusMessage() {
  ProtocolMessage status_msg;
  status_msg.version = PROTOCOL_VER;
  status_msg.message_kind = MESSAGE_KIND_LAUNCH_STATUS;
  


  // Simulate sensor data (replace with actual sensor readings)


  float ax, ay, az, gx, gy, gz;
  accGyroSensor.read(ax, ay, az, gx, gy, gz);
  status_msg.launch_status_message.acceleration[0] = (int16_t)(ax * 1000); // Convert to milli-g
  status_msg.launch_status_message.acceleration[1] = (int16_t)(ay * 1000);
  status_msg.launch_status_message.acceleration[2] = (int16_t)(az * 1000);
  status_msg.launch_status_message.gyro[0] = (int16_t)(gx * 100); // Convert to centi-degrees/s
  status_msg.launch_status_message.gyro[1] = (int16_t)(gy * 100);
  status_msg.launch_status_message.gyro[2] = (int16_t)(gz * 100);

  float pressure, temperature, altitude;
  barometerSensor.read(pressure, temperature, altitude);
  status_msg.launch_status_message.altitude = (uint8_t)(altitude / 1000); // Convert to meters (0-255 -> 0-0.25m)
  

  
  status_msg.launch_status_message.battery = 85; // 85% battery
  
  esp_now_send((uint8_t*)GATEWAY_MAC, (uint8_t*) &status_msg, sizeof(ProtocolMessage));
}
void setup(){
  Serial.begin(115200);
  Serial.println("Rocket ESP8266 - ESP-NOW Slave Mode");

  pinMode(LED_BUILTIN, OUTPUT);
  
  printMacAddresses();

  WiFi.mode(WIFI_STA);

  // Init ESP-NOW
  if (esp_now_init() != 0) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  // Set ESP-NOW role
  esp_now_set_self_role(ESP_NOW_ROLE_SLAVE);
  esp_now_register_send_cb(OnDataSent);
  esp_now_register_recv_cb(OnDataRecv);
  
  // Add peer (gateway/launchpad)
  esp_now_add_peer((uint8_t*)GATEWAY_MAC, ESP_NOW_ROLE_CONTROLLER, 1, NULL, 0);
  
  Serial.println("ESP-NOW initialized successfully");
  
  // Initialize sensors
  if (!accGyroSensor.initialize()) {
    Serial.println("Failed to initialize accelerometer/gyroscope sensor");
  } else {
    Serial.println("Accelerometer/Gyroscope sensor initialized successfully");
  }

  if (!barometerSensor.initialize()) {
    Serial.println("Failed to initialize barometer sensor");
  } else {
    Serial.println("Barometer sensor initialized successfully");
  }


  // Send initial message to launchpad
  ProtocolMessage initial_msg;
  initial_msg.version = PROTOCOL_VER;
  initial_msg.message_kind = MESSAGE_KIND_LOG;
  initial_msg.log_message.log_level = 0;
  String msg = "Rocket Ready";
  initial_msg.log_message.log_length = min(msg.length(), 16u);
  for(int i = 0; i < initial_msg.log_message.log_length; i++) {
    initial_msg.log_message.log[i] = msg[i];
  }
  
  esp_now_send((uint8_t*)GATEWAY_MAC, (uint8_t*) &initial_msg, sizeof(ProtocolMessage));
}
 
void loop(){
  // Send periodic status updates
  if (millis() - lastTime > timerDelay) {
    digitalWrite(LED_BUILTIN, LOW);  // LED on (inverted on ESP8266)
    sendStatusMessage();
    lastTime = millis();
  }
  else 
  {
    digitalWrite(LED_BUILTIN, HIGH); // LED off
  
  }
  
}
