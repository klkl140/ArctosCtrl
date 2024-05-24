#include <CAN.h>

// first setup
void setupCANServer();

//serves a request for a new CAN connection
void checkForCANServerConnections();

// send a msg via CAN in slcan format if connected
void sendCANtoClient(CAN_frame_t frame, int *tstamp=0);

// get a CAN msg from the Server in slcan format and send it to CAN
int sl2can();

// for testing
void SendHalloValue();
void EchoReceivedData();

