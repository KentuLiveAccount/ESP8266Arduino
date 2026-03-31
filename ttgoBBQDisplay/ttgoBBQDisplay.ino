#include <ArduinoJson.h>
#include <HTTPClient.h>
#include <WiFi.h>
#include <SPI.h>
#include <Wire.h>
#include <WiFiClient.h>
#include "wifisetting.h"
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define OLED_RST 16
Adafruit_SSD1306 display(128, 64, &Wire, OLED_RST);

#define DEBUG(X) X

const char* turn_on = "http://192.168.1.4/cur";

const char *c_szDisconnected = "WiFi Disconnected";
const char *szMessage = c_szDisconnected;
String strExtra;

void WiFiEvent(WiFiEvent_t event) {


  switch (event) {
    case ARDUINO_EVENT_ETH_LOST_IP: szMessage = "ARDUINO_EVENT_ETH_LOST_IP";break;
    case ARDUINO_EVENT_WIFI_OFF: szMessage = "ARDUINO_EVENT_WIFI_OFF";break;
    case ARDUINO_EVENT_WIFI_FTM_REPORT: szMessage = "ARDUINO_EVENT_WIFI_FTM_REPORT";break;
    case ARDUINO_EVENT_WPS_ER_PBC_OVERLAP: szMessage = "ARDUINO_EVENT_WPS_ER_PBC_OVERLAP";break;
    case ARDUINO_EVENT_SC_SCAN_DONE: szMessage = "ARDUINO_EVENT_SC_SCAN_DONE";break;
    case ARDUINO_EVENT_SC_FOUND_CHANNEL: szMessage = "ARDUINO_EVENT_SC_FOUND_CHANNEL";break;
    case ARDUINO_EVENT_SC_GOT_SSID_PSWD: szMessage = "ARDUINO_EVENT_SC_GOT_SSID_PSWD";break;
    case ARDUINO_EVENT_SC_SEND_ACK_DONE: szMessage = "ARDUINO_EVENT_SC_SEND_ACK_DONE";break;
    case ARDUINO_EVENT_PROV_INIT: szMessage = "ARDUINO_EVENT_PROV_INIT";break;
    case ARDUINO_EVENT_PROV_DEINIT: szMessage = "ARDUINO_EVENT_PROV_DEINIT";break;
    case ARDUINO_EVENT_PROV_START: szMessage = "ARDUINO_EVENT_PROV_START";break;
    case ARDUINO_EVENT_PROV_END: szMessage = "ARDUINO_EVENT_PROV_END";break;
    case ARDUINO_EVENT_PROV_CRED_RECV: szMessage = "ARDUINO_EVENT_PROV_CRED_RECV";break;
    case ARDUINO_EVENT_PROV_CRED_FAIL: szMessage = "ARDUINO_EVENT_PROV_CRED_FAIL";break;
    case ARDUINO_EVENT_PROV_CRED_SUCCESS: szMessage = "ARDUINO_EVENT_PROV_CRED_SUCCESS";break;
    case ARDUINO_EVENT_PPP_START: szMessage = "ARDUINO_EVENT_PPP_START";break;
    case ARDUINO_EVENT_PPP_STOP: szMessage = "ARDUINO_EVENT_PPP_STOP";break;
    case ARDUINO_EVENT_PPP_CONNECTED: szMessage = "ARDUINO_EVENT_PPP_CONNECTED";break;
    case ARDUINO_EVENT_PPP_DISCONNECTED: szMessage = "ARDUINO_EVENT_PPP_DISCONNECTED";break;
    case ARDUINO_EVENT_PPP_GOT_IP: szMessage = "ARDUINO_EVENT_PPP_GOT_IP";break;
    case ARDUINO_EVENT_PPP_LOST_IP: szMessage = "ARDUINO_EVENT_PPP_LOST_IP";break;
    case ARDUINO_EVENT_PPP_GOT_IP6: szMessage = "ARDUINO_EVENT_PPP_GOT_IP6";break;
    case ARDUINO_EVENT_WIFI_READY:               szMessage = "WiFi interface ready"; break;
    case ARDUINO_EVENT_WIFI_SCAN_DONE:           szMessage = "Completed scan for AP"; break;
    case ARDUINO_EVENT_WIFI_STA_START:           szMessage = "WiFi client started"; break;
    case ARDUINO_EVENT_WIFI_STA_STOP:            szMessage = "WiFi clients stopped"; break;
    case ARDUINO_EVENT_WIFI_STA_CONNECTED:       szMessage = "Connected to AP"; break;
    case ARDUINO_EVENT_WIFI_STA_DISCONNECTED:    szMessage = "Disconnected from WiFi AP"; 
      break;
    case ARDUINO_EVENT_WIFI_STA_AUTHMODE_CHANGE: szMessage = "Auth mode of AP changed"; break;
    case ARDUINO_EVENT_WIFI_STA_GOT_IP:          szMessage = "Obtained IP address"; break;
    case ARDUINO_EVENT_WIFI_STA_LOST_IP:         szMessage = "Lost IP address and IP address is reset to 0"; break;
    case ARDUINO_EVENT_WPS_ER_SUCCESS:           szMessage = "WiFi Protected Setup (WPS): succeeded in enrollee mode"; break;
    case ARDUINO_EVENT_WPS_ER_FAILED:            szMessage = "WiFi Protected Setup (WPS): failed in enrollee mode"; break;
    case ARDUINO_EVENT_WPS_ER_TIMEOUT:           szMessage = "WiFi Protected Setup (WPS): timeout in enrollee mode"; break;
    case ARDUINO_EVENT_WPS_ER_PIN:               szMessage = "WiFi Protected Setup (WPS): pin code in enrollee mode"; break;
    case ARDUINO_EVENT_WIFI_AP_START:            szMessage = "WiFi access point started"; break;
    case ARDUINO_EVENT_WIFI_AP_STOP:             szMessage = "WiFi access point stopped"; break;
    case ARDUINO_EVENT_WIFI_AP_STACONNECTED:     szMessage = "Client connected"; break;
    case ARDUINO_EVENT_WIFI_AP_STADISCONNECTED:  szMessage = "Client disconnected"; break;
    case ARDUINO_EVENT_WIFI_AP_STAIPASSIGNED:    szMessage = "Assigned IP address to client"; break;
    case ARDUINO_EVENT_WIFI_AP_PROBEREQRECVED:   szMessage = "Received probe request"; break;
    case ARDUINO_EVENT_WIFI_AP_GOT_IP6:          szMessage = "AP IPv6 is preferred"; break;
    case ARDUINO_EVENT_WIFI_STA_GOT_IP6:         szMessage = "STA IPv6 is preferred"; break;
    case ARDUINO_EVENT_ETH_GOT_IP6:              szMessage = "Ethernet IPv6 is preferred"; break;
    case ARDUINO_EVENT_ETH_START:                szMessage = "Ethernet started"; break;
    case ARDUINO_EVENT_ETH_STOP:                 szMessage = "Ethernet stopped"; break;
    case ARDUINO_EVENT_ETH_CONNECTED:            szMessage = "Ethernet connected"; break;
    case ARDUINO_EVENT_ETH_DISCONNECTED:         szMessage = "Ethernet disconnected"; break;
    case ARDUINO_EVENT_ETH_GOT_IP:               szMessage = "Obtained IP address"; break;
    default:                                    break;
  }

  DEBUG(Serial.printf("[WiFi-event] event: %d, %s\n", event, szMessage));

}

void setup() {
  DEBUG(Serial.begin(115200));

  // put your setup code here, to run once:
  //WiFi.disconnect(true);

  WiFi.onEvent(WiFiEvent);

  Serial.printf("Connecting to: %s\n", WIFINAME);

  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  WiFi.begin(WIFINAME, WIFIPW);

  while (WiFi.status() != WL_CONNECTED) {
    delay(2000);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  pinMode(16, OUTPUT);
  
  digitalWrite(16, LOW);
  pinMode(2, OUTPUT);
  digitalWrite(2,LOW);
  delay(20);
  digitalWrite(OLED_RST, HIGH);
  Wire.begin(4, 15);
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3c, false, false)) { // Address 0x3C for 128x32
    DEBUG(Serial.println(F("SSD1306 allocation failed")));
    for (;;); // Don't proceed, loop forever
  }
  display.clearDisplay();
  display.setTextColor(WHITE);
  display.setTextSize(3);
  display.setCursor(0, 0);
  display.print("BBQ Mon");
  display.display();
  delay(1000);
}

void loop() {
  // put your main code here, to run repeatedly:

  //getstrin is the url that it gets
  if (WiFi.status() == WL_CONNECTED) {
    WiFiClient client;
    HTTPClient http;
    http.begin(client, turn_on);

    // Send HTTP GET request
    int httpResponseCode = http.GET();

    if (httpResponseCode > 0) {
      DEBUG(Serial.print("HTTP Response code: "));
      DEBUG(Serial.println(httpResponseCode));
      String payload = http.getString();
      StaticJsonDocument<192> doc;

      DeserializationError error = deserializeJson(doc, payload);

      if (error) {
        DEBUG(Serial.print("deserializeJson() failed: "));
        DEBUG(Serial.println(error.c_str()));

        display.clearDisplay();
        display.setCursor(0,0);
        display.setTextSize(2);
        display.print("JSON Err: ");
        display.print(error.c_str());
        display.display();

        return;
      }

      JsonObject message_0 = doc["message"][0];
      int message_0_currenttemp = message_0["currenttemp"]; // 222
      int message_0_internatemp = message_0["internatemp"]; // 221
      int message_0_targettemp = message_0["targettemp"]; // 100
      int message_0_servoangle = message_0["servoangle"]; // 100
      display.clearDisplay();
      display.setCursor(0,0);
      display.setTextSize(2); 
      display.print("Atm: ");
      display.print(message_0_currenttemp);
      display.setCursor(0,17);
      display.print("Int: ");
      display.print(message_0_internatemp);
      display.setCursor(0,34);
      display.print("Tgt: ");
      display.print(message_0_targettemp);
      display.setCursor(0,51);
      display.print("Srv: ");
      display.print(message_0_servoangle);
      display.display();
      
      DEBUG(Serial.println(payload));
    }
    else
    {
      display.clearDisplay();
      display.setCursor(0,0);
      display.setTextSize(2);
      display.print("HttpGet Err: ");
      display.print(String(httpResponseCode));
      display.display();

      DEBUG(Serial.print("Error code: "));
      DEBUG(Serial.println(httpResponseCode));
    }
    // Free resources
    http.end();
    delay(10000);
  }
  else 
  {
      display.clearDisplay();
      display.setCursor(0,0);
      display.setTextSize(2);
      display.print(szMessage);
      display.display();
      DEBUG(Serial.println("WiFi Disconnected"));
      delay(2000);
  }


}
