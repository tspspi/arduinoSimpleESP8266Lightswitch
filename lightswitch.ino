/*
    Simple alternative ESP8266 or ESP8265 firmware for Sonoff Basic
    switches.
    
    Note that this firmware does:
    * Support OTA Updates using an firmware included password
    * Includes wireless network information in the firmware image
    * Does not perform any authentication on the webpages

    This means that anybody who can extract firmware from one of the
    deployed devices has access to wireless or OTA passwords. This also
    means that changing these passwords requires a change to the firmware
    and it means that 802.1x EAP-TLS is not supported by this sketch.

	Of course it also means that anybody having access to the network
	can toggle the switch (and cause potential damage)

    This is one of the most basic possible siwtches. For production
    environments one should consider using authentication on web interfaces,
    move wireless configuration and flash passwords to SPIFFS - and
    possibly add MQTT support (and drop HTTP support entirely)
*/

 #include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include <ESP8266WebServer.h>

const char* ssid = "{FILLIN_WIFI_SSID}";
const char* password = "{FILLIN_WIFI_PASSWORD}";

#define LEDPORT 13
#define RELAISPORT 12
#define SWITCHPORT 0

ESP8266WebServer server(80);

static const char* htmlPart1 = "<!DOCTYPE HTML><html><head><title>Simpler Lichtschalter</title><style type=\"text/css\">h1{display:block;color:white;background-color:darkgreen;font-variant:small-caps;padding:0.5ex 1ex 0.5ex 1ex;margin-bottom:0;}div#m{margin-top:0;border:1px solid grey;padding:0.5ex 1ex 0.5ex 1ex;}</style></head><body><h1>Simpler Lichtschalter</h1><div id=\"m\"><p>Status: <strong>";
static const char* htmlPart2 = "</strong> (<a href=\"/on\">Ein</a>, <a href=\"/off\">Aus</a>)</p></div></body></html>";

static const char* htmlOn = "Ein";
static const char* htmlOff = "Aus";

static boolean switchedOn = false;

static void sendStatusPage(int statusCode) {
  String resp;

  resp += htmlPart1;
  resp += switchedOn ? htmlOn : htmlOff;
  resp += htmlPart2;

  /* We set auto-refresh every 15 seconds */
  server.sendHeader("Refresh", "15; url=/");
  /*
    And disable client- and proxy side caching
    as well as reverse proxy caching
  */
  server.sendHeader("Cache-control", "no-cache, no-store, must-revalidate");
  server.sendHeader("Pragma", "no-cache");
  server.send(statusCode, "text/html", resp);
}

static void switchOn() {
  digitalWrite(RELAISPORT, HIGH);
  switchedOn = true;
}

static void switchOff() {
  digitalWrite(RELAISPORT, LOW);
  switchedOn = false;
}

static void handleOn() { switchOn(); sendStatusPage(200); }
static void handleOff() { switchOff(); sendStatusPage(200); }
static void handleRoot() { sendStatusPage(200); }

static void handleNotFound() { server.send(404, "text/plain", "Unknown URI"); }

static boolean ledStatus = true;

void setup() {
  pinMode(RELAISPORT, OUTPUT);
  pinMode(LEDPORT, OUTPUT);

  digitalWrite(LEDPORT, LOW);
  digitalWrite(RELAISPORT, LOW);

  Serial.begin(115200);
  Serial.println("Booting");

  WiFi.persistent(false); /* Disable WiFi persistence to avoid flash writes */
  WiFi.mode(WIFI_OFF);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.waitForConnectResult() != WL_CONNECTED) {
    Serial.println("Connection Failed! Rebooting...");
    delay(5000);
    ESP.restart();
  }


  // Hostname defaults to esp8266-[ChipID]
  //ArduinoOTA.setHostname("lightswitch");

  ArduinoOTA.setPassword("{FILLIN_OTAPASSWORD}");

  ArduinoOTA.onStart([]() { Serial.println("Start updating"); });
  ArduinoOTA.onEnd([]() { Serial.println("\nEnd"); });

  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  });

  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) {
      Serial.println("Auth Failed");
    } else if (error == OTA_BEGIN_ERROR) {
      Serial.println("Begin Failed");
    } else if (error == OTA_CONNECT_ERROR) {
      Serial.println("Connect Failed");
    } else if (error == OTA_RECEIVE_ERROR) {
      Serial.println("Receive Failed");
    } else if (error == OTA_END_ERROR) {
      Serial.println("End Failed");
    }
  });

  ArduinoOTA.begin();

  server.on("/on", handleOn);
  server.on("/off", handleOff);
  server.on("/", handleRoot);
  server.onNotFound(handleNotFound);
  server.begin();

  Serial.println("HTTP Server started");

  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}

void loop() {
  ArduinoOTA.handle();
  server.handleClient();

  /*
     We show a different blinking pattern
     depending on on/off state.
  */
  unsigned long currentMillis = millis();

  if((currentMillis % 5000) > 4500) {
    if(ledStatus == switchedOn) {
      digitalWrite(LEDPORT, switchedOn ? HIGH : LOW);
      ledStatus = !switchedOn;
    }
  } else {
    if(ledStatus != switchedOn) {
      digitalWrite(LEDPORT, switchedOn ? LOW : HIGH);
      ledStatus = switchedOn;
    }
  }
}
