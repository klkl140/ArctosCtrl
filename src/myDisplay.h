#include <Arduino.h>
#include "heltec.h"

#pragma once

// this is shown on the display
const String lineName[3]  = {"", "", "PWM:" };
String lineVal[3]         = {"", "", "none" };

//Display Größe und Formatierung
int dispSizeX = 128;
int dispSizeY = 64;
int frameSize = 4;                                                // aeusserer Rand auf dem Display
int lineSpacing = 16 + (dispSizeY - 2 * frameSize - 3 * 16) / 2;  // Zeilenabstand berechnen, Texthoehe 16px

SSD1306Wire display(0x3c, SDA_OLED, SCL_OLED, RST_OLED);

void setupDisplay(void){
  delay(100);
  display.init();
  display.setTextAlignment(TEXT_ALIGN_CENTER);
  display.setContrast(255);
  display.screenRotate(ANGLE_0_DEGREE);
  display.setFont(ArialMT_Plain_16);     // Schriftart 16px hoch

  //startup screen
  display.clear();
  display.drawString(64, 32 - 16 / 2, "Starting...");
  display.display();
  delay(500);
}

void drawDisplay() {
  //Ausgabe auf Display
  display.clear();
  display.setTextAlignment(TEXT_ALIGN_LEFT);  //LINKS: Begriffe schreiben
  display.drawString(frameSize, frameSize, lineName[0]);
  display.drawString(frameSize, frameSize + lineSpacing, lineName[1]);
  display.drawString(frameSize, frameSize + lineSpacing * 2, lineName[2]);
  display.setTextAlignment(TEXT_ALIGN_RIGHT);  //RECHTS: Werte schreiben
  display.drawString(dispSizeX - frameSize, frameSize, lineVal[0]);
  display.drawString(dispSizeX - frameSize, frameSize + lineSpacing, lineVal[1]);
  display.drawString(dispSizeX - frameSize, frameSize + lineSpacing * 2, lineVal[2]);
  display.display();
}

