#ifndef ARDUINOMQTT_H
#define ARDUINOMQTT_H

#include <Stream.h>

#if 0

extern "C" {

}
struct mqtt_broker_handle_t;

class WifiClient;
class elapsedMillis;

class Arduinomqtt {
public:
	typedef enum {
		STATUS_OK,
		NEW_MESSAGE,
		CONNECTION_ERROR,
		PACKET_ERROR
	} Status;

	Arduinomqtt(const char* address, int port);
	~Arduinomqtt();

	Status Subscribe(const char* topic);
	Status Update(char* topic, char* message);

private:
	Arduinomqtt();

	WiFiClient* m_client;
	mqtt_broker_handle_t* m_broker;

	byte b_packetBuffer[1024];

	elapsedMillis* elapsedTime;

};

#endif

extern int arduinomqtt_init(const char* address, int port);
extern int arduinomqtt_subscribe(const char*);
extern int arduinomqtt_readsubscribtion(char* topic, char* message);

#endif
