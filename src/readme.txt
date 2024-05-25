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
