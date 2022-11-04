#include <Arduino.h>
#include <PubSubClient.h>  
#include "WiFi.h"

/////////////////////////////////////////////////////////////////////////// mqtt variable
char msgToPublish[60];
char stgFromFloat[10];


/////////////////////////////////////////////////////////////////////////// Pin output zuweisen
#define M1_re 2   // D2  - grau weiss - Pin 7
#define M1_li 4   // D4  - grün - Pin 6
#define M2_re 5   // D5  - violett - Pin 5
#define M2_li 18  // D18 - schwarz - Pin 4

/////////////////////////////////////////////////////////////////////////// Interrup
int sturmschutzschalterpin =  13;
int panelsenkrechtpin =  12;

/////////////////////////////////////////////////////////////////////////// ADC zuweisen
const int adc_NO = 34; //ADC1_6 - Fotowiderstand 
const int adc_NW = 35; //ADC1_7 - Fotowiderstand 
const int adc_SO = 33; //ADC1_8 - Fotowiderstand
const int adc_SW = 32; //ADC1_9 - Fotowiderstand 

/////////////////////////////////////////////////////////////////////////// ADC Strommessung ACS712
const int acd_strom = 39; //Strommesser ACD Panel 
int acd_strom_acs712;

/////////////////////////////////////////////////////////////////////////// Variablen Sonensensor
int sensorSonne_NO, sensorSonne_NW, sensorSonne_SO, sensorSonne_SW;
int horizontal_hoch, horizontal_runter, vertikal_rechts, vertikal_links; 
int differenz_neigen, differenz_drehen, sonne_quersumme, neigen_fahrt;
int traker_tolleranz_neigen = 90; // Getestet mit 300
int traker_tolleranz_drehen = 110;
int helligkeit_schwellwert = 750; // Wolkenschwellwert
int helligkeit_nachtstellung = 1600; // Wolkenschwellwert

/////////////////////////////////////////////////////////////////////////// Windsensor Variablen
int wind_zu_stark = 0;
int sturmschutz_pause = 150000;
int pin_anemometer = 23; // Impulsgeber des Anemometer
unsigned long start_time = 0;
unsigned long end_time = 0
;
int steps = 0;
int steps_schwellwert = 120;

/////////////////////////////////////////////////////////////////////////// Schleifen verwalten
unsigned long previousMillis_Sturmcheck = 0; // Windstärke prüfen
unsigned long interval_Sturmcheck = 5000; 

unsigned long previousMillis_sonnensensor = 0; // Sonnenstand prüfen
unsigned long interval_sonnensensor = 10000; 

unsigned long previousMillis_sturmschutzschalter = 0; // Sturmschutz Schalter prüfen
unsigned long interval_sturmschutzschalter = 1200; 

unsigned long previousMillis_panelsenkrecht = 0; // Sturmschutz Schalter prüfen
unsigned long interval_panelsenkrecht = 1300; 

unsigned long previousMillis_strom_messung = 0; // Sturmschutz Schalter prüfen
unsigned long interval_strom_messung = 35000; 

/////////////////////////////////////////////////////////////////////////// Funktionsprototypen
void loop                       ();
void m1                         (int); // Panel neigen
void m2                         (int); // Panel drehen
void sturmschutz                ();
void panel_senkrecht            ();
void sonnenaufgang              ();
void sonnensensor               ();
void sturmschutzschalter        ();
void callback                   (char* topic, byte* payload, unsigned int length);
void reconnect                  ();

/////////////////////////////////////////////////////////////////////////// Kartendaten 
const char* kartenID = "Solarmodul_001_Panele";

/////////////////////////////////////////////////////////////////////////// MQTT 
WiFiClient espClient;
PubSubClient client(espClient);

const char* mqtt_server = "192.168.150.1";

/////////////////////////////////////////////////////////////////////////// SETUP - Wifi
void wifi_setup() {

// WiFi Zugangsdaten
const char* WIFI_SSID = "GuggenbergerLinux";
const char* WIFI_PASS = "Isabelle2014samira";

// Static IP
IPAddress local_IP(192, 168, 13, 51);
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

//****************************************************************************************** VOID mqtt reconnected
void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    //Serial.print("Baue Verbindung zum mqtt Server auf. IP: ");
    // Attempt to connect
    if (client.connect(kartenID,"zugang1","43b4134735")) {
      //Serial.println("connected");
      ////////////////////////////////////////////////////////////////////////// SUBSCRIBE Eintraege
      //client.subscribe("relais_licht_wohnzimmer_1_0/IN");

    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

/////////////////////////////////////////////////////////////////////////// MQTT callback
void callback(char* topic, byte* payload, unsigned int length) {
/*
  /////////////////////////////////////////////////////////////////////////// Relais 0
      if (strcmp(topic,"relais_licht_wohnzimmer_1_0/IN")==0) {

          // ON und OFF Funktion auslesen
          if ((char)payload[0] == 'o' && (char)payload[1] == 'n') {  
                  Serial.println("relais_0 -> AN");

                }

          if ((char)payload[0] == 'o' && (char)payload[1] == 'f' && (char)payload[2] == 'f') {  
                  Serial.println("relais_0 -> AUS");

                }
        } 
  */
}


/////////////////////////////////////////////////////////////////////////// SETUP
void setup() {

// Serielle Kommunikation starten
Serial.begin(38400);

// Wifi setup
wifi_setup();

// MQTT Broker
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);

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
//sonne_quersumme = (sensorSonne_NO + sensorSonne_NW + sensorSonne_SO + sensorSonne_SW) / 4;
sonne_quersumme = (sensorSonne_SO + sensorSonne_SW) / 2;

int quersumme_mqtt = 4096 - sonne_quersumme;

      // mqtt Datensatz senden
    dtostrf(quersumme_mqtt, 4, 0, stgFromFloat);
    sprintf(msgToPublish, "%s", stgFromFloat);
    client.publish("Solarpanel/001/sonnenQuersumme", msgToPublish);


Serial.print("Sonne Quersumme: ");
Serial.println(sonne_quersumme);
Serial.print("Sonne Quersumme max Wert ");
Serial.println(helligkeit_schwellwert);

// Justierung stoppen Wolken
if (sonne_quersumme < helligkeit_schwellwert) {
Serial.println("Helligkeit - Ausrichten ");
//horizontal_hoch   = ((sensorSonne_NO + sensorSonne_NW)/2)*(-1);
horizontal_hoch   = ((sensorSonne_NO + sensorSonne_NW)/2)*(-1);
horizontal_runter = ((sensorSonne_SO + sensorSonne_SW)/2)*(-1);
vertikal_rechts   = ((sensorSonne_NO + sensorSonne_SO)/2)*(-1);
vertikal_links    = ((sensorSonne_NW + sensorSonne_SW)/2)*(-1);

    // Sonnentraking Drehen
    differenz_drehen = ((sensorSonne_NW + sensorSonne_SW)/2  - (sensorSonne_NO + sensorSonne_SO)/2) *(-1);
    Serial.print("Differenz Drehen: ");
    Serial.println(differenz_drehen);


    if (vertikal_rechts > vertikal_links && (vertikal_rechts-vertikal_links) > traker_tolleranz_drehen) {
      Serial.println("XXX Motor drehen - LINKS");
      client.publish("Solarpanel/001/meldung", "Drehe ... links");
      m2(1); 
      delay(1000);
    } else {
      m2(3);
    }
    
    if (vertikal_links > vertikal_rechts && (vertikal_links-vertikal_rechts) > traker_tolleranz_drehen) {
      Serial.println("XXXX Motor drehen - RECHTS");
      client.publish("Solarpanel/001/meldung", "Drehe ... rechts");
      m2(2); 
    delay(1000);
    } else {
      m2(3);
    }

    // Schwellwert überschritten Ausrichten
    // Sonnentraking Neigen
    differenz_neigen = (((sensorSonne_NO + sensorSonne_NW)/2) - ((sensorSonne_SO + sensorSonne_SW)/2)) * (-1);
    Serial.print("Differenz Neigen: ");
    Serial.println(differenz_neigen);


    if (horizontal_hoch > horizontal_runter && (horizontal_hoch-horizontal_runter) > traker_tolleranz_neigen) {
          Serial.println("YYYY Motor neigen - RUNTER");
          client.publish("Solarpanel/001/meldung", "Neigen ... runter");
          m1(2); 
          // delay(1000);
    } else {
      m1(3);
    }
 
    if (horizontal_runter > horizontal_hoch && (horizontal_runter-horizontal_hoch) > traker_tolleranz_neigen) {
          Serial.println("YYYY Motor neigen - HOCH");
          client.publish("Solarpanel/001/meldung", "Neigen ... hoch");
          m1(1); 
          // delay(1000);      
    } else {
      m1(3);
    }


} else {
Serial.println("Helligkeit - Nichts tun ");
client.publish("Solarpanel/001/meldung", "Sonneneinstrahlung zu gering");

// Wenn die Helligkeit dunkler als der Nachtwert ist Platten horizontal stellen

  if (sonne_quersumme > helligkeit_nachtstellung) { 
  // Platte horizontal stellen
    Serial.println("Platten Nachtstellung");
    client.publish("Solarpanel/001/meldung", "Nachtstellung!");
    m1(2);
    m2(1);
  } else {
    Serial.println("Zu wenig Sonne, keine Bewegung");
    client.publish("Solarpanel/001/meldung", "Sonneneinstrahlung zu gering");

  }

}

}

/////////////////////////////////////////////////////////////////////////// m1 Neigen
void m1(int x) {
  // x = 1 senken | x = 2 heben | x = 3 aus

  if (x == 1) {
    // Panele senken
    //Serial.println("Panele senken");
    digitalWrite(M1_re, HIGH); // D2 - weiss - Pin7
    digitalWrite(M1_li, LOW); // D4 - grün + Pin 6
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
    digitalWrite(M2_re, HIGH); // D5 - violett - Pin 5
    digitalWrite(M2_li, LOW); // D18 - schwarz - Pin 4
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
        client.publish("Solarpanel/001/meldung", "Panele senkrecht");
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
        client.publish("Solarpanel/001/meldung", "Sturmschutzschalter aktiv");
        m1(2);
        delay(500);
      }

}

/////////////////////////////////////////////////////////////////////////// LOOP
void loop() {
/*
  // MQTT Server kontaktieren
  if (!client.connected()) {
  reconnect();
  }
  client.loop();
*/

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
       sonnensensor();
      } else {
        Serial.println("Keine Ausrichtung, da Wind zu stark!");
        client.publish("Solarpanel/001/meldung", "Wind zu stark!");
      }
  
    }

    delay (300);
}