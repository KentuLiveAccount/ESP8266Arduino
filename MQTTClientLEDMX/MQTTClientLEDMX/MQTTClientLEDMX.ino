#include <WiFiManager.h>
#include <PubSubClient.h>
#include <Wire.h>

#include <WEMOS_Matrix_LED.h>

const char *mqtt_server = "192.168.1.38"; 
const char *device_id = "esp8266MX";

WiFiClient espClient;
PubSubClient client(espClient);


//const byte D7 = 13;
//const byte D5 = 14;

MLED mled(0, D7, D5); //set intensity=0


static const uint8_t smileData[] =
  { B00111100,         
    B01111110,
    B11011011,
    B11111111,
    B11111111,
    B10111101,
    B01000010,
    B00111100 };

static const uint8_t sunData[] = {
    B00100100,
    B10111101,
    B01111110,         
    B01111110,
    B01111110,
    B01111110,
    B10111101,
    B00100100
};

static const uint8_t rainData[] =
  { B00001000,
    B00111110,         
    B01111111,
    B01111111,
    B00001000,
    B00001000,
    B00101000,
    B00010000 };

static const uint8_t blackData[] =
  { B00000000,
    B00000000,         
    B00000000,         
    B00000000,         
    B00000000,         
    B00000000,         
    B00000000,         
    B00000000 };
    
    
void allDots()
{
  for(int y=0;y<8;y++)
  {
    for(int x=0;x<8;x++)
    {
        mled.dot(x,y); // draw dot
        mled.display();
        delay(20);
        mled.dot(x,y,0);//clear dot
        mled.display();
        delay(20);        
    }  
  }
}

void clearBitmap()
{
  for (size_t i = 0; i < 8; i++)
  {
    mled.disBuffer[i] = 0;
  }
  mled.display();
}

void setBitmap(const uint8_t data[])
{
  for (size_t i = 0; i < 8; i++)
  {
    mled.disBuffer[i] = data[i];
  }
  mled.display();
}

void showBitmap(int intensity, const uint8_t data[])
{
  if (intensity == 0)
  {
    setBitmap(blackData);
    mled.display();
    return;
  }
  
  mled.intensity = intensity - 1;
  setBitmap(data);
  mled.display();
}

char message_buff[100];

void callback(char *led_control, byte *payload, unsigned int length)
{
  Serial.print("Message arrived [");
  Serial.print(led_control);
  Serial.println("] ");
  Serial.print("Length :");
  Serial.println(length);
  int i;
  for (i = 0; i < length; i++)
  {
    message_buff[i] = payload[i];
  }
  message_buff[i] = '\0';

  String msgString = String(message_buff);
  Serial.println(msgString);
  if (strcmp(led_control, "esp8266/led_control") == 0)
  { 
    if (msgString == "1")
    {
      setBitmap(smileData);
     Serial.println("Message == 1");
    }
    else if (msgString == "0")
    {
        setBitmap(rainData);
       Serial.println("Message == 0");
    }
    else
    {
       Serial.print("Unknown message :");
       Serial.println(payload[0]);
       Serial.println((byte)message_buff[0]);
       Serial.println(msgString);
       Serial.println(message_buff);
    }
  }
}

void reconnect()
{
  while (!client.connected())
  {
    Serial.print("Attempting MQTT connection...");
    if (client.connect(device_id, "cloud_username", "cloud_password"))
    { 
    Serial.println("connected");
    client.subscribe("esp8266/led_control"); // write your unique ID here
    }
    else
    {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

void setup()
{
  Serial.begin(115200);

  clearBitmap();

  allDots();

   WiFi.mode(WIFI_STA); // explicitly set mode, esp defaults to STA+AP

  //WiFiManager, Local intialization. Once its business is done, there is no need to keep it around
  WiFiManager wm;

  wm.setSTAStaticIPConfig(IPAddress(192,168,1,39)  /*ip*/, IPAddress(192,168,1,1 /*gw*/), IPAddress(255,255,255,0 /*sn mask*/)); // optional DNS 4th argument

  // reset settings - wipe stored credentials for testing
  // these are stored by the esp library
  //wm.resetSettings();

  bool res;
  res = wm.autoConnect("AutoConnectAP","password"); // password protected ap

  if(!res) {
      Serial.println("Failed to connect");
      // ESP.restart();
  } 
  else {
      //if you get here you have connected to the WiFi    
      Serial.println("connected...yeey :)");
  }

  Serial.println("");

  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(wm.getWiFiSSID(false /*persistent*/));
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  client.setServer(mqtt_server, 1883); // change port number as mentioned in your cloudmqtt console
  client.setCallback(callback);
}

void loop()
{
  if (!client.connected())
  {
    reconnect();
  }
  client.loop();
}
