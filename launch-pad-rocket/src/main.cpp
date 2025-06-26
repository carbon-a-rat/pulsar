#include <Arduino.h>
#include <WiFi.h>
#include <esp_now.h>
#include <Wire.h>

#include <pulsar/addrconfig.hpp>
#include <pulsar/protocol.hpp>

unsigned long lastTime = 0;  
unsigned long timerDelay = 3000;  // send readings timer

// Callback when data is sent
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  Serial.print("Last Packet Send Status: ");
  if (status == ESP_NOW_SEND_SUCCESS){
    Serial.println("Delivery success");
  }
  else{
    Serial.println("Delivery fail");
  }
}

// Callback when data is received
void OnDataRecv(const uint8_t *mac_addr, const uint8_t *incomingData, int len) {
  if (len == sizeof(ProtocolMessage)) {
    ProtocolMessage message;
    memcpy(&message, incomingData, sizeof(ProtocolMessage));
    
    Serial.print("Received message from rocket, kind: ");
    Serial.println(message.message_kind);
    
    if (message.message_kind == MESSAGE_KIND_LOG) {
      Serial.print("Rocket says: ");
      for (int i = 0; i < message.log_message.log_length; i++) {
        Serial.print((char)message.log_message.log[i]);
      }
      Serial.println();
    }
    else if (message.message_kind == MESSAGE_KIND_LAUNCH_STATUS) {
      Serial.println("=== ROCKET STATUS ===");
      Serial.print("Altitude: ");
      Serial.print(message.launch_status_message.altitude);
      Serial.println("m");
      Serial.print("Acc (x,y,z): ");
      Serial.print(message.launch_status_message.acceleration[0]); Serial.print(", ");
      Serial.print(message.launch_status_message.acceleration[1]); Serial.print(", ");
      Serial.println(message.launch_status_message.acceleration[2]);
      Serial.print("Gyro (x,y,z): ");
      Serial.print(message.launch_status_message.gyro[0]); Serial.print(", ");
      Serial.print(message.launch_status_message.gyro[1]); Serial.print(", ");
      Serial.println(message.launch_status_message.gyro[2]);
      Serial.print("Battery: ");
      Serial.print(message.launch_status_message.battery);
      Serial.println("%");
      Serial.println("==================");
    }
    
    // Store received message for I2C communication if needed
    //add_received_message((ProtocolMessage*)&message);
  }
}

void sendCommandToRocket(MessageKind kind) {
  ProtocolMessage command_msg;
  command_msg.version = PROTOCOL_VER;
  command_msg.message_kind = kind;
  
  if (kind == MESSAGE_KIND_LAUNCH) {
    command_msg.launch_message.launch_id = millis(); // Use timestamp as launch ID
    Serial.print("Sending LAUNCH command with ID: ");
    Serial.println(command_msg.launch_message.launch_id);
  }
  
  esp_now_send((uint8_t*)ROCKET_MAC, (uint8_t*) &command_msg, sizeof(ProtocolMessage));
}
 

void setup(){
  Serial.begin(115200);
  Serial.println("Launch Pad Rocket ESP32 - ESP-NOW Controller + I2C Slave Mode");
  
  // Initialize I2C as slave (for communication with main launchpad)
  setup_communication_slave(21, 22, LAUNCHPAD_S_ADDRESS); // SDA=21, SCL=22 for ESP32
  Wire.begin(LAUNCHPAD_S_ADDRESS);

  // Print the MAC address of the ESP32
  uint8_t mac[6];
  esp_read_mac(mac, ESP_MAC_WIFI_STA);
  Serial.print("ESP32 MAC Address: ");
  for (int i = 0; i < 6; i++) {
    if (mac[i] < 16) Serial.print("0");
    Serial.print(mac[i], HEX);
    if (i < 5) Serial.print(":");
  }
  Serial.println();
  
  // Set device in station mode
  WiFi.mode(WIFI_STA);

  // Init ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  // Register callbacks
  esp_now_register_send_cb(OnDataSent);
  esp_now_register_recv_cb(OnDataRecv);
  
  // Add rocket peer
  esp_now_peer_info_t peerInfo;
  memcpy(peerInfo.peer_addr, ROCKET_MAC, 6);
  peerInfo.channel = 0;  
  peerInfo.encrypt = false;
  
  if (esp_now_add_peer(&peerInfo) != ESP_OK){
    Serial.println("Failed to add rocket peer");
    return;
  }
  
  Serial.print("I2C Slave initialized at address: 0x");
  Serial.println(LAUNCHPAD_S_ADDRESS, HEX);
  Serial.println("ESP-NOW initialized as controller");
  
  // Send initial message to I2C master
  ProtocolMessage message;
  message.version = PROTOCOL_VER;
  message.message_kind = MESSAGE_KIND_LOG;
  message.log_message.log_level = 0;
  String msg = "LPR Ready";
  message.log_message.log_length = min(msg.length(), 16u);
  for(int i = 0; i < message.log_message.log_length; i++) {
    message.log_message.log[i] = msg[i];
  }
  
  send_message_to_master(&message);
  
  pinMode(0, OUTPUT); // LED for status indication
}


void loop() {
  // Check for received messages from I2C master
  ProtocolMessage received_msg;
  if (has_received_message(&received_msg)) {
    Serial.print("Received message from I2C master, kind: ");
    Serial.println(received_msg.message_kind);
    
    if (received_msg.message_kind == MESSAGE_KIND_LOG) {
      Serial.print("Master says: ");
      for (int i = 0; i < received_msg.log_message.log_length; i++) {
        Serial.print((char)received_msg.log_message.log[i]);
      }
      Serial.println();
    }
    else if (received_msg.message_kind == MESSAGE_KIND_LAUNCH) {
      Serial.println("Forwarding LAUNCH command to rocket!");
      sendCommandToRocket(MESSAGE_KIND_LAUNCH);
    }
    else if (received_msg.message_kind == MESSAGE_KIND_PREPARE) {
      Serial.println("Forwarding PREPARE command to rocket!");
      // Forward prepare message to rocket
      esp_now_send((uint8_t*)ROCKET_MAC, (uint8_t*) &received_msg, sizeof(ProtocolMessage));
    }
  }
  
  // Send periodic status updates to I2C master
  static unsigned long lastUpdate = 0;
  if (millis() - lastUpdate > 8000) { // Every 8 seconds
    ProtocolMessage status_msg;
    status_msg.version = PROTOCOL_VER;
    status_msg.message_kind = MESSAGE_KIND_LOG;
    status_msg.log_message.log_level = 1;
    
    String status = "LPR+Rocket OK";
    status_msg.log_message.log_length = min(status.length(), 16u);
    for(int i = 0; i < status_msg.log_message.log_length; i++) {
      status_msg.log_message.log[i] = status[i];
    }
    
    //send_message_to_master(&status_msg);
    lastUpdate = millis();
  }
  
  // Send periodic messages to rocket for testing
  if (millis() - lastTime > timerDelay) {
    ProtocolMessage test_msg;
    test_msg.version = PROTOCOL_VER;
    test_msg.message_kind = MESSAGE_KIND_LOG;
    test_msg.log_message.log_level = 0;
    String msg = "Hello Rocket";
    test_msg.log_message.log_length = min(msg.length(), 16u);
    for(int i = 0; i < test_msg.log_message.log_length; i++) {
      test_msg.log_message.log[i] = msg[i];
    }
    
    esp_now_send((uint8_t*)ROCKET_MAC, (uint8_t*) &test_msg, sizeof(ProtocolMessage));
    lastTime = millis();
  }
  
  // Blink LED to show activity
  digitalWrite(0, HIGH);
  delay(100);
  digitalWrite(0, LOW);
  delay(100);
}
