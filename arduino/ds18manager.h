/*
 * DS18Sensor.h
 *
 *  Created on: 17.2.2018
 *      Author: Mafioso
 */

#ifndef DS18MANAGER_H_
#define DS18MANAGER_H_

#include <OneWire.h>

class DS18Manager
{
public:
	DS18Manager(uint8_t pin);

	void Init();
	void StartConversion();
	bool ConversionReady();
	float ReadTemperature(uint8_t device);

	uint8_t NumDevices();

	static const int maxDevices = 8;
private:
	struct Device {
		uint8_t address[8];
	};

	float CalculateTemperature(const uint8_t* data, uint8_t numDevice);

	OneWire oneWire;
	Device devices[maxDevices];
	uint8_t numDevices;
};

#endif /* DS18MANAGER_H_ */
