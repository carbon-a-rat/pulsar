#include <Arduino.h>
#include <HardwareSerial.h>

#include <pulsar/callback.hpp>
#include <Wire.h>  

bool log_callback(ProtocolMessage *message, uint8_t addr)
{
    Serial.print("Log from slave 0x");
    Serial.print(addr, HEX);
    Serial.print(": ");
    for (int i = 0; i < message->log_message.log_length; i++)
    {
        Serial.print((char)message->log_message.log[i]);
    }
    Serial.println();
    return true;
}

void scanI2C() {
  byte error, address;
  int nDevices;

  Serial.println("Scanning for I2C devices...");

  nDevices = 0;
  for(address = 1; address < 127; address++ ) {
    Wire.beginTransmission(address);
    error = Wire.endTransmission();

    if (error == 0) {
      Serial.print("I2C device found at address 0x");
      if (address < 16)
        Serial.print("0");
      Serial.print(address, HEX);
      Serial.println("  !");
      nDevices++;
    }
    else if (error == 4) {
      Serial.print("Unknown error at address 0x");
      if (address < 16)
        Serial.print("0");
      Serial.println(address, HEX);
    }
  }
  if (nDevices == 0)
    Serial.println("No I2C devices found");
  else {
    Serial.print(nDevices);
    Serial.println(" device(s) found");
  }
  Serial.println("---");
}

void setup() {
  Serial.begin(115200);
  
  // Initialize I2C communication as Master
  setup_communication_master();
  
  Serial.println("Arduino I2C Master initialized");
  Serial.println("Available I2C slave addresses:");
  Serial.println("- 0x08: SERVER_S_ADDRESS (launch-pad-server)");
  Serial.println("- 0x09: LAUNCH_PAD_ROCKET_ADDRESS (launch-pad-rocket)");
  
  delay(2000);
  
  // Initial I2C scan
  scanI2C();
}

void loop() {
  // Periodic I2C scan every 10 seconds
  static unsigned long lastScan = 0;
  if (millis() - lastScan > 10000) {
    scanI2C();
    lastScan = millis();
  }
  
  // Request messages from ESP slave devices
  ProtocolMessage message;
  
  // Request from launch-pad-rocket (ESP32)
  if (request_from_slave(&message, LAUNCHPAD_S_ADDRESS)) { // Using address 0x20 for launch-pad-rocket
    if (message.version == PROTOCOL_VER && message.message_kind == MESSAGE_KIND_LOG) {
      log_callback(&message, LAUNCHPAD_S_ADDRESS);
    }
  }
  
  // Request from launch-pad-server (ESP32)
  if (request_from_slave(&message, SERVER_S_ADDRESS)) {
    if (message.version == PROTOCOL_VER && message.message_kind == MESSAGE_KIND_LOG) {
      log_callback(&message, SERVER_S_ADDRESS);
    }
  }
  
  // Send commands to slaves if needed
  // Example: Send a test message to launch-pad-rocket
  static unsigned long lastCommand = 0;
  if (millis() - lastCommand > 8000) { // Every 8 seconds
    ProtocolMessage test_message;
    test_message.version = PROTOCOL_VER;
    test_message.message_kind = MESSAGE_KIND_LOG;
    test_message.log_message.log_level = 1;
    String test_text = "Arduino Master";
    test_message.log_message.log_length = min(test_text.length(), 16);
    for(int i = 0; i < test_message.log_message.log_length; i++) {
      test_message.log_message.log[i] = test_text[i];
    }
    
    Serial.println("Sending test message to slaves...");
    send_message_to_slave(&test_message, LAUNCHPAD_S_ADDRESS);
    send_message_to_slave(&test_message, SERVER_S_ADDRESS);
    
    lastCommand = millis();
  }
  
  delay(1000);
}
/*
#include <Wire.h>

#include <HardwareSerial.h>
#include <Arduino.h>
#include <time.h>
#include <pulsar/callback.hpp>

#define I2C_ADDRESS 0x0f // default I2C address is 0x0f



bool log_callback(ProtocolMessage *message, uint8_t addr)
{
    Serial.print("Log from ");
    Serial.print(addr);
    Serial.print(": ");
    for (int i = 0; i < message->log_message.log_length; i++)
    {
        Serial.print((char)message->log_message.log[i]);
    }
    Serial.println();
    return true;
}

void setup() {
    Serial.begin(9600);

    // Initialize I2C communication as Master


    Serial.println("I2C Scanner");
    delay(500);
  //  Motor.begin(I2C_ADDRESS);
  //  Wire.begin(LAUNCHPAD_S_ADDRESS);
    setup_communication_master();

    Serial.println("I2C Scanner");


    // Set the I2C address of the motor driver
 
}

void loop() {
    // Set speed of MOTOR1, Clockwise, speed: -100~100
//    Motor.speed(MOTOR1, -100);
   // Motor.speed(MOTOR2, 30);
  //  Motor.stop(MOTOR1);
    
    //delay(2000

    // print the message
    ProtocolMessage message;
    request_from_slave(&message, ROCKET_S_ADDRESS);
    if (message.version != PROTOCOL_VER)
    {
        Serial.println("Error: version mismatch");
        return;
    }

    if(message.message_kind == MESSAGE_KIND_LOG)
    {
        log_callback(&message, ROCKET_S_ADDRESS);
    }
    
    request_from_slave(&message, SERVER_S_ADDRESS);
    if (message.version != PROTOCOL_VER)
    {
        Serial.println("Error: version mismatch");
        return;
    }

    if(message.message_kind == MESSAGE_KIND_LOG)
    {
        
        log_callback(&message, SERVER_S_ADDRESS);
    }
    delay(100);
    // Set speed of MOTOR2, Anticlockwise
    //Motor.speed(MOTOR2, -70);
    //delay(2000);
    //// Change speed and direction of MOTOR1
    //Motor.speed(MOTOR1, -100);
    //// Change speed and direction of MOTOR2
    //Motor.speed(MOTOR2, 100);
    //delay(2000);
    //// Stop MOTOR1 and MOTOR2
    //Motor.stop(MOTOR1);
    //Motor.stop(MOTOR2);
   // delay(2000);
}*/