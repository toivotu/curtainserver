/*
 *  This sketch demonstrates how to set up a simple HTTP-like server.
 *  The server will set a GPIO pin depending on the request
 *    http://server_ip/gpio/0 will set the GPIO2 low,
 *    http://server_ip/gpio/1 will set the GPIO2 high
 *  server_ip is the IP address of the ESP8266 module, will be
 *  printed to Serial when the module is connected.
 */

#include <ESP8266WiFi.h>
#include <MQTTClient.h>
#include <Servo.h>
#include "arduinomqtt.h"

Servo servo;

const char* ssid = "Notwjork";
const char* password = "kukkaloora";

// Create an instance of the server
// specify the port to listen on as an argument
//WiFiServer server(80);

void setup() {
  Serial.begin(115200);
  delay(10);

  // prepare GPIO2
  pinMode(2, OUTPUT);
  digitalWrite(2, 0);

  servo.attach(10);

  // Connect to WiFi network
  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");

  // Print the IP address
  Serial.println(WiFi.localIP());

  Serial.println("Opening mqtt");
  Serial.println(arduinomqtt_init("192.168.0.1", 1883));

  arduinomqtt_subscribe("bitwise/409/right_curtain/position");
  //Serial.println("Subscribe hello/emqtt");
  //Serial.println(arduinomqtt_subscribe("hello/emqtt"));
}


void loop()
{
	char topic[64];
	char message[64];
	if (arduinomqtt_readsubscribtion(topic, message) > 0) {
		Serial.println(topic);
		Serial.println(message);

		int value = atoi(message);

		if (value >= -180 && value <= 180) {
			servo.write(value);
		}
	}
}

