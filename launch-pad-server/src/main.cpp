#include <Arduino.h>

#include <pulsar/callback.hpp>
#include <pulsar/protocol.hpp>

void setup(){
  Serial.begin(115200);
  Serial.println("Launch Pad Server ESP32 - I2C Slave Mode");
  
  // Initialize I2C as slave with predefined address
  setup_communication_slave(21, 22, SERVER_S_ADDRESS); // SDA=21, SCL=22 for ESP32
  Wire.begin(SERVER_S_ADDRESS);
  
  Serial.print("I2C Slave initialized at address: 0x");
  Serial.println(SERVER_S_ADDRESS, HEX);
  
  delay(500);
  
  // Send initial message to master
  ProtocolMessage message;
  message.version = PROTOCOL_VER;
  message.message_kind = MESSAGE_KIND_LOG;
  message.log_message.log_level = 0;
  message.log_message.log[0] = 'S';
  message.log_message.log[1] = 'e';
  message.log_message.log[2] = 'r';
  message.log_message.log[3] = 'v';
  message.log_message.log[4] = 'e';
  message.log_message.log[5] = 'r';
  message.log_message.log[6] = ' ';
  message.log_message.log[7] = 'U';
  message.log_message.log[8] = 'p';
  message.log_message.log_length = 9;

  send_message_to_master(&message);
  
  pinMode(0, OUTPUT); // LED for status indication
}

void loop() {
  // Check for received messages from master
  ProtocolMessage received_msg;
  if (has_received_message(&received_msg)) {
    Serial.print("Received message from master, kind: ");
    Serial.println(received_msg.message_kind);
    
    if (received_msg.message_kind == MESSAGE_KIND_LOG) {
      Serial.print("Master says: ");
      for (int i = 0; i < received_msg.log_message.log_length; i++) {
        Serial.print((char)received_msg.log_message.log[i]);
      }
      Serial.println();
    }
  }
  
  // Send periodic status updates
  static unsigned long lastUpdate = 0;
  if (millis() - lastUpdate > 3000) { // Every 3 seconds
    ProtocolMessage status_msg;
    status_msg.version = PROTOCOL_VER;
    status_msg.message_kind = MESSAGE_KIND_LOG;
    status_msg.log_message.log_level = 1;
    
    String status = "Server Online";
    status_msg.log_message.log_length = min(status.length(), 16u);
    for(int i = 0; i < status_msg.log_message.log_length; i++) {
      status_msg.log_message.log[i] = status[i];
    }
    
    send_message_to_master(&status_msg);
    lastUpdate = millis();
  }

  Serial.print("Buffered messages: ");
  Serial.println(buffered_message);
  Serial.print("Received messages: ");
  Serial.println(received_message);
  
  // Blink LED to show activity
  digitalWrite(0, HIGH);
  delay(100);
  digitalWrite(0, LOW);
  delay(100);
}
