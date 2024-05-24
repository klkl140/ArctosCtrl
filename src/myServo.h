#include <ESP32Servo.h>

#pragma once

Servo servo1;
int minUs = 500;
int maxUs = 2000;
int servo1Pin = 25;

void setupServo(void){
    ESP32PWM::allocateTimer(0);
	ESP32PWM::allocateTimer(1);
	ESP32PWM::allocateTimer(2);
	ESP32PWM::allocateTimer(3);
	
    servo1.setPeriodHertz(50);      // Standard 50hz servo
    printf("attach servo to %d", servo1Pin);
    servo1.attach(servo1Pin, minUs, maxUs);
    printf(", done\r\n");    
}
