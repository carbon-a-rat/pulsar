#include <Arduino.h>
#include <HardwareSerial.h>

#include <pulsar/callback.hpp>
#include <Wire.h>

void setup() {

  pinMode(0, OUTPUT);
  Wire.begin(ROCKET_S_ADDRESS);

  setup_communication_slave(5,4, ROCKET_S_ADDRESS);
  delay(1000);


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
  message.log_message.log[6] = '1';
  message.log_message.log[7] = ')';
  message.log_message.log_length = 8;

  send_message_to_master(&message);


}

void loop() {
 
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