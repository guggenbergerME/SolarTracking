#include <Arduino.h>

/////////////////////////////////////////////////////////////////////////// Pin output zuweisen
#define M1_re 2  // D2
#define M1_li 4  // D4
#define M2_re 5  // D5
#define M2_li 18  // D18

/////////////////////////////////////////////////////////////////////////// Interrup
int sturmschutzschalterpin =  13;
int panelsenkrechtpin =  12;

/////////////////////////////////////////////////////////////////////////// ADC zuweisen
const int adc_NO = 34; //ADC1_6 - Fotowiderstand 
const int adc_NW = 35; //ADC1_7 - Fotowiderstand 
const int adc_SO = 33; //ADC1_8 - Fotowiderstand
const int adc_SW = 32; //ADC1_9 - Fotowiderstand 

int sensorSonne_NO, sensorSonne_NW, sensorSonne_SO, sensorSonne_SW;
int horizontal_hoch, horizontal_runter, vertikal_rechts, vertikal_links; 
int differenz_neigen, differenz_drehen, sonne_quersumme, neigen_fahrt;
int traker_tolleranz_neigen = 60; // Getestet mit 300
int traker_tolleranz_drehen = 90;
int helligkeit_schwellwert = 700; // Wolkenschwellwert
int helligkeit_nachtstellung = 1400; // Wolkenschwellwert

/////////////////////////////////////////////////////////////////////////// Windsensor Variablen
int wind_zu_stark = 0;
int sturmschutz_pause = 5000000;
int pin_anemometer = 23; // Impulsgeber des Anemometer
unsigned long start_time = 0;
unsigned long end_time = 0;
int steps = 0;
int steps_schwellwert = 8;

/////////////////////////////////////////////////////////////////////////// Schleifen verwalten
unsigned long previousMillis_Sturmcheck = 0; // Windstärke prüfen
unsigned long interval_Sturmcheck = 5000; 

unsigned long previousMillis_sonnensensor = 0; // Sonnenstand prüfen
unsigned long interval_sonnensensor = 5000; 

unsigned long previousMillis_sturmschutzschalter = 0; // Sturmschutz Schalter prüfen
unsigned long interval_sturmschutzschalter = 1200; 

unsigned long previousMillis_panelsenkrecht = 0; // Sturmschutz Schalter prüfen
unsigned long interval_panelsenkrecht = 1300; 

/////////////////////////////////////////////////////////////////////////// Funktionsprototypen
void loop                       ();
void m1                         (int); // Panel neigen
void m2                         (int); // Panel drehen
void sturmschutz                ();
void panel_senkrecht            ();
void sonnenaufgang              ();
void sonnensensor               ();
void sturmschutzschalter        ();


/////////////////////////////////////////////////////////////////////////// SETUP
void setup() {

  // Serielle Kommunikation starten
  Serial.begin(38400);

// Sturmschutzschalter init
pinMode(sturmschutzschalterpin, INPUT);

// Panel senkrecht init
pinMode(panelsenkrechtpin, INPUT);

//Pins deklarieren
  pinMode(M1_re,OUTPUT);
  pinMode(M1_li,OUTPUT);
  pinMode(M2_re,OUTPUT);
  pinMode(M2_li,OUTPUT);
}

/////////////////////////////////////////////////////////////////////////// Sonnensensor - Fotowiderstände
void sonnensensor(){

sensorSonne_NO = analogRead(adc_NO);  
sensorSonne_NW = analogRead(adc_NW);
sensorSonne_SO = analogRead(adc_SO);
sensorSonne_SW = analogRead(adc_SW);


// Werte Seriell ausgeben
Serial.print("Wert sensorSonne_NO : ");
Serial.println(sensorSonne_NO);
Serial.print("Wert sensorSonne_NW : ");
Serial.println(sensorSonne_NW);
Serial.print("Wert sensorSonne_SO : ");
Serial.println(sensorSonne_SO);
Serial.print("Wert sensorSonne_SW : ");
Serial.println(sensorSonne_SW);

// Quersumme aller Werte
sonne_quersumme = (sensorSonne_NO + sensorSonne_NW + sensorSonne_SO + sensorSonne_SW) / 4;

Serial.print("Sonne Quersumme: ");
Serial.println(sonne_quersumme);
Serial.print("Sonne Quersumme max Wert ");
Serial.println(helligkeit_schwellwert);

// Justierung stoppen Wolken
if (sonne_quersumme < helligkeit_schwellwert) {
Serial.println("Helligkeit - Ausrichten ");

horizontal_hoch   = ((sensorSonne_NO + sensorSonne_NW)/2)*(-1);
horizontal_runter = ((sensorSonne_SO + sensorSonne_SW)/2)*(-1);
vertikal_rechts   = ((sensorSonne_NO + sensorSonne_SO)/2)*(-1);
vertikal_links    = ((sensorSonne_NW + sensorSonne_SW)/2)*(-1);

    // Sonnentraking Drehen
    differenz_drehen = (sensorSonne_NW + sensorSonne_SW)/2  - (sensorSonne_NO + sensorSonne_SO)/2;
    Serial.print("Differenz Drehen: ");
    Serial.println(differenz_drehen);


    if (vertikal_rechts > vertikal_links && (vertikal_rechts-vertikal_links) > traker_tolleranz_drehen) {
      Serial.println("Motor drehen - LINKS");
      m2(1);  
       delay(1000);
    } else {
      m2(3);
    }
    
    if (vertikal_links > vertikal_rechts && (vertikal_links-vertikal_rechts) > traker_tolleranz_drehen) {
      Serial.println("Motor drehen - RECHTS");
      m2(2); 
       delay(1000);
    } else {
      m2(3);
    }
    // Motor stoppe
    /*    m2(3);
    delay(2000);   */

    // Schwellwert überschritten Ausrichten
    // Sonnentraking Neigen
    differenz_neigen = ((sensorSonne_NO + sensorSonne_NW)/2) - ((sensorSonne_SO + sensorSonne_SW)/2);
    Serial.print("Differenz Neigen: ");
    Serial.println(differenz_neigen);


if (horizontal_hoch > horizontal_runter && (horizontal_hoch-horizontal_runter) > traker_tolleranz_neigen) {
      Serial.println("Motor neigen - RUNTER");
      m1(2); 
       delay(1000);
} else {
  m1(3);
}
 
if (horizontal_runter > horizontal_hoch && (horizontal_runter-horizontal_hoch) > traker_tolleranz_neigen) {
      Serial.println("Motor neigen - HOCH");
      m1(1); 
       delay(1000);      
} else {
  m1(3);
}

    // Motor stoppen
    //m1(3);


} else {
Serial.println("Helligkeit - Nichts tun ");

// Wenn die Helligkeit dunkler als der Nachtwert ist Platten horizontal stellen

  if (sonne_quersumme > helligkeit_nachtstellung) { 
  // Platte horizontal stellen
    Serial.println("Platten Nachtstellung");
    m1(2);
    m2(1);
  } else {
    Serial.println("Zu wenig Sonne, keine Bewegung");

  }

}

}

/////////////////////////////////////////////////////////////////////////// m1 Neigen
void m1(int x) {
  // x = 1 senken | x = 2 heben | x = 3 aus

  if (x == 1) {
    // Panele senken
    //Serial.println("Panele senken");
    digitalWrite(M1_re, HIGH);
    digitalWrite(M1_li, LOW);
  }

  if (x == 2) {
    // Panele heben
    //Serial.println("Panele heben");
    digitalWrite(M1_re, LOW);
    digitalWrite(M1_li, HIGH);
  }

  if (x == 3) {
    // Neigen stop
    //Serial.println("Neigen stop");
    digitalWrite(M1_re, LOW);
    digitalWrite(M1_li, LOW);
  }  

}

/////////////////////////////////////////////////////////////////////////// m1 Drehen
void m2(int x) {
  // x = 1 links | x = 2 rechts | x = 3 aus

  if (x == 1) {
    // Panel links drehen
    //erial.println("Panel links drehen");
    digitalWrite(M2_re, HIGH);
    digitalWrite(M2_li, LOW);
  }

  if (x == 2) {
    // Panel rechts drehen
    //Serial.println("Panel rechts drehen");
    digitalWrite(M2_re, LOW);
    digitalWrite(M2_li, HIGH);
  }

  if (x == 3) {
    // Drehen stop
    //Serial.println("Drehen stop");
    digitalWrite(M2_re, LOW);
    digitalWrite(M2_li, LOW);
  }  

}

/////////////////////////////////////////////////////////////////////////// Sturmschutz - Solarpanel waagerecht ausrichten 
void sturmschutz() {

  // Speedsensor auslesen
  //Serial.println("Winddaten messen");

     start_time=millis();
    end_time=start_time+1000;
      while(millis()<end_time)
      {

        if(digitalRead(pin_anemometer))
        {
          steps=steps+1; 
          //while(digitalRead(pin_anemometer));
          while(digitalRead(pin_anemometer));
        }
      
      }

    Serial.print("Steps  ");
    Serial.println(steps);

    if (steps > steps_schwellwert) {
      // Variable Windschutz schreiben
      wind_zu_stark = 1;
      // Wenn Wind eine gewisse stärke erreicht hat Panel fahren und int wind_zu_stark auf 1 setzen

        Serial.println("Wind zu stark Panele sichern ");
        // Panel in Position fahren
        m1(2);
        // Lange pause um Prozessor zu unterbrechen.
        delay (sturmschutz_pause);
        
    } else {
      wind_zu_stark = 0;

    }

}

/////////////////////////////////////////////////////////////////////////// Schneelast / Reinigen - Solarpanel senkrecht ausrichten 
void panel_senkrecht() {

  // Schalter abfragen
  	while( digitalRead(panelsenkrechtpin) == 1 ) //while the button is pressed
      {
        //blink
        Serial.println("Panele senkrecht stellen");
        m1(1);

        delay(500);
      }

}

/////////////////////////////////////////////////////////////////////////// Sonnenaufgang - Panele ausrichten 
void sonnenaufgang() {

  // Motor m1 Panel senkrecht ausrichten bis Endlage
  m1(1);

  // Motor m2 Panel drehen Osten bis Endlage
  m2(1); 

}

/////////////////////////////////////////////////////////////////////////// Sonnenaufgang - Panele ausrichten 
void sturmschutzschalter() {

  // Schalter abfragen
  	while( digitalRead(sturmschutzschalterpin) == 1 ) //while the button is pressed
      {
        //blink
        Serial.println("Alles unterbrechen wegen Windschutz!");
        m1(2);
        delay(500);
      }

}

/////////////////////////////////////////////////////////////////////////// LOOP
void loop() {


  //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ Auf Sturm prüfen
  if (millis() - previousMillis_Sturmcheck > interval_Sturmcheck) {
      previousMillis_Sturmcheck = millis(); 
      // Windstärke prüfen
      Serial.println("Windstärke prüfen");
      // Messwert zurücksetzen
      steps = 0;
      sturmschutz();
    }


  //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ Sturmschutzschalter abfragen
  if (millis() - previousMillis_sturmschutzschalter > interval_sturmschutzschalter) {
      previousMillis_sturmschutzschalter = millis(); 
      // Windstärke prüfen
      //Serial.println("Sturmschutzschalter Prüfen");
     sturmschutzschalter();
    }


  //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ Panele senkrecht
  if (millis() - previousMillis_panelsenkrecht > interval_panelsenkrecht) {
      previousMillis_panelsenkrecht = millis(); 
      // Windstärke prüfen
      //Serial.println("Panele senkrecht stellen");
      panel_senkrecht();
    }


  //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ Auf Sturm prüfen
  if (millis() - previousMillis_sonnensensor > interval_sonnensensor) {
      previousMillis_sonnensensor = millis(); 
      // Sonnenposition prüfen wenn windstärke okay
      if (wind_zu_stark != 1) {
        Serial.println("Position der Sonne prüfen.");
      sonnensensor();
      } else {
        Serial.println("Keine Ausrichtung, da Wind zu stark!");
      }
  
    }



delay(800);
}