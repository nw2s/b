/*

	nw2s::b - A microcontroller-based modular synth control framework
	Copyright (C) 2013 Scott Wilson (thomas.scott.wilson@gmail.com)

	This program is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program.  If not, see <http://www.gnu.org/licenses/>.

*/

#include <Key.h>
#include <EventManager.h>
#include <Trigger.h>
#include <Clock.h>
#include <Slew.h>
#include <Sequence.h>
#include <IO.h>
#include <SPI.h>
#include <SD.h>
#include <Wire.h>

using namespace nw2s;

PinAnalogOut outpins[16] = {
	
	DUE_SPI_4822_00,
	DUE_SPI_4822_01,
	DUE_SPI_4822_02,
	DUE_SPI_4822_03,
	DUE_SPI_4822_04,
	DUE_SPI_4822_05,
	DUE_SPI_4822_06,
	DUE_SPI_4822_07,
	DUE_SPI_4822_08,
	DUE_SPI_4822_09,
	DUE_SPI_4822_10,
	DUE_SPI_4822_11,
	DUE_SPI_4822_12,
	DUE_SPI_4822_13,
	DUE_SPI_4822_14,
	DUE_SPI_4822_15

};

int values[11] = {
	
	0,
	500,
	1000,
	1500,
	2000,
	2500,
	3000,
	3500,
	4000,
	4500,
	5000
};

AnalogOut* outputs[16];
int counter = 0;

void setup() 
{
	Serial.begin(19200);
	Serial.println("Starting...");

	EventManager::initialize();

	for (int i = 0; i < 16; i++)
	{
		outputs[i] = AnalogOut::create(outpins[i]);
		
		outputs[i]->outputCV(0000);
	}
}

void loop() 
{	
	if (millis() % 10000 == 0)
	{
		for (int i = 0; i < 16; i++)
		{
			outputs[i]->outputCV(values[counter]);
		}
		
		counter = (counter + 1) % 11;
	}

	EventManager::loop();
}
