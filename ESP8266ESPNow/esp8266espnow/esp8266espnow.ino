#include <ESP8266WiFi.h>
#include <espnow.h>
#include "wifisetting.h"

// documentation: https://www.espressif.com/sites/default/files/documentation/2c-esp8266_non_os_sdk_api_reference_en.pdf

// Structure example to send data
// Must match the receiver structure
typedef struct struct_message {
  char a[32];
  int b;
  float c;
  String d;
  bool e;
} struct_message;

// Create a struct_message called myData
struct_message myData;

unsigned long lastTime = 0;  
unsigned long timerDelay = 2000;  // send readings timer
bool b1 = false;

bool IsB1(const byte *macAddr)
{
  for (byte i = 0; i < sizeof(macAddr1); i++)
  {
    if (macAddr[i] != macAddr1[i])
      return false;
  }

  return true;
}

// Callback when data is sent
void OnDataSent(uint8_t *mac_addr, uint8_t sendStatus) {
  Serial.print("Last Packet Send Status: ");
  if (sendStatus == 0){
    Serial.println("Delivery success");
  }
  else{
    Serial.println("Delivery fail");
  }
}

// Callback function that will be executed when data is received
void OnDataRecv(uint8_t * mac, uint8_t *incomingData, uint8_t len) {
  memcpy(&myData, incomingData, sizeof(myData));
  Serial.print("Bytes received: ");
  Serial.println(len);
  Serial.print("Char: ");
  Serial.println(myData.a);
  Serial.print("Int: ");
  Serial.println(myData.b);
  Serial.print("Float: ");
  Serial.println(myData.c);
  Serial.print("String: ");
  Serial.println(myData.d);
  Serial.print("Bool: ");
  Serial.println(myData.e);
  Serial.println();
}

void setup() {
  Serial.begin(115200);

  // Set device as a Wi-Fi Station
  WiFi.mode(WIFI_STA);

  byte mac[8];
  b1 = IsB1(WiFi.macAddress(mac));

  if (b1)
    Serial.print("\nB1 board - master\n");
  else
    Serial.print("\nB2 board - slave\n");


  Serial.print("\nInitializing ESPNow -");

  if (esp_now_init() != 0)
  {
    Serial.println("failed!");
    return;     
  }
  else
  {    
    Serial.println("success!");     
  }

  // Once ESPNow is successfully Init, we will register for Send CB to
  // get the status of Trasnmitted packet
  if (b1)
  {
    esp_now_set_self_role(ESP_NOW_ROLE_CONTROLLER);
    esp_now_register_send_cb(OnDataSent);
    esp_now_add_peer(macAddr2, ESP_NOW_ROLE_SLAVE, 1, NULL, 0);
  }
  else
  {
    esp_now_set_self_role(ESP_NOW_ROLE_SLAVE);
    esp_now_register_recv_cb(OnDataRecv);
  }

  // Register peer
}



void loop() {
  if (b1 && ((millis() - lastTime) > timerDelay)) {
    // Set values to send
    strcpy(myData.a, "THIS IS A CHAR");
    myData.b = random(1,20);
    myData.c = 1.2;
    myData.d = "Hello";
    myData.e = false;

    // Send message via ESP-NOW
    Serial.println("Sending Data");     
    esp_now_send(macAddr2, (uint8_t *) &myData, sizeof(myData));

    lastTime = millis();
  }
}
