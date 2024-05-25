// ArctosCtrl

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
  String nodeName = "Arctos_" + WiFi.macAddress();
  nodeName.replace(":", "");
  return nodeName;
}

void outputChanges(){
  if(actCANdata.servo1Changed){
    servo1.write(actCANdata.servo1);
    lineVal[2] = String(actCANdata.servo1);
    drawDisplay();
    actCANdata.servo1Changed = false;
  }
}

void setup(){
  Serial.begin(115200);
  Serial.println("ArctosCtrl 1.0");
  
  Serial.println("starting Autoconnect");
  setupAutoconnect();

  setupServo();
  setupDisplay();
  setupCAN();
  setupCANServer();

  drawDisplay(); // show the actual values once
  Serial.println("setup() complete");
}

void loop(){
  loopCAN();
  outputChanges();

  portal.handleClient();
  checkForCANServerConnections(); // maybe make a new connection
  sl2can();
}

