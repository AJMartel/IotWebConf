/**
 * IotWebConf09CustomConnection.ino -- IotWebConf is an ESP8266/ESP32
 *   non blocking WiFi/AP web configuration library for Arduino.
 *   https://github.com/prampec/IotWebConf 
 *
 * Copyright (C) 2018 Balazs Kelemen <prampec+arduino@gmail.com>
 *
 * This software may be modified and distributed under the terms
 * of the MIT license.  See the LICENSE file for details.
 */

/**
 * Example: Custom connection
 * Description:
 *   This example is for advanced users only!
 *   In this example custom connection handler methods are defined
 *   to override the default connecting behavior.
 *   Also, three custom parameters are introduced, that are used
 *   at the connection.
 *   (See previous examples for more details!)
 * 
 * Hardware setup for this example:
 *   - An LED is attached to LED_BUILTIN pin with setup On=LOW.
 *   - [Optional] A push button is attached to pin D2, the other leg of the
 *     button should be attached to GND.
 */
 
 /**
 * Important: This example requires ESPAsyncWebServer library, https://github.com/me-no-dev/ESPAsyncWebServer
 * Also for ESP8266 it requires ESPAsyncTCP, https://github.com/me-no-dev/ESPAsyncTCP
 * For ESP32 it requires AsyncTCP, https://github.com/me-no-dev/AsyncTCP
 * 
 * To enable the use of the AsyncWebServer #define IOTWEBCONF_CONFIG_USE_ASYNC must be
 * uncommented in IotWebConf.h, or otherwise defined.
 */

#include <IotWebConf.h>

// -- Initial name of the Thing. Used e.g. as SSID of the own Access Point.
const char thingName[] = "testThing";

// -- Initial password to connect to the Thing, when it creates an own Access Point.
const char wifiInitialApPassword[] = "smrtTHNG8266";

#define STRING_LEN 128
#define NUMBER_LEN 32

// -- Configuration specific key. The value should be modified if config structure was changed.
#define CONFIG_VERSION "dem9"

// -- When CONFIG_PIN is pulled to ground on startup, the Thing will use the initial
//      password to buld an AP. (E.g. in case of lost password)
#define CONFIG_PIN 2

// -- Status indicator pin.
//      First it will light up (kept LOW), on Wifi connection it will blink,
//      when connected to the Wifi it will turn off (kept HIGH).
#define STATUS_PIN LED_BUILTIN

// -- Callback method declarations.
void configSaved();
boolean formValidator(AsyncWebServerRequest *request);
boolean connectAp(const char* apName, const char* password);
void connectWifi(const char* ssid, const char* password);

DNSServer dnsServer;
AsyncWebServer server(80);

char ipAddressValue[STRING_LEN];
char gatewayValue[STRING_LEN];
char netmaskValue[STRING_LEN];

IotWebConf iotWebConf(thingName, &dnsServer, &server, wifiInitialApPassword, CONFIG_VERSION);
IotWebConfParameter ipAddressParam = IotWebConfParameter("IP address", "ipAddress", ipAddressValue, STRING_LEN, "text", NULL, "192.168.3.222");
IotWebConfParameter gatewayParam = IotWebConfParameter("Gateway", "gateway", gatewayValue, STRING_LEN, "text", NULL, "192.168.3.0");
IotWebConfParameter netmaskParam = IotWebConfParameter("Subnet mask", "netmask", netmaskValue, STRING_LEN, "text", NULL, "255.255.255.0");

IPAddress ipAddress;
IPAddress gateway;
IPAddress netmask;

void setup() 
{
  Serial.begin(115200);
  Serial.println();
  Serial.println("Starting up...");

  iotWebConf.setStatusPin(STATUS_PIN);
  iotWebConf.setConfigPin(CONFIG_PIN);
  iotWebConf.addParameter(&ipAddressParam);
  iotWebConf.addParameter(&gatewayParam);
  iotWebConf.addParameter(&netmaskParam);
  iotWebConf.setConfigSavedCallback(&configSaved);
  iotWebConf.setFormValidator(&formValidator);
  iotWebConf.setApConnectionHandler(&connectAp);
  iotWebConf.setWifiConnectionHandler(&connectWifi);

  // -- Initializing the configuration.
  boolean validConfig = iotWebConf.init();

  // -- Set up required URL handlers on the web server.
  server.on("/", handleRoot);
  server.on("/config", [](AsyncWebServerRequest *request){ iotWebConf.handleConfig(request); });
  server.onNotFound([](AsyncWebServerRequest *request){ iotWebConf.handleNotFound(request); });

  Serial.println("Ready.");
}

void loop() 
{
  // -- doLoop should be called as frequently as possible.
  iotWebConf.doLoop();
}

/**
 * Handle web requests to "/" path.
 */
void handleRoot(AsyncWebServerRequest *request)
{
  // -- Let IotWebConf test and handle captive portal requests.
  if (iotWebConf.handleCaptivePortal(request))
  {
    // -- Captive portal request were already served.
    return;
  }
  String s = "<!DOCTYPE html><html lang=\"en\"><head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1, user-scalable=no\"/>";
  s += "<title>IotWebConf 09 Custom Connection</title></head><body>Hello world!";
  s += "<ul>";
  s += "<li>IP address: ";
  s += ipAddressValue;
  s += "</ul>";
  s += "Go to <a href='config'>configure page</a> to change values.";
  s += "</body></html>\n";

  request->send(200, "text/html", s);
}

void configSaved()
{
  Serial.println("Configuration was updated.");
}

boolean formValidator(AsyncWebServerRequest *request)
{
  Serial.println("Validating form.");
  boolean valid = true;

  if (!ipAddress.fromString(request->arg(ipAddressParam.getId())))
  {
    ipAddressParam.errorMessage = "Please provide a valid IP address!";
    valid = false;
  }
  if (!netmask.fromString(request->arg(netmaskParam.getId())))
  {
    netmaskParam.errorMessage = "Please provide a valid netmask!";
    valid = false;
  }
  if (!gateway.fromString(request->arg(gatewayParam.getId())))
  {
    gatewayParam.errorMessage = "Please provide a valid gateway address!";
    valid = false;
  }

  return valid;
}

boolean connectAp(const char* apName, const char* password)
{
  // -- Custom AP settings
  return WiFi.softAP(apName, password, 4);
}
void connectWifi(const char* ssid, const char* password)
{
  ipAddress.fromString(String(ipAddressValue));
  netmask.fromString(String(netmaskValue));
  gateway.fromString(String(gatewayValue));

  if (!WiFi.config(ipAddress, gateway, netmask)) {
    Serial.println("STA Failed to configure");
  }
  Serial.print("ip: ");
  Serial.println(ipAddress);
  Serial.print("gw: ");
  Serial.println(gateway);
  Serial.print("net: ");
  Serial.println(netmask);
  WiFi.begin(ssid, password);
}
