/*
 *
 */

#include <ESP8266WiFiMulti.h>
#include <Servo.h>
#include <WebSocketsServer.h>

#include "ds18manager.h"

static ESP8266WiFiMulti WiFiMulti;
static WebSocketsServer webSocket = WebSocketsServer(81);
static Servo servo;
static DS18Manager dsSensors(D4);

static int f_servoPosition = 50;
static uint8_t f_clients[10];


static void ControlServo(const char* positionStr)
{
	int position = atoi(positionStr);
	if (position >= 0 && position <= 100) {
		int servoValue = position * 3.6f - 180;
		//Serial.print("Servo:");
		//Serial.println(servoValue);
		servo.write(servoValue);
		f_servoPosition = position;
	}
}

static void AddClient(uint8_t num)
{
	for (uint8_t i = 0; i < sizeof(f_clients); ++i) {
		if (f_clients[i] == 0xFF) {
			f_clients[i] = num;
			break;
		}
	}
}

static void RemoveClient(int8_t num)
{
	for (uint8_t i = 0; i < sizeof(f_clients); ++i) {
		if (f_clients[i] == num) {
			f_clients[i] = 0xFF;
			break;
		}
	}
}

static void NotifyClients(const char* message)
{
	for (uint8_t i = 0; i < sizeof(f_clients); ++i) {
		if (f_clients[i] != 0xFF) {
		    //Serial.print(i);
            //Serial.print(f_clients[i]);
            Serial.println(message);
			webSocket.sendTXT(f_clients[i], message);
		}
	}
}

static void SendServoPosition(void)
{
    char buffer[32];
    sprintf(buffer, "servo:%i", f_servoPosition);
    NotifyClients(buffer);
}

static void HandleWsEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length)
{
	(void)num;
	(void)length;

	switch (type) {
	case WStype_ERROR:
		Serial.println("WS: ERROR"); break;
	case WStype_DISCONNECTED:
	    RemoveClient(num);
	    Serial.print(num);
	    Serial.println("WS: DISCONNECTED"); break;
	case WStype_CONNECTED:
		AddClient(num);
		SendServoPosition();
		Serial.print("WS: CONNECTED ");
		Serial.println(num);
		break;
	case WStype_TEXT:
		//Serial.print("WS: TEXT ");
		//Serial.println((char*)payload);
		ControlServo((char*)payload);
		SendServoPosition();
		break;
	case WStype_BIN:
		Serial.println("WS: BIN"); break;
	case WStype_FRAGMENT_TEXT_START:
		Serial.println("WS: FRAGMENT_TEXT_START"); break;
	case WStype_FRAGMENT_BIN_START:
		Serial.println("WS: RAGMENT_BIN_START"); break;
	case WStype_FRAGMENT:
		Serial.println("WS: FRAGMENT"); break;
	case WStype_FRAGMENT_FIN:
		Serial.println("WS: FRAGMENT_FIN"); break;
	default:
		break;
	}
}

#if 0
static void InitSensors()
{
	oneWire.reset_search();
	uint8_t address[8];

	Serial.println("Starting to search DS oneWire...");
	while(oneWire.search(address)) {
		char addressStr[17];
		char* pBuf = addressStr;
		for (int i= 0; i < 8; ++i) {
			pBuf += sprintf(pBuf, "%02x", address[i]);
		}
		*pBuf = 0;
		Serial.println(addressStr);
	}
	Serial.println("Finished!");
}
#endif

void setup()
{
	Serial.begin(115200);

	WiFiMulti.addAP("bitwisewlan", "bitsalasana");
	WiFiMulti.addAP("Notwjork", "kukkaloora");

	servo.attach(D0);

	while (WiFiMulti.run() != WL_CONNECTED) {
		delay(100);
		Serial.print(".");
	}

	Serial.println("");
	Serial.print("WiFi connected to ");
	Serial.println(WiFi.BSSIDstr());
	Serial.println(WiFi.localIP());

	webSocket.begin();
	webSocket.onEvent(HandleWsEvent);

	memset(f_clients, 0xFF, sizeof(f_clients));
}

void loop()
{
    webSocket.loop();

	static uint32_t prevMillis = 0;
	uint32_t milliS = millis();

	if (milliS - prevMillis > 10000) {
	    prevMillis = milliS;
		dsSensors.StartConversion();
		while (!dsSensors.ConversionReady());

		for (uint8_t i = 0; i < dsSensors.NumDevices(); ++i) {
		    char message[32];
            sprintf(message, "temp%i:%.2f", i, dsSensors.ReadTemperature(i));
			Serial.println(message);
	        NotifyClients(message);
		}
	}
}

