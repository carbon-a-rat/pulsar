#include <Arduino.h>

#include <pulsar/networking.hpp>

#include <Wire.h>

#include <pulsar/callback.hpp>



void setup(){


  Serial.begin(115200);
  Serial.println("\nI2C Scanner");
  Wire.begin(5,4); // Join I2C bus as master
  Wire.setClock(20000); // Set I2C clock speed (100kHz is standard, try 50kHz if issues persist)




  
}

void loop() {
  byte error, address;
  int nDevices;

  Serial.println("Scanning...");

  nDevices = 0;
  for(address = 1; address < 127; address++ ) {
   // The i2c_scanner uses the return value of
    // the Write.endTransmission to see if
    // a device did acknowledge to the address.
    Wire.beginTransmission(address);
    error = Wire.endTransmission();

    if (error == 0) {
      Serial.print("I2C device found at address 0x");
      if (address<16)
        Serial.print("0");
      Serial.print(address,HEX);
      Serial.println("  !");
      nDevices++;
    }
    else if (error==4) {
      Serial.print("Unknown error at address 0x");
      if (address<16)
        Serial.print("0");
      Serial.println(address,HEX);
    }
  }
  if (nDevices == 0)
    Serial.println("No I2C devices found\n");
  else
    Serial.println("done\n");

  delay(5000); // Wait 5 seconds for next scan
  digitalWrite(0, HIGH);
  delay(200);
  digitalWrite(0, LOW);
  delay(200);
}
