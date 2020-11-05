/*
  HelloServerBearSSL - Simple HTTPS server example
  This example demonstrates a basic ESP8266WebServerSecure HTTPS server
  that can serve "/" and "/inline" and generate detailed 404 (not found)
  HTTP respoinses.  Be sure to update the SSID and PASSWORD before running
  to allow connection to your WiFi network.
  Adapted by Earle F. Philhower, III, from the HelloServer.ino example.
  This example is released into the public domain.
*/
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServerSecure.h>


#ifndef STASSID
#define STASSID "----"
#define STAPSK  "----"
#endif

const char* ssid = STASSID;
const char* password = STAPSK;

BearSSL::ESP8266WebServerSecure server(443);

static const char serverCert[] PROGMEM = R"EOF(
-----BEGIN CERTIFICATE-----
MIIC4jCCAkugAwIBAgIUG1LUbeNmYtZ9gS9v43SELrNycjQwDQYJKoZIhvcNAQEL
BQAwczELMAkGA1UEBhMCVVMxCzAJBgNVBAgMAldBMREwDwYDVQQHDAhLaXJrbGFu
ZDEVMBMGA1UECgwMS2VudGFyb1VyYXRhMRUwEwYDVQQLDAxLZW50YXJvVXJhdGEx
FjAUBgNVBAMMDWVzcDgyNjYubG9jYWwwHhcNMjAxMTA0MTA1MTU0WhcNMzAxMTAy
MTA1MTU0WjBzMQswCQYDVQQGEwJVUzELMAkGA1UECAwCV0ExETAPBgNVBAcMCEtp
cmtsYW5kMRUwEwYDVQQKDAxLZW50YXJvVXJhdGExFTATBgNVBAsMDEtlbnRhcm9V
cmF0YTEWMBQGA1UEAwwNZXNwODI2Ni5sb2NhbDCBnzANBgkqhkiG9w0BAQEFAAOB
jQAwgYkCgYEAwovnVnqKSJddsxy41LGZxNefiGmaLTGIhIYAes4G/qzHgb0hKp8x
eqq8Gf2P7/75dJg7mYAvHdZEhbfHZFW4Sp+DFVicvOTNVy1DNudOOVRhJ5VpY1b0
bl7MOSC3tAJhMGlW/VzepIp+S19PlgKCY1YqbiVXQYIi4IUy5gb9Ws8CAwEAAaNz
MHEwHQYDVR0OBBYEFJy9klciJk0RKeApo6TFFT7R+L3WMB8GA1UdIwQYMBaAFJy9
klciJk0RKeApo6TFFT7R+L3WMA8GA1UdEwEB/wQFMAMBAf8wHgYDVR0RBBcwFYIN
ZXNwODI2Ni5sb2NhbIcEwKgBJzANBgkqhkiG9w0BAQsFAAOBgQCXNNVby5aOYSb/
rMCzHS4mnoeCoBmUdvL94rIT8fIxtVpugUSp25OYl2HFLr6MZBhxRegOi2EpFZ61
eN5kPwpPCim6xk6nyKgNK6c3Am+yx0fV18rUGASl53vJir4dVItJNO+Wf1KojuLo
70QkXB27tJ2H+mogKHKbVZTgH+1vig==
-----END CERTIFICATE-----
)EOF";

static const char serverKey[] PROGMEM =  R"EOF(
-----BEGIN PRIVATE KEY-----
MIICdwIBADANBgkqhkiG9w0BAQEFAASCAmEwggJdAgEAAoGBAMKL51Z6ikiXXbMc
uNSxmcTXn4hpmi0xiISGAHrOBv6sx4G9ISqfMXqqvBn9j+/++XSYO5mALx3WRIW3
x2RVuEqfgxVYnLzkzVctQzbnTjlUYSeVaWNW9G5ezDkgt7QCYTBpVv1c3qSKfktf
T5YCgmNWKm4lV0GCIuCFMuYG/VrPAgMBAAECgYArO7P5P9ojL6AfAa8BdUZavCAz
zGP6zxsreCv7HnXnerYLWuBX/HCedfq/O94U03DUPFBiWF4gH1Gy9ZhV78a4S1dO
eNW583Y1tQwet0SOSBOsGSgq2WqqlZTduxnXiXhlxIdEJ9z2OTmZ+Dm20EObKdT2
lysfGFCyjJxFxJgeoQJBAOmpwVw+LuazpBQ459d8B7uUMwcxhNyO068KFMJOfxLQ
HSmqo1l/YjxYc4lPITYYmSM67C0kZvENM9Xym1nWfhMCQQDVJOE8oSjvixbzmmEA
b5dlKJENQ65bpQYvzZ1fnR6o1ewjz+qoKPFW8I/gV8xw053yz3BRvnNKbSkcZL2w
WlfVAkBGLyQSNedWilzdah5RPkMAV8pf/cK/kPMKX4fOuU838mTgEA4Sos15/MNv
WeJK4maVC2zHPmjhPKr2N3HsMR4bAkEAxABYB5MDu9Qh1P2/+dCkXefCC3qTmb7V
Q5xG/afUi9m8fuoxlVWhKRuqktjqLU7MTn6ngOQrzOM5DN0u+j75yQJBAJ3e+E/a
h1cPtwkc2fO5iVBhs2DhN6UuqSkh9Mhlmwz+0HtMvWV4Y1xqROiRBpAvtEsAaa/x
QwJKaEtyN11w0+E=
-----END PRIVATE KEY-----
)EOF";


const int led = 2;

void handleRoot() {

  digitalWrite(led, HIGH);
  server.send(200, "text/plain", "Hello from esp8266 over HTTPS! ");
  delay(200);
  digitalWrite(led, LOW);
}

void handleJson() {

  digitalWrite(led, HIGH);
  server.send(200, "application/json;charset=utf-8", "{\"message\": \"hello from ESP8266!\"}");
  delay(200);
  digitalWrite(led, LOW);
}

void handleNotFound(){
  digitalWrite(led, HIGH);
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET)?"GET":"POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i=0; i<server.args(); i++){
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  server.send(404, "text/plain", message);
  digitalWrite(led, LOW);
}

void setup(void){
  pinMode(led, OUTPUT);
  digitalWrite(led, HIGH);
  Serial.begin(115200);
  WiFi.begin(ssid, password);
  Serial.println("");

  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  configTime(3 * 3600, 0, "pool.ntp.org", "time.nist.gov");

  time_t now;

  while (now < 1546300800)
  {
    now = time(nullptr);
    delay(500);
    Serial.print("*");
  }

  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  server.getServer().setRSACert(new BearSSL::X509List(serverCert), new BearSSL::PrivateKey(serverKey));

  server.enableCORS(true);

  server.on("/", handleRoot);

  server.on("/json", handleJson);

  server.onNotFound(handleNotFound);

  server.begin();
  Serial.println("HTTPS server started");

  digitalWrite(led, LOW);

}

void loop(void){
  server.handleClient();
}
