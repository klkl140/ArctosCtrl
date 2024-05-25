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

// returns true if the PWM has changed
void workOnCAN(CAN_frame_t *rx_frame){
  if(rx_frame->MsgID == 0x007){ // a command for the gripper
      if( (rx_frame->FIR.B.DLC==0) || (rx_frame->FIR.B.RTR == CAN_RTR) ){
        // empty or RTR messages are returned with the actual value
        CAN_frame_t tx_frame={0}; // init of frame is important, otherwhise the bit are random
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

    sendCANtoClient(rx_frame);  // all received msgs might be send to a connected client
  }
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