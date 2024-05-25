ArctosCtrl, a CAN Wifi interface 

working
-connnect to a Wifi automatically, configure everything over Wifi access point
-setting a PWM for driving a servo
-receive CAN msgs, (id=7 data[0] sets PWM)
-showing a webpage where the PWM could be changed
-show the actual state on the display
not competely tested
-serving a port (2323) where CAN msgs could be read and send 

ToDo
-configure a PC so that the slcan msgs are working properly
-what port to choose? 2323 is a first guess

How to connect to the CAN?
telnet 192.168.178.175 2323   (where 192.168.178.175 should be the ip/name of your device)
(to finish telnet at OSX ctrl-ü + q)
when using Windows use putty

other things...
-Library für OLED Display nutzen zum zeichnen der einzelnen Seiten, Schriftgrößen, etc.
  Library "Heltec_ESP32-master" [#include "oled/OLEDDisplayUi.h"]


--- Vorgaben für config.txt
msgId		Hex wird nicht in json unerstützt, also per String

Beispiel [https://forum.arduino.cc/t/string-to-hex-converter/120931/5]

startbit	int
bitlen		int, darf auch 0 sein wenn es keine Variable gibt
dataformat	int	[int, uint8_t, uint16_t, uint32_t, float, double]

offset	double
factor	double

char[]	VarName
char[]	Einheit

bildschirmzeile int 0-basiert
bildschirmNr int 0-basiert
---
