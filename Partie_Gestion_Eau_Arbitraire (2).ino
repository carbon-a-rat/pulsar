volatile unsigned long pulseCount = 0; //Nombre d'impulsion qu'on initialise à 0 
volatile int freq_flow = 0; //Nombre d'impulsion reçu sur la denrière seconde

unsigned char flow_sensor = 2;
const int RELAY_PIN =  9;

float consigne_mL = 500.0; //Du coup pour ce code, on décide de ce qu'on veut ici 

float TOLERANCE = 50; //Tolérance en mL
 

/*On a F = 11 x Q[L/min] avec F[pulse/s] ainsi en 1min, si on prends Q = 1L/min, on a 1L d'eau qui est passé.
De plus on a F = 11 impulsions/s. Ainsi en 1min on en déduit qu'il y a eu 11 x 60 = 660 impulsions.
Enfin on en conclue qu'on a une valeur de 660 pulses par litre
*/
const float mL_per_pulse = 1000.0/660.0;

unsigned long previous_time = 0 ;
unsigned long current_time = 0 ; 

void flow_ISR(){ //ISR parce que cette fonction va venir interrompre le programme en cours pour réaliser sa fonction
  freq_flow++;
  pulseCount++;
}

void setup() {
  pinMode(flow_sensor, INPUT);
  attachInterrupt(digitalPinToInterrupt(flow_sensor), flow_ISR, RISING); //Du coup ça c'est pour que quand un pic est détécté (une impulsion) alors la fonction s'acitve

  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, LOW); // Fermer la vanne au début (on passe en analog pour la piloter précisément)

  Serial.begin(9600);
  previous_time = millis();
  Serial.println("En attente de la consigne");
}

void loop() {
  current_time = millis();

  //Asservissement proportionnel de la vanne
  float volume_eau_actuel = pulseCount * mL_per_pulse;
  float erreur = consigne_mL - volume_eau_actuel;


  if (erreur > TOLERANCE){
    digitalWrite(RELAY_PIN, HIGH);
  }
  else{
    digitalWrite(RELAY_PIN, LOW);
  }



  //Au cas où pour le débug
  Serial.print("Volume actuel : ");
  Serial.print(volume_eau_actuel);
  Serial.print("Consigne : ");
  Serial.print(consigne_mL);
  Serial.print("Erreur : ");
  Serial.println(erreur);

  //Bonus : Code déjà présent mais c'est histoire de connaître le débit instantanné
  if (current_time - previous_time >= 1000){
    unsigned int F = freq_flow;
    float Q_L_per_hour = (float)F * 60 / 11;

    Serial.print("Débit instantanné :");
    Serial.print(Q_L_per_hour, 3);
    Serial.println("L/h");

    freq_flow = 0 ;
    previous_time = current_time;
  }

  delay(100);
}


