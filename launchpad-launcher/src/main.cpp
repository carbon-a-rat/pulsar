#include <Arduino.h>
// #include <HardwareSerial.h>

#include <Grove_I2C_Motor_Driver.h>

typedef enum
{
    LAUNCH_WAIT,
    LAUNCH_PREPARE,
    LAUNCH_FILL_WATER,
    LAUNCH_FILL_PRESSURE,
    LAUNCH_RELEASING,
    LAUNCH_LAUNCHING,
} LaunchState;

static LaunchState launch_state = LAUNCH_WAIT;

#define MOTOR_I2C_ADDRESS 0x0f

// #include <pulsar/callback.hpp>
// #include <Wire.h>

const int ledPin = 4;    // the number of the LED pin, D3
const int buttonPin = 5; // the number of the pushbutton pin, D4

// ----- EAU

volatile unsigned long pulseCount = 0; // Nombre d'impulsion qu'on initialise à 0
volatile int freq_flow = 0;            // Nombre d'impulsion reçu sur la denrière seconde

unsigned char flow_sensor = 2;
const int RELAY_PIN = 9;

float consigne_mL = 500.0; // Du coup pour ce code, on décide de ce qu'on veut ici

float TOLERANCE = 50; // Tolérance en mL

/*On a F = 11 x Q[L/min] avec F[pulse/s] ainsi en 1min, si on prends Q = 1L/min, on a 1L d'eau qui est passé.
De plus on a F = 11 impulsions/s. Ainsi en 1min on en déduit qu'il y a eu 11 x 60 = 660 impulsions.
Enfin on en conclue qu'on a une valeur de 660 pulses par litre
*/
const float mL_per_pulse = 1000.0 / 660.0;

unsigned long previous_time = 0;

void flow_ISR()
{ // ISR parce que cette fonction va venir interrompre le programme en cours pour réaliser sa fonction
  //  Serial.println("Flow sensor triggered");
    freq_flow++;
    pulseCount++;
}

// ----- pression

#include "DFRobot_MPX5700.h"
#define I2C_ADDRESS_PRESSURE 0x16

DFRobot_MPX5700 mpx5700(&Wire, I2C_ADDRESS_PRESSURE);

const int RELAY_PIN_GAUCHE = 2;
const int RELAY_PIN_DROITE = 3;

float consigne_bar = 6.0; // On met ce qu'on veut ici

float TOLERANCE_BAR = 0.1; // Au pif on verra bien mdr

unsigned long current_time = 0;

void setup()
{
    Serial.begin(9600);

    // ---- button
    pinMode(buttonPin, INPUT);
    pinMode(ledPin, OUTPUT);
    digitalWrite(ledPin, HIGH);

    Serial.println("Started arduino");

    // ---- motor
    
    Serial.println("Initialisation de la gestion du moteur");
    Motor.begin(MOTOR_I2C_ADDRESS);

    // ---- eau
    /*
    Serial.println("Initialisation de la gestion de l'eau");
    

    pinMode(flow_sensor, INPUT);
    attachInterrupt(digitalPinToInterrupt(flow_sensor), flow_ISR, RISING); // Du coup ça c'est pour que quand un pic est détécté (une impulsion) alors la fonction s'acitve

    pinMode(RELAY_PIN, OUTPUT);
    digitalWrite(RELAY_PIN, LOW); // Fermer la vanne au début (on passe en analog pour la piloter précisément)
    */
  //  Serial.begin(9600);
    previous_time = millis();
    Serial.println("En attente de la consigne");
    // ---- pression

    Serial.println("Initialisation du capteur de pression");

    pinMode(RELAY_PIN_GAUCHE, OUTPUT);
    pinMode(RELAY_PIN_DROITE, OUTPUT);
    digitalWrite(RELAY_PIN_GAUCHE, LOW);
    digitalWrite(RELAY_PIN_DROITE, LOW);

    while (false == mpx5700.begin())
    {
        Serial.println("i2c begin fail,please chack connect!");
        delay(1000);
    }
    Serial.println("i2c begin success");
    mpx5700.setMeanSampleSize(/*Sample Quantity*/ 5);
    Serial.println("En attente de la consigne");
    launch_state = LAUNCH_WAIT;
}

void loop()
{

    if (launch_state == LAUNCH_WAIT)
    {
        // Wait for the button to be pressed
        if (digitalRead(buttonPin) == LOW)
        {
            launch_state = LAUNCH_PREPARE;
            Serial.println("Button pressed, preparing for launch...");
        }
    }

    if (launch_state == LAUNCH_PREPARE)
    {
        // Prepare for launch
        Serial.println("Preparing for launch...");
        // Here you can add code to prepare the rocket, like checking systems, etc.
        delay(300); // Simulate preparation time
        launch_state = LAUNCH_FILL_WATER;
    }

    if(launch_state == LAUNCH_FILL_WATER)
    {
        launch_state = LAUNCH_FILL_PRESSURE;
    }
    if (launch_state == LAUNCH_FILL_WATER)
    {
        // Fill water
        Serial.println("Filling water...");

        current_time = millis();

        // Asservissement proportionnel de la vanne
        float volume_eau_actuel = pulseCount * mL_per_pulse;
        float erreur = consigne_mL - volume_eau_actuel;

        if (erreur > TOLERANCE)
        {
            digitalWrite(RELAY_PIN, HIGH);
        }
        else
        {
            digitalWrite(RELAY_PIN, LOW);
            
            launch_state = LAUNCH_FILL_PRESSURE;
        }

        // Au cas où pour le débug
        Serial.print("Volume actuel : ");
        Serial.print(volume_eau_actuel);
        Serial.print("Consigne : ");
        Serial.print(consigne_mL);
        Serial.print("Erreur : ");
        Serial.println(erreur);

        // Bonus : Code déjà présent mais c'est histoire de connaître le débit instantanné
        if (current_time - previous_time >= 1000)
        {
            unsigned int F = freq_flow;
            float Q_L_per_hour = (float)F * 60 / 11;

            Serial.print("Débit instantanné :");
            Serial.print(Q_L_per_hour, 3);
            Serial.println("L/h");

            freq_flow = 0;
            previous_time = current_time;
        }

        delay(30);
    }
    if (launch_state == LAUNCH_FILL_PRESSURE)
    {
        // Fill pressure
        current_time = millis();

        // Asservissement non proportionnel de la vanne
        float pression_actuelle = mpx5700.getPressureValue_kpa(1) / 100; // valeur en bar
        float erreur = consigne_bar - pression_actuelle;


     //   consigne_bar -= 0.1;

        if (pression_actuelle < consigne_bar - TOLERANCE_BAR)
        {
            digitalWrite(RELAY_PIN_GAUCHE, LOW);
            digitalWrite(RELAY_PIN_DROITE, HIGH);
            Serial.println("Remplissage");
        }
        else if (pression_actuelle > consigne_bar + TOLERANCE_BAR)
        {
            digitalWrite(RELAY_PIN_GAUCHE, HIGH);
            digitalWrite(RELAY_PIN_DROITE, LOW);
            Serial.println("Vidage");
        }
        else
        {
            digitalWrite(RELAY_PIN_GAUCHE, LOW);
            digitalWrite(RELAY_PIN_DROITE, LOW);
            Serial.println("Neutre");

            launch_state = LAUNCH_RELEASING;
        }

        // Au cas oÃ¹ pour le dÃ©bug
        Serial.print("Pression actuel : ");
        Serial.print(pression_actuelle, 3);
        Serial.print("Consigne : ");
        Serial.print(consigne_bar);
        Serial.print("Erreur : ");
        Serial.print(erreur);
        // Here you can add code to fill the pressure tank
        delay(100); // Simulate filling time
    }
    if (launch_state == LAUNCH_RELEASING)
    {
       
        
        
        // make it as powerful as possible
        Motor.frequence(0x02); // Set frequency to 3921Hz
        //Motor.frequence(F_30Hz);
        
        Serial.println("Releasing pressure...");
        Motor.speed(MOTOR1, 70);

        // Release pressure
        // Here you can add code to release the pressure
        delay(2000); // Simulate releasing time
        Motor.speed(MOTOR1, -30);

        delay(500); // Simulate releasing time

        Motor.stop(MOTOR1);
        launch_state = LAUNCH_WAIT;
                Serial.println("Releasing pressure...");

        digitalWrite(RELAY_PIN_GAUCHE, HIGH);
        digitalWrite(RELAY_PIN_DROITE, LOW);
        delay(1000); // Simulate releasing time
        Serial.println("Vidage");
        
        digitalWrite(RELAY_PIN_GAUCHE, LOW);
        digitalWrite(RELAY_PIN_DROITE, LOW);

    }

    // Set speed of MOTOR1, Clockwise, speed: -100~100
    // Motor.speed(MOTOR2, 30);
    //  Motor.stop(MOTOR1);

    // delay(2000

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
    // Motor.speed(MOTOR2, -70);
    // delay(2000);
    //// Change speed and direction of MOTOR1
    // Motor.speed(MOTOR1, -100);
    //// Change speed and direction of MOTOR2
    // Motor.speed(MOTOR2, 100);
    // delay(2000);
    //// Stop MOTOR1 and MOTOR2
    // Motor.stop(MOTOR1);
    // Motor.stop(MOTOR2);
    // delay(2000);
}