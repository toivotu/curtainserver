//#include <elapsedMillis.h>
#include <ESP8266Wifi.h>

#include "arduinomqtt.h"

extern "C" {
#include "libemqtt.h"
}

#define TIMEOUT 5000u

static WiFiClient f_client;
static mqtt_broker_handle_t f_broker;

static byte f_packetBuffer[1024];

//elapsedMillis elapsedTime;

static int Read(byte* buffer, uint16_t numBytes)
{
	int bytesRead = 0;
	uint32_t timeout = TIMEOUT;

	while (bytesRead < numBytes) {
		int data = f_client.read();

		if (data < 0) {
			if (--timeout == 0) {
				break;
			}
			delay(1);
		} else {
			buffer[bytesRead++] = data;
		}
	}

	return bytesRead;
}

static int ReadPacket(void)
{
	const int headerLength = 2;
	int packetLength = -1;

	memset(f_packetBuffer, 0, sizeof(f_packetBuffer));
  
	if (Read(f_packetBuffer, headerLength) == headerLength) {
  
		packetLength =
			mqtt_parse_rem_len(f_packetBuffer) +
			mqtt_num_rem_len_bytes(f_packetBuffer) +
			1;

		if (Read(&f_packetBuffer[headerLength], packetLength - headerLength) != packetLength - headerLength) {
			packetLength = -2;
		}
  }

  return packetLength;
}

static int send_packet(void* socket_info, const void* buf, unsigned int count)
{
  WiFiClient* client = (WiFiClient*)socket_info;
  const uint8_t* buffer = (const uint8_t*)buf;
  return client->write(buffer, count);
}

int arduinomqtt_init(const char* address, int port)
{
	int retval = 0;
	retval = f_client.connect(address, port);

	if (retval < 0) return retval;

	f_broker.socket_info = &f_client;
	f_broker.send = send_packet;

	mqtt_init(&f_broker, "ESP");

	retval = mqtt_connect(&f_broker);

	if (retval > 0) {
		retval = ReadPacket();
		if (retval > 0) {
			if (MQTTParseMessageType(f_packetBuffer) != MQTT_MSG_CONNACK) {
				retval = -2;
			} else if (f_packetBuffer[3] != 0x00) {
				retval = -3;
			}
		} else {
			retval = -4;
		}
	}

	return retval;
}

int arduinomqtt_subscribe(const char* topic)
{
	int retval = mqtt_subscribe(&f_broker, topic, 0);
	retval = ReadPacket();
	if (retval > 0) {
		if (MQTTParseMessageType(f_packetBuffer) != MQTT_MSG_SUBACK) {
			retval = -2;
		}
	} else {
		retval = -1;
	}

	return retval;
}

bool arduinomqtt_ping(void)
{
	bool success = false;
	mqtt_ping(&f_broker);

	if (ReadPacket() > 0) {
		success = MQTTParseMessageType(f_packetBuffer) == MQTT_MSG_PINGRESP;
	}

	return success;
}

int arduinomqtt_readsubscribtion(char* topic, char* message)
{
	int retval = -1;

	if (ReadPacket() > 0) {

		if(MQTTParseMessageType(f_packetBuffer) == MQTT_MSG_PUBLISH)
		{
			int len;
			len = mqtt_parse_pub_topic((uint8_t*)f_packetBuffer, (uint8_t*)topic);
			topic[len] = '\0';
			len = mqtt_parse_publish_msg((uint8_t*)f_packetBuffer, (uint8_t*)message);
			message[len] = '\0';
			retval = 1;
		}
	} /*else {
		if (elapsedTime > 30000) {
			retval = arduinomqtt_ping() ? 0 : 1;
			elapsedTime = 0;
		}
	}*/

	return retval;
}
#if 0
Arduinomqtt::Arduinomqtt(const char* address, int port)
{
	int retval = 0;

	m_client = new(WifiClient);
	//m_broker = new mqtt_broker_handle_t();
	retval = m_client->connect(address, port);

	//if (retval < 0) return retval;

	m_broker->socket_info = &f_client;
	m_broker->send = send_packet;

	mqtt_init(m_broker, "ESP");

	retval = mqtt_connect(m_broker);

	if (retval > 0) {
		retval = ReadPacket();
		if (retval > 0) {
			if (MQTTParseMessageType(f_packetBuffer) != MQTT_MSG_CONNACK) {
				retval = -2;
			} else if (f_packetBuffer[3] != 0x00) {
				retval = -3;
			}
		} else {
			retval = -1;
		}
	}

	return retval;
}

Arduinomqtt::~Arduinomqtt()
{
	Disconnect();
}

Arduinomqtt::Status Arduinomqtt::Subscribe(const char* topic)
{

}

Arduinomqtt::Status Arduinomqtt::Update(char* topic, char* message)
{

}
#endif
