// Arctos CAN

#include <Arduino.h>
#include "Arctos.h"

#include "myCAN.h"
#include "myDisplay.h"

#include "myServo.h"
#include "myCANServer.h"

#include <WiFi.h>
#include "AutoConnect.h"
#include "MyWebServer.h"
extern AutoConnect portal;

String GetMyNodeName(void) {
  // die MAC Adresse wird aus einer Chip-Kennzahl und der ChipID gebildet
  String nodeName = "Arctos_" + WiFi.macAddress();
  nodeName.replace(":", "");
  return nodeName;
}

void outputChanges(){
  if(actCANdata.servo1Changed){
    servo1.write(actCANdata.servo1);
    lineVal[2] = String(actCANdata.servo1);
    drawDisplay();                        // und aufs Display
    actCANdata.servo1Changed = false;
  }
}

void setup(){
  Serial.begin(115200);
  Serial.println("Arctos CAN 1.0");
  
  Serial.println("starting Autoconnect");
  setupAutoconnect();

  setupServo();
  setupDisplay();
  setupCAN();
  setupCANServer();

  drawDisplay(); // einmal die aktuellen Werte schreiben
  Serial.println("setup() complete");
}

void loop(){
  loopCAN();
  outputChanges();

  portal.handleClient();
  checkForCANServerConnections(); // evtl. neue Verbindung aufbauen
  sl2can();
}

