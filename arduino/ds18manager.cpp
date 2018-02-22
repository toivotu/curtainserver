/*
 * temperature.cpp
 *
 *  Created on: 17.2.2018
 *      Author: Mafioso
 */

#include "ds18manager.h"

#include <cmath>
#include <inttypes.h>
#include <onewire.h>

namespace {

const uint8_t DEVICE_FAMILY_DS18B = 0x28;
const uint8_t DEVICE_FAMILY_DS18S = 0x10;

bool IsDS18S20(const uint8_t* address)
{
	return address[0] == DEVICE_FAMILY_DS18S;
}

bool IsDS18B20(const uint8_t* address)
{
	return address[0] == DEVICE_FAMILY_DS18B;
}

bool IsDS18Device(uint8_t* address)
{
	return IsDS18S20(address) || IsDS18B20(address);
}

uint8_t TemperatureLsb(const uint8_t* data)
{
	return data[0];
}

uint8_t TemperatureMsb(const uint8_t* data)
{
	return data[1];
}

uint8_t CountPerC(const uint8_t* data)
{
	return data[7];
}

uint8_t CountRemain(const uint8_t* data)
{
	return data[6];
}

}

DS18Manager::DS18Manager(uint8_t pin):
		oneWire(pin), numDevices(0)
{
	oneWire.reset_search();
	uint8_t address[8];

	while(oneWire.search(address) && numDevices < 8) {
		if (IsDS18Device(address)) {
			memcpy(&devices[numDevices++].address[0], address, sizeof(address));
		}
	}
}

void DS18Manager::Init()
{
	return;
	oneWire.reset_search();
	uint8_t address[8];

	while(oneWire.search(address)) {
		if (IsDS18Device(address)) {
			memcpy(&devices[numDevices++].address[0], address, sizeof(address));
		}
	}
}

uint8_t DS18Manager::NumDevices()
{
	return numDevices;
}

void DS18Manager::StartConversion()
{
	if (oneWire.reset()) {
		const uint8_t startConversion = 0x44;

		oneWire.skip();
		oneWire.write(startConversion);
	}
}

bool DS18Manager::ConversionReady()
{
	return oneWire.read_bit();
}

float DS18Manager::CalculateTemperature(const uint8_t* data, uint8_t numDevice)
{
	float temperature = (TemperatureLsb(data) + TemperatureMsb(data) * 256);

	if (IsDS18S20(devices[numDevice].address)) {
		temperature = floorf(temperature * 0.5) + (16 - CountRemain(data))/16.f - 0.25f;
	} else if (IsDS18B20(devices[numDevice].address)) {
		temperature = temperature * 0.0625f;
	}

	return temperature;
}

float DS18Manager::ReadTemperature(uint8_t device)
{
	if (oneWire.reset()) {
		const uint8_t readScratchpad = 0xBE;
		uint8_t buffer[8];

		oneWire.reset();
		oneWire.select(devices[device].address);
		oneWire.write(readScratchpad);
		oneWire.read_bytes(buffer, sizeof(buffer));

		return CalculateTemperature(buffer, device);
	} else {
		return 0;
	}
}
