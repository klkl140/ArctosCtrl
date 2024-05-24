#include <ESP32CAN.h>               // Library: https://github.com/miwagner/ESP32-Arduino-CAN
                                    // besser waere wohl
                                    // https://github.com/sdp8483/ESP32-Arduino-CAN weil aktuellerer fork
                                    // aber alles ist zu alt für die ESP32-C3
#include <CAN_config.h>

// CAN information we are interested in
typedef struct CANdataT{
    int servo1;
    bool servo1Changed;     // is set when a new value arrives, could be cleared when it was processed
}CANdataT;
extern CANdataT actCANdata;

extern CAN_device_t CAN_cfg;        // the connection to CAN
const int rx_queue_size = 10;       // receive Queue size

// call it once
void setupCAN(void);

// the data of a CAN msg should be set to the device
void workOnCAN(CAN_frame_t *rx_frame);

// print one CAN msg to Serial
void printCANframe(CAN_frame_t *rx_frame);

// show be called regularly
void loopCAN(void);