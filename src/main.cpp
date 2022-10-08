#include <Arduino.h>
#include "time.h"
#include "WiFi.h"

/////////////////////////////////////////////////////////////////////////// Pin output zuweisen
#define M1_re 2  // D2
#define M1_li 4  // D4
#define M2_re 5  // D5
#define M2_li 18  // D18

/////////////////////////////////////////////////////////////////////////// ADC zuweisen
const int adc_NO = 34; //ADC1_6 - Fotowiderstand 
const int adc_NW = 35; //ADC1_7 - Fotowiderstand 
const int adc_SO = 33; //ADC1_8 - Fotowiderstand 
const int adc_SW = 32; //ADC1_9 - Fotowiderstand 

int sensorSonne_NO, sensorSonne_NW, sensorSonne_SO, sensorSonne_SW;
int ausrichten_oben, ausrichten_unten, ausrichten_rechts, ausrichten_links, neigen_fahrt; 
int differenz_neigen, differenz_drehen, sonne_quersumme;
int traker_tolleranz = 150; // Getestet mit 300
int traker_wolken = 2500; // Wolkenschwellwert

/////////////////////////////////////////////////////////////////////////// Windsensor Variablen
int wind_zu_stark = 0;

/////////////////////////////////////////////////////////////////////////// NTP Daten
const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = 3600;
const int   daylightOffset_sec = 3600;

/////////////////////////////////////////////////////////////////////////// Schleifen verwalten
unsigned long previousMillis_Sturmcheck = 0; // Windstärke prüfen
unsigned long interval_Sturmcheck = 15000; 

unsigned long previousMillis_sonnensensor = 0; // Sonnenstand prüfen
unsigned long interval_sonnensensor = 2000; 

/////////////////////////////////////////////////////////////////////////// Funktionsprototypen
void loop                       ();
void m1                         (int); // Panel neigen
void m2                         (int); // Panel drehen
void sturmschutz                ();
void panel_senkrecht            ();
void sonnenaufgang              ();
void sonnensensor               ();
void wifi_setup                 ();
void LokaleZeit                 ();


/////////////////////////////////////////////////////////////////////////// SETUP - Wifi
void wifi_setup() {

// WiFi Zugangsdaten
const char* WIFI_SSID = "GuggenbergerLinux";
const char* WIFI_PASS = "Isabelle2014samira";

// Static IP
IPAddress local_IP(192, 168, 13, 50);
IPAddress gateway(192, 168, 1, 1);
IPAddress subnet(255, 0, 0, 0);  
IPAddress dns(192, 168, 1, 1); 

// Verbindung zu SSID
Serial.print("Verbindung zu SSID - ");
Serial.println(WIFI_SSID); 

// IP zuweisen
if (!WiFi.config(local_IP, gateway, subnet, dns)) {
   Serial.println("STA fehlerhaft!");
  }

// NTP Setup
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  LokaleZeit();

// WiFI Modus setzen
WiFi.mode(WIFI_OFF);
WiFi.disconnect();
delay(100);

WiFi.begin(WIFI_SSID, WIFI_PASS);
Serial.println("Verbindung aufbauen ...");

while (WiFi.status() != WL_CONNECTED) {

  if (WiFi.status() == WL_CONNECT_FAILED) {
     Serial.println("Keine Verbindung zum SSID möglich : ");
     Serial.println();
     Serial.print("SSID: ");
     Serial.println(WIFI_SSID);
     Serial.print("Passwort: ");
     Serial.println(WIFI_PASS);
     Serial.println();
    }
  delay(2000);
}
    Serial.println("");
    Serial.println("Mit Wifi verbunden");
    Serial.println("IP Adresse: ");
    Serial.println(WiFi.localIP());

}

/////////////////////////////////////////////////////////////////////////// SETUP
void setup() {

  // Serielle Kommunikation starten
  Serial.begin(115200);

// Wifi setup
wifi_setup();

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


ausrichten_oben = (sensorSonne_NW + sensorSonne_NO)/2; // Ausrichten oben
ausrichten_unten = (sensorSonne_SW + sensorSonne_SO)/2; // Ausrichten unten
ausrichten_links = (sensorSonne_NW + sensorSonne_SW) / 2; // Ausrichten links
ausrichten_rechts = (sensorSonne_NO + sensorSonne_SO) / 2; // Ausrichten rechts

// Differenz ermitteln
int differenz_neigen = ausrichten_oben - ausrichten_unten; // Prüfe Differenz Neigen
Serial.print("Differenz Neigen: ");
Serial.println(differenz_neigen);
int differenz_drehen = ausrichten_links - ausrichten_rechts;// Prüfe Differenz Drehen
Serial.print("Differenz Drehen: ");
Serial.println(differenz_drehen);

// Quersumme aller Werte
sonne_quersumme = (sensorSonne_NO + sensorSonne_NW + sensorSonne_SO + sensorSonne_SO) / 4;

Serial.print("Sonne Quersumme: ");
Serial.println(sonne_quersumme);
Serial.print("Sonne Quersumme max Wert ");
Serial.println(traker_wolken);


// Justierung stoppen Wolken
if (sonne_quersumme > traker_wolken) {
  // Zu viele Wolken keine Regelung
  Serial.println("Quersumme zu hoch! Keine Steuerung!");

    // Panele auf Nachstellung bringen
    if (sonne_quersumme > 3850) 
    {
        //Panele in Nachtstellung fahren
        Serial.println("Panele in Nachtstellung fahren");
        m1(2);
        delay(50000);
        m1(3);
    }

} else {


        // Bewegung ermitteln Neigen (-1*traker_t

        if (-1*traker_tolleranz > differenz_neigen || differenz_neigen > traker_tolleranz) 
        {
          if (ausrichten_oben > ausrichten_unten)
          {
            // 
            Serial.println("Motor NEIGEN - unten fahren");
            m1(1);


          }
          else if (ausrichten_oben < ausrichten_unten)
          {
            //
            Serial.println("Motor NEIGEN - oben fahren");
            m1(2);


          }
        } else {

          m1(3);

        }

        // Bewegung ermitteln Drehen

        if (-1*traker_tolleranz > differenz_drehen || differenz_drehen > traker_tolleranz) 
        {
          if (ausrichten_links > ausrichten_rechts)
          {
            // 
            Serial.println("Motor DREHEN - rechts fahren");
            m2(2);
            }
          else if (ausrichten_links < ausrichten_rechts)
          {
            //
            Serial.println("Motor DREHEN - links fahren");
            m2(1);
          }
        } else {
          m2(3);
        }


}

}

/////////////////////////////////////////////////////////////////////////// NTP Local Time
void LokaleZeit(){
  struct tm timeinfo;
  if(!getLocalTime(&timeinfo)){
    Serial.println("Failed to obtain time");
    return;
  }
  Serial.println(&timeinfo, "%A, %B %d %Y %H:%M:%S");
 /* Serial.print("Day of week: ");
  Serial.println(&timeinfo, "%A");
  Serial.print("Month: ");
  Serial.println(&timeinfo, "%B");
  Serial.print("Day of Month: ");
  Serial.println(&timeinfo, "%d");
  Serial.print("Year: ");
  Serial.println(&timeinfo, "%Y");
  Serial.print("Hour: ");
  Serial.println(&timeinfo, "%H");
  Serial.print("Hour (12 hour format): ");
  Serial.println(&timeinfo, "%I");
  Serial.print("Minute: ");
  Serial.println(&timeinfo, "%M");
  Serial.print("Second: ");
  Serial.println(&timeinfo, "%S");

  Serial.println("Time variables");
  char timeHour[3];
  strftime(timeHour,3, "%H", &timeinfo);
  Serial.println(timeHour);
  char timeWeekDay[10];
  strftime(timeWeekDay,10, "%A", &timeinfo);
  Serial.println(timeWeekDay);
  Serial.println();
  */
}

/////////////////////////////////////////////////////////////////////////// m1 Neigen
void m1(int x) {
  // x = 1 senken | x = 2 heben | x = 3 aus

  if (x == 1) {
    // Panele senken
    Serial.println("Panele senken");
    digitalWrite(M1_re, HIGH);
    digitalWrite(M1_li, LOW);
  }

  if (x == 2) {
    // Panele heben
    Serial.println("Panele heben");
    digitalWrite(M1_re, LOW);
    digitalWrite(M1_li, HIGH);
  }

  if (x == 3) {
    // Neigen stop
    Serial.println("Neigen stop");
    digitalWrite(M1_re, LOW);
    digitalWrite(M1_li, LOW);
  }  

}

/////////////////////////////////////////////////////////////////////////// m1 Drehen
void m2(int x) {
  // x = 1 links | x = 2 rechts | x = 3 aus

  if (x == 1) {
    // Panel links drehen
    Serial.println("Panel links drehen");
    digitalWrite(M2_re, HIGH);
    digitalWrite(M2_li, LOW);
  }

  if (x == 2) {
    // Panel rechts drehen
    Serial.println("Panel rechts drehen");
    digitalWrite(M2_re, LOW);
    digitalWrite(M2_li, HIGH);
  }

  if (x == 3) {
    // Drehen stop
    Serial.println("Drehen stop");
    digitalWrite(M2_re, LOW);
    digitalWrite(M2_li, LOW);
  }  

}

/////////////////////////////////////////////////////////////////////////// Sturmschutz - Solarpanel waagerecht ausrichten 
void sturmschutz() {

  // Prüfen auf Messwert des Sensors

  // Wenn zu stark wind_zu_stark auf 1 setzen

  // Motor m1 Panel waagerecht ausrichten bis Endlage
  m1(1);

  // Motor m2 Panel drehen Osten bis Endlage
  m2(2); 

}

/////////////////////////////////////////////////////////////////////////// Schneelast / Reinigen - Solarpanel senkrecht ausrichten 
void panel_senkrecht() {

  // Motor m1 Panel senkrecht ausrichten bis Endlage
  m1(2);

  // Motor m2 Panel drehen Osten bis Endlage
  m2(2); 

}

/////////////////////////////////////////////////////////////////////////// Sonnenaufgang - Panele ausrichten 
void sonnenaufgang() {

  // Motor m1 Panel senkrecht ausrichten bis Endlage
  m1(1);

  // Motor m2 Panel drehen Osten bis Endlage
  m2(1); 

}

/////////////////////////////////////////////////////////////////////////// LOOP
void loop() {
/*
  //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ Auf Sturm prüfen
  if (millis() - previousMillis_Sturmcheck > interval_Sturmcheck) {
      previousMillis_Sturmcheck = millis(); 
      // Windstärke prüfen
      Serial.println("Windstärke prüfen");
    }
*/

  //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ Auf Sturm prüfen
  if (millis() - previousMillis_sonnensensor > interval_sonnensensor) {
      previousMillis_sonnensensor = millis(); 
      // Sonnenposition prüfen wenn windstärke okay
      if (wind_zu_stark != 1) {
      //Serial.println("Position der Sonne prüfen.");
      sonnensensor();
      } else {
        Serial.println("Keine Ausrichtung, da Wind zu stark!");
      }
      
    }

//m1(2);
}