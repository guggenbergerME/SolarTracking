#include <Arduino.h>
#include <ArduinoOTA.h>



/////////////////////////////////////////////////////////////////////////// Funktionsprototypen
//void callback                (char*, byte*, unsigned int);
void loop                      ();



/////////////////////////////////////////////////////////////////////////// SETUP
void setup() {

 //OTA Setup für Firmware
  ArduinoOTA.setHostname("SolarTracking");
  ArduinoOTA.setPassword(")Vp-gt.qdMwdF>s{fc$9");
  ArduinoOTA.begin();

  // Serielle Kommunikation starten
  Serial.begin(115200);





}



/////////////////////////////////////////////////////////////////////////// LOOP
void loop() {

  // OTA Handle starten
  ArduinoOTA.handle();  


}