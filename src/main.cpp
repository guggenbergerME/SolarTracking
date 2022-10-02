#include <Arduino.h>

// Pin output zuweisen
#define M1_re 2  // D2
#define M1_li 4  // D4

#define M2_re 5  // D5
#define M2_li 18  // D18

/////////////////////////////////////////////////////////////////////////// Funktionsprototypen
void loop                       ();
void m1                         (int); // Panel neigen
void m2                         (int); // Panel drehen
void sturmschutz                ();


/////////////////////////////////////////////////////////////////////////// SETUP
void setup() {

  // Serielle Kommunikation starten
  Serial.begin(115200);

//Pins deklarieren
  pinMode(M1_re,OUTPUT);
  pinMode(M1_li,OUTPUT);
  pinMode(M2_re,OUTPUT);
  pinMode(M2_li,OUTPUT);
}


/////////////////////////////////////////////////////////////////////////// m1 Motor 1 einfahren oder ausfahren
void m1(int x) {
  // x = 1 ausfahren | x = 2 einfahren | x = 3 aus

  if (x == 1) {
    // Motor ausfahren
    Serial.println("M1 ausfahren");
    digitalWrite(M1_re, HIGH);
    digitalWrite(M1_li, LOW);
  }

  if (x == 2) {
    // Motor einfahren
    Serial.println("M1 einfahren");
    digitalWrite(M1_re, LOW);
    digitalWrite(M1_li, HIGH);
  }

  if (x == 3) {
    // Motor einfahren
    Serial.println("M1 einfahren");
    digitalWrite(M1_re, LOW);
    digitalWrite(M1_li, LOW);
  }  

}

/////////////////////////////////////////////////////////////////////////// m1 Motor 1 einfahren oder ausfahren
void m2(int x) {
  // x = 1 ausfahren | x = 2 einfahren | x = 3 aus

  if (x == 1) {
    // Motor ausfahren
    Serial.println("M2 ausfahren");
    digitalWrite(M2_re, HIGH);
    digitalWrite(M2_li, LOW);
  }

  if (x == 2) {
    // Motor einfahren
    Serial.println("M2 einfahren");
    digitalWrite(M2_re, LOW);
    digitalWrite(M2_li, HIGH);
  }

  if (x == 3) {
    // Motor einfahren
    Serial.println("M2 einfahren");
    digitalWrite(M2_re, LOW);
    digitalWrite(M2_li, LOW);
  }  

}

/////////////////////////////////////////////////////////////////////////// Sturmschutz - Solarpanel waagerecht ausrichten 
void sturmschutz() {

  // Motor m1 Panel waagerecht ausrichten
  m1(1);

  // Motor m2 Panel drehen Osten
  m2(2); 

}


/////////////////////////////////////////////////////////////////////////// LOOP
void loop() {




/*// Motoren Test
m2(1);
delay(3000);
m2(2);
delay(3000);
m2(3);
delay(3000);
*/
}