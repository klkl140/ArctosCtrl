//https://www.megunolink.com/articles/wireless/talk-esp32-over-wifi/
#include <Arduino.h>

#include "myCAN.h"
#include "myCANServer.h"
#include "WIFIServer.h"

const uint ServerPort = 2323;
WiFiServer Server(ServerPort);
 
void setupCANServer(){
  Server.begin();
}

WiFiClient RemoteClient;
bool isConnected = false;
 
void checkForCANServerConnections(){
  if (Server.hasClient()){
    // If we are already connected to another computer, 
    // then reject the new connection. Otherwise accept the connection. 
    if (RemoteClient.connected()){
      Serial.println("Connection rejected");
      Server.available().stop();
    } else {
      Serial.println("Connection accepted");
      RemoteClient = Server.available();
      isConnected = true;
    }
  } else {
    if(!RemoteClient.connected() && isConnected){
        Serial.println("connection lost");
        isConnected = false;
    }
 }
}

void SendHalloValue(){
  if (RemoteClient.connected()){ 
    RemoteClient.println("hallihallo");
  }
}

void EchoReceivedData(){
  uint8_t receiveBuffer[30];
  while (RemoteClient.connected() && RemoteClient.available()){
    int received = RemoteClient.read(receiveBuffer, sizeof(receiveBuffer));
    Serial.println("received " + String(received) + " Bytes");
    RemoteClient.write(receiveBuffer, received);
    Serial.println("written");
  }
}

/* read CAN frames from CAN interface and write it to the pty */
// from https://github.com/linux-can/can-utils/blob/master/slcanpty.c
void sendCANtoClient(CAN_frame_t frame, int *tstamp){
	char cmd;   // the string to send
#define SLC_MTU (sizeof("T1111222281122334455667788EA5F\r") + 1)    // maximale Variante
	char buf[SLC_MTU];
	
	/* convert to slcan ASCII frame */
	if (frame.FIR.B.RTR)
		cmd = 'R'; /* becomes 'r' in SFF format */
	else
		cmd = 'T'; /* becomes 't' in SFF format */

	if (frame.FIR.B.FF){
		sprintf(buf, "%c%08X%d", cmd, frame.MsgID, frame.FIR.B.DLC);
    } else {
		sprintf(buf, "%c%03X%d", cmd | 0x20, frame.MsgID, frame.FIR.B.DLC);
    }
	
    int ptr = strlen(buf);
	for (int i = 0; i < frame.FIR.B.DLC; i++){
		sprintf(&buf[ptr + 2 * i], "%02X", frame.data.u8[i]);
    }
	/* timestamp noch nicht implementiert
    if (*tstamp) {
		struct timeval tv;

		if (ioctl(socket, SIOCGSTAMP, &tv) < 0)
			perror("SIOCGSTAMP");

		sprintf(&buf[ptr + 2*frame.FIR.B.DLC], "%04llX",
			(unsigned long long)(tv.tv_sec%60)*1000 + tv.tv_usec/1000);
	}*/

	strcat(buf, "\r\n");
    if(RemoteClient.connected()){
        RemoteClient.write(buf, strlen(buf));
        if(0){  //debug
            Serial.print("send:");
            Serial.print(buf);
        }
    }
 }

unsigned char asc2nibble(char c){
	if ((c >= '0') && (c <= '9'))
		return c - '0';
	if ((c >= 'A') && (c <= 'F'))
		return c - 'A' + 10;
	if ((c >= 'a') && (c <= 'f'))
		return c - 'a' + 10;
	return 16; /* error */
}

// write received slcan msg to CAN-bus
// from https://github.com/linux-can/can-utils/blob/master/slcanpty.c
/* read data from Server, send CAN frames to CAN socket and answer commands */
// gibt bei erfolg 0 zurueck, sonst 1
int sl2can()
{
	unsigned int nbytes, tmp;
	char cmd;
	static char buf[200];
	char replybuf[10]; /* for answers to received commands */
	unsigned int ptr;
	int *is_open;       // Port offen oder geschlossen, noch nicht benutzt
    int *tstamp;        // Timestamp ein/aus, noch nicht benutzt
    //struct can_filter *fi;
    CAN_frame_t frame;   //der wird evtl. gesendet
    
    int ret, i;
	static unsigned int rxoffset = 0; /* points to the end of an received incomplete SLCAN message */

#if 0
	ret = read(pty, &buf[rxoffset], sizeof(buf) - rxoffset - 1);
	/* ret == 0 : no error but pty descriptor has been closed */
#else
    if(!RemoteClient.connected()){
        return 1;
    }
    ret = RemoteClient.read((uint8_t*)&buf[rxoffset], (size_t)(sizeof(buf) - rxoffset - 1));
#endif
	if (ret <= 0) {
		if (ret < 0)
			perror("read pty");

		return 1;
	}

	Serial.print("received " + String(ret) + " Bytes :");
    for(int lauf=0; lauf<ret; lauf++){Serial.print(buf[rxoffset+lauf]);}
    Serial.println();
    nbytes = ret;
	/* reset incomplete message offset */
	nbytes += rxoffset;
	rxoffset = 0;

rx_restart:
	/* remove trailing '\r' characters to be robust against some apps */
	while (buf[0] == '\r' && nbytes > 0) {
		for (tmp = 0; tmp < nbytes; tmp++)
			buf[tmp] = buf[tmp + 1];
		nbytes--;
	}

	if (!nbytes)
		return 0;

	/* check if we can detect a complete SLCAN message including '\r' */
	for (tmp = 0; tmp < nbytes; tmp++) {
		if (buf[tmp] == '\r')
			break;
	}

	/* no '\r' found in the message buffer? */
	if (tmp == nbytes) {
		/* save incomplete message */
		rxoffset = nbytes;

		/* leave here and read from pty again */
		return 0;
	}

	cmd = buf[0];
	buf[nbytes] = 0;

	/* check for filter configuration commands */
	if (cmd == 'm' || cmd == 'M') {
		buf[9] = 0; /* terminate filter string */
		ptr = 9;
		goto rx_out_ack;
	}

	/* check for timestamp on/off command */
	if (cmd == 'Z') {
		*tstamp = buf[1] & 0x01;
		ptr = 2;
		goto rx_out_ack;
	}

	/* check for 'O'pen command */
	if (cmd == 'O') {
		//setsockopt(socket, SOL_CAN_RAW, CAN_RAW_FILTER, fi, sizeof(struct can_filter));
		ptr = 1;
		*is_open = 1;
		goto rx_out_ack;
	}

	/* check for 'C'lose command */
	if (cmd == 'C') {
		//setsockopt(socket, SOL_CAN_RAW, CAN_RAW_FILTER, NULL, 0);
		ptr = 1;
		*is_open = 0;
		goto rx_out_ack;
	}

	/* check for 'V'ersion command */
	if (cmd == 'V') {
		sprintf(replybuf, "V1013\r");
		tmp = strlen(replybuf);
		ptr = 1;
		goto rx_out;
	}
	/* check for 'v'ersion command */
	if (cmd == 'v') {
		sprintf(replybuf, "v1014\r");
		tmp = strlen(replybuf);
		ptr = 1;
		goto rx_out;
	}

	/* check for serial 'N'umber command */
	if (cmd == 'N') {
		sprintf(replybuf, "N4242\r");
		tmp = strlen(replybuf);
		ptr = 1;
		goto rx_out;
	}

	/* check for read status 'F'lags */
	if (cmd == 'F') {
		sprintf(replybuf, "F00\r");
		tmp = strlen(replybuf);
		ptr = 1;
		goto rx_out;
	}

	/* correctly answer unsupported commands */
	if (cmd == 'U') {
		ptr = 2;
		goto rx_out_ack;
	}
	if (cmd == 'S') {
		ptr = 2;
		goto rx_out_ack;
	}
	if (cmd == 's') {
		ptr = 5;
		goto rx_out_ack;
	}
	if (cmd == 'P' || cmd == 'A') {
		ptr = 1;
		goto rx_out_nack;
	}
	if (cmd == 'X') {
		ptr = 2;
		if (buf[1] & 0x01)
			goto rx_out_ack;
		else
			goto rx_out_nack;
	}

	/* catch unknown commands */
	if ((cmd != 't') && (cmd != 'T') && (cmd != 'r') && (cmd != 'R')) {
		ptr = nbytes - 1;
		goto rx_out_nack;
	}

	if (cmd & 0x20) /* tiny chars 'r' 't' => SFF */
		ptr = 4; /* dlc position tiiid */
	else
		ptr = 9; /* dlc position Tiiiiiiiid */

	memset(&frame.data, 0, 8); /* clear data[] */

	if ((cmd | 0x20) == 'r' && buf[ptr] != '0') {
		/*
		 * RTR frame without dlc information!
		 * This is against the SLCAN spec but sent
		 * by a commercial CAN tool ... so we are
		 * robust against this protocol violation.
		 */

		frame.FIR.B.DLC = buf[ptr]; /* save following byte */

		buf[ptr] = 0; /* terminate can_id string */

		frame.MsgID = strtoul(buf + 1, NULL, 16);
		frame.FIR.B.RTR = CAN_RTR;

		if (!(cmd & 0x20)) /* NO tiny chars => EFF */
			frame.FIR.B.FF = CAN_frame_ext;

		buf[ptr] = frame.FIR.B.DLC; /* restore following byte */
		frame.FIR.B.DLC = 0;
		ptr--; /* we have no dlc component in the violation case */

	} else {
		if (!(buf[ptr] >= '0' && buf[ptr] < '9'))
			goto rx_out_nack;

		frame.FIR.B.DLC = buf[ptr] - '0'; /* get dlc from ASCII val */

		buf[ptr] = 0; /* terminate can_id string */

		frame.MsgID = strtoul(buf + 1, NULL, 16);

		if (!(cmd & 0x20)) /* NO tiny chars => EFF */
			frame.FIR.B.FF = CAN_frame_ext;

		if ((cmd | 0x20) == 'r') /* RTR frame */
		    frame.FIR.B.RTR = CAN_RTR;

		for (i = 0, ptr++; i < frame.FIR.B.DLC; i++) {
			tmp = asc2nibble(buf[ptr++]);
			if (tmp > 0x0F)
				goto rx_out_nack;
			frame.data.u8[i] = (tmp << 4);
			tmp = asc2nibble(buf[ptr++]);
			if (tmp > 0x0F)
				goto rx_out_nack;
			frame.data.u8[i] |= tmp;
		}
		/* point to last real data */
		if (frame.FIR.B.DLC)
			ptr--;
	}

	ESP32Can.CANWriteFrame(&frame);
    workOnCAN(&frame);
    //printCANframe(&frame);  // debug

rx_out_ack:
	replybuf[0] = '\r';
	tmp = 1;
	goto rx_out;
rx_out_nack:
	replybuf[0] = '\a';
	tmp = 1;
rx_out:
#if 0
	ret = write(pty, replybuf, tmp);
#else
    RemoteClient.write(replybuf, tmp);
#endif
	if (ret < 0) {
		perror("write pty replybuf");
		return 1;
	}

	/* check if there is another command in this buffer */
	if (nbytes > ptr + 1) {
		for (tmp = 0, ptr++; ptr + tmp < nbytes; tmp++)
			buf[tmp] = buf[ptr + tmp];
		nbytes = tmp;
		goto rx_restart;
	}

	return 0;
}
