#include <Arduino.h>


#include <pulsar/callback.hpp>
#include <pulsar/protocol.hpp>
//#include <WiFi.h>


void setup(){
  Serial.begin(115200);
//  WiFi.mode(WIFI_MODE_STA);
//  Serial.println(WiFi.macAddress());
// sleep 
  delay(500);
  Serial.println("I2C Scanner");
//  pinMode(0, OUTPUT);
  

  
  Wire.begin(4,5, SERVER_S_ADDRESS);
  setup_communication_slave(5,4, SERVER_S_ADDRESS);

  ProtocolMessage message;
  
  message.version = PROTOCOL_VER;
  message.message_kind = MESSAGE_KIND_LOG;
  message.log_message.log_level = 0;
  message.log_message.log[0] = 'H';
  message.log_message.log[1] = 'e';
  message.log_message.log[2] = 'l';
  message.log_message.log[3] = 'l';
  message.log_message.log[4] = 'o';

  message.log_message.log[5] = '(';
  message.log_message.log[6] = '2';
  message.log_message.log[7] = ')';
  message.log_message.log_length = 8;

  Serial.println("sending message to master");
  send_message_to_master(&message);
  
  Serial.println("sended message to master");
}


void loop() {
  Serial.print("Buffered message: ");


  Serial.println(buffered_message);

  Serial.print("Received message: ");

  Serial.println(received_message);
  digitalWrite(0, HIGH);
  delay(100);
  digitalWrite(0, LOW);
  delay(100);
}
