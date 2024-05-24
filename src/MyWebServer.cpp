#include <Arduino.h>

#include "MyWebServer.h"
#include "Arctos.h"
#include "MyCAN.h"
#include <ESPmDNS.h>

#include "AutoConnect.h"

WebServer   server;
AutoConnect portal(server);

void sendRedirect(String uri) {
  server.sendHeader("Location", uri, true);
  server.send(302, "text/plain", "");
  server.client().stop();
}

void handleChange() {
  Serial.println("IO = " + server.arg("v"));
  String arg = server.arg("v");
  if (arg == "low"){
    if(actCANdata.servo1>0){
      actCANdata.servo1--;
    }
  } else if (arg == "high"){
    if(actCANdata.servo1<255){
      actCANdata.servo1++;
    }
  } else if (arg == "open"){
    actCANdata.servo1 = 35;
  } else if (arg == "close"){
    actCANdata.servo1 = 4;
  }
  actCANdata.servo1Changed=true;
  outputChanges();
  sendRedirect("/");
}

//#define refresh_s 10              // the website

void rootPage() {
  String page = 
  "<html><head>"
#ifdef refresh_s
      "<meta http-equiv='refresh' content='" refresh_s "'/>"
#endif
  "<title>" + GetMyNodeName() + "</title>" // das wird im Browser als Tab Titel angezeigt
  "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">"
  "<style type=\"text/css\">"
    "body {"
    "-webkit-appearance:none;"
    "-moz-appearance:none;"
    "font-family:'Arial',sans-serif;"
    "text-align:center;"
    "}"
    ".menu > a:link {"
    "position: absolute;"
    "display: inline-block;"
    "right: 12px;"
    "padding: 0 6px;"
    "text-decoration: none;"
    "}"
    ".button {"
    "display:inline-block;"
    "border-radius:7px;"
    "background:#73ad21;"
    "margin:0 10px 0 10px;"
    "padding:10px 20px 10px 20px;"
    "text-decoration:none;"
    "color:#000000;"
    "}"
  "</style>"
"</head>"
"<body>"
  "<div class=\"menu\">" AUTOCONNECT_LINK(BAR_32) "</div>"
  "<h1>" + GetMyNodeName() +"</h1><br><br>"
  "PWM Gripper=" + String(actCANdata.servo1);
  page += String(F("<p>"
    "<a class='button' href='/io?v=open'>open</a>"
    "<a class='button' href='/io?v=low'>-</a>"
    "<a class='button' href='/io?v=high'>+</a>"
    "<a class='button' href='/io?v=close'>close</a>"
  "</p>"));
  page += String(F("</body></html>"));
  server.send(200, "text/html", page);
}

void setupAutoconnect(){
  AutoConnectConfig config;
  config.ota = AC_OTA_BUILTIN;
  portal.config(config);

  server.on("/", rootPage);
  server.on("/io", handleChange);
  if (portal.begin()) {
    Serial.println("WiFi connected: " + WiFi.localIP().toString());
  } else {
    Serial.println("Portal failed");
  }
  if (MDNS.begin(GetMyNodeName().c_str())){
    MDNS.addService("http", "tcp", 80);
    Serial.println(F("mDNS responder started"));
  } else {
    Serial.println(F("mDNS.begin() failed"));
  }
}
