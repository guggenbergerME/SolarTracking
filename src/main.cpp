#include <Arduino.h>

// Pin output zuweisen
#define M1_re 2  // D2
#define M1_li 4  // D4

/////////////////////////////////////////////////////////////////////////// Funktionsprototypen
//void callback                (char*, byte*, unsigned int);
void loop                      ();



/////////////////////////////////////////////////////////////////////////// SETUP
void setup() {

  // Serielle Kommunikation starten
  Serial.begin(115200);

//Pins deklarieren
  pinMode(M1_re,OUTPUT);
  pinMode(M1_li,OUTPUT);




}



/////////////////////////////////////////////////////////////////////////// LOOP
void loop() {



Serial.println("M1_re HIGH");
digitalWrite(M1_re, HIGH);
delay(1000);
Serial.println("M1_re LOW");
digitalWrite(M1_re, LOW);
delay(1000);

Serial.println("M1_li HIGH");
digitalWrite(M1_li, HIGH);
delay(1000);
Serial.println("M1_li LOW");
digitalWrite(M1_li, LOW);
delay(1000);
}