#include <Arduino.h>

#include "myCAN.h"
#include "myCANServer.h"

CANdataT actCANdata={0};
CAN_device_t CAN_cfg;

void setupCAN(void){
  CAN_cfg.speed = (CAN_speed_t)(CAN_SPEED_500KBPS*2);  //Lora V2 sendet mit halber Fequenz, desshalb hier gerade verdoppelt.(liegt Nicht an CPU Frequenz!)
  CAN_cfg.tx_pin_id = GPIO_NUM_14;      //pin 5 hat bei Lora V2 Probleme mit dem Display
  CAN_cfg.rx_pin_id = GPIO_NUM_13;      //bei pin4 Probleme mit dem Display
  CAN_cfg.rx_queue = xQueueCreate(rx_queue_size, sizeof(CAN_frame_t));
  ESP32Can.CANInit();
}

// gibt true zurueck wenn sich die PWM veraendert hat
void workOnCAN(CAN_frame_t *rx_frame){
  if(rx_frame->MsgID == 0x007){ // PWM
      if( (rx_frame->FIR.B.DLC==0) || (rx_frame->FIR.B.RTR == CAN_RTR) ){
        // leere oder RTR messages werden mit dem aktuellen Wert beantwortet
        CAN_frame_t tx_frame={0}; // initialisierung ist wichtig, sonst sind die Bits zufaellig
        tx_frame.MsgID = 0x007;
        tx_frame.FIR.B.DLC = 1;
        tx_frame.data.u8[0] = actCANdata.servo1;
        tx_frame.data.u8[1] = 0x00;
        tx_frame.data.u8[2] = 0x00;
        tx_frame.data.u8[3] = 0x00;
        tx_frame.data.u8[4] = 0x00;
        tx_frame.data.u8[5] = 0x00;
        tx_frame.data.u8[6] = 0x00;
        tx_frame.data.u8[7] = 0x00;
        ESP32Can.CANWriteFrame(&tx_frame);
      } else {
        // set the PWM
        actCANdata.servo1 = rx_frame->data.u8[0];
        actCANdata.servo1Changed=true;
        printf("recv PWM=%d\n\r",rx_frame->data.u8[0]);
      }
  }
}

void loopCAN(void){
  unsigned long currentMillis = millis();
  static unsigned long previousMillis = 0;  // will store last time a CAN Message was send

  // Receive next CAN frame from queue
  CAN_frame_t rx_frame;
  if (xQueueReceive(CAN_cfg.rx_queue, &rx_frame, 3 * portTICK_PERIOD_MS) == pdTRUE) {
    //printCANframe(&rx_frame);
  
    workOnCAN(&rx_frame);

    sendCANtoClient(rx_frame);  // alles empfangene evtl an einen Client senden
  }
  
  /* // regelmaessig etwas senden
  const int kInterval = 1000;
  if (currentMillis - previousMillis >= kInterval) {
    previousMillis = currentMillis;
    CAN_frame_t tx_frame;
    tx_frame.FIR.B.FF = CAN_frame_std;
    tx_frame.MsgID = 0x111;
    tx_frame.FIR.B.DLC = 8;
    tx_frame.data.u8[0] = 0x00;
    tx_frame.data.u8[1] = 0x01;
    tx_frame.data.u8[2] = 0x02;
    tx_frame.data.u8[3] = 0x03;
    tx_frame.data.u8[4] = 0x04;
    tx_frame.data.u8[5] = 0x05;
    tx_frame.data.u8[6] = 0x06;
    tx_frame.data.u8[7] = 0x07;
    ESP32Can.CANWriteFrame(&tx_frame);
  }*/
}

void printCANframe(CAN_frame_t *rx_frame){
    if (rx_frame->FIR.B.FF == CAN_frame_std) {
      printf("New standard frame");
    } else {
      printf("New extended frame");
    }

    if (rx_frame->FIR.B.RTR == CAN_RTR) {
      printf(" RTR from 0x%08X, DLC %d\r\n", rx_frame->MsgID, rx_frame->FIR.B.DLC);
    } else {
      printf(" from 0x%08X, DLC %d, Data ", rx_frame->MsgID, rx_frame->FIR.B.DLC);
      for (int i = 0; i < rx_frame->FIR.B.DLC; i++) {
        printf("0x%02X ", rx_frame->data.u8[i]);
      }
      printf("\n");
    }
}