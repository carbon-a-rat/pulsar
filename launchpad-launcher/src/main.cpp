#include <Arduino.h>
//#include <HardwareSerial.h>


#include <Grove_I2C_Motor_Driver.h>


typedef enum {
    LAUNCH_WAIT, 
    LAUNCH_PREPARE,
    LAUNCH_FILL_WATER,
    LAUNCH_FILL_PRESSURE, 
    LAUNCH_RELEASING, 
    LAUNCH_LAUNCHING,
} LaunchState;


static LaunchState launch_state = LAUNCH_WAIT;

#define MOTOR_I2C_ADDRESS 0x0f

//#include <pulsar/callback.hpp>
//#include <Wire.h>  

const int ledPin = 4;      // the number of the LED pin, D3
const int buttonPin = 5;    // the number of the pushbutton pin, D4
void setup() {
    Serial.begin(115200);

    // Initialize I2C communication as Master

    pinMode(buttonPin, INPUT);
    pinMode(ledPin, OUTPUT);
  //  TCA.begin(WIRE);

    Serial.println("Started arduino");
    Motor.begin(MOTOR_I2C_ADDRESS);
    
  digitalWrite(ledPin, HIGH);
  launch_state = LAUNCH_WAIT;
  //  Wire.begin(LAUNCHPAD_S_ADDRESS);
   // setup_communication_master();

    //Serial.println("I2C Scanner");


    // Set the I2C address of the motor driver
 
}

void loop() {
    

    if (launch_state == LAUNCH_WAIT) {
        // Wait for the button to be pressed
        if (digitalRead(buttonPin) == LOW) {
            launch_state = LAUNCH_PREPARE;
            Serial.println("Button pressed, preparing for launch...");
        }
    }


    if(launch_state == LAUNCH_PREPARE)
    {
        // Prepare for launch
        Serial.println("Preparing for launch...");
        // Here you can add code to prepare the rocket, like checking systems, etc.
        delay(300); // Simulate preparation time
        launch_state = LAUNCH_FILL_WATER;
    }
    if(launch_state == LAUNCH_FILL_WATER)
    {
        // Fill water
        Serial.println("Filling water...");
        // Here you can add code to fill the water tank
        delay(300); // Simulate filling time
        launch_state = LAUNCH_FILL_PRESSURE;
    }
    if(launch_state == LAUNCH_FILL_PRESSURE)
    {
        // Fill pressure
        Serial.println("Filling pressure...");
        // Here you can add code to fill the pressure tank
        delay(300); // Simulate filling time
        launch_state = LAUNCH_RELEASING;
    }
    if(launch_state == LAUNCH_RELEASING)
    {
        // make it as powerful as possible
        Motor.frequence(0x02); // Set frequency to 3921Hz
        Serial.println("Releasing pressure...");
        Motor.speed(MOTOR1, 70);
  
        // Release pressure
        // Here you can add code to release the pressure
        delay(4000); // Simulate releasing time
        Motor.speed(MOTOR1, -30);

        delay(500); // Simulate releasing time

  
        Motor.stop(MOTOR1);
        launch_state = LAUNCH_WAIT;
    }

    // Set speed of MOTOR1, Clockwise, speed: -100~100
     // Motor.speed(MOTOR2, 30);
  //  Motor.stop(MOTOR1);

    //delay(2000

    /*
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
    }*/
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
}