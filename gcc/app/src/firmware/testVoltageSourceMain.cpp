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

#include <b.h>
#include <EventManager.h>
#include <Clock.h>

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


AnalogOut* outputs[16];

void setup() 
{
	Serial.begin(19200);
	Serial.println("Starting...");

	/* Set up a clock to have something to watch :P */
	EventManager::initialize();
	//Clock* democlock = VariableClock::create(20, 200, DUE_IN_A01, 16);
	Clock* democlock = RandomTempoClock::create(118, 122, 16);
	EventManager::registerDevice(democlock);

	/* Iterate each of the outputs and set to 0mV */
	for (int i = 0; i < 16; i++)
	{
		outputs[i] = AnalogOut::create(outpins[i]);
		
		outputs[i]->outputCV(0000);
	}
}

void loop() 
{
	long t = millis();
	
	if (t % 100 == 0)
	{
		/* Read two analog inputs. We'll use these for coarse and fine */
		int cv1 = ::analogRead(DUE_IN_A00);
		delay(5);
		int cv2 = ::analogRead(DUE_IN_A01);
	
		int cv0 = ((cv1 << 1) & 0xFC0) | (cv2 >> 4);
	
		// for (int i = 0; i < 16; i++)
		// {
		// 	outputs[i]->outputRaw(cv0);
		// }
		
		// 95 = 10
		// 495 = 8
		// 895 = 6
		// 1295 = 4
		// 1695 = 2
		// 2095 = 0
		// 2495 = -2
		// 2895 = -4
		// 3295 = -6
		// 3695 = -8
		// 4095 = -10

		cv0 += 40;

		if (cv0 < 95) cv0 = 95;
		if (cv0 > 4095) cv0 = 4095;
		
		float volts = 10.0f - (((cv0 - 95) / 4000.0f) * 20.0f);
		
		int millivolts = (int)(volts * 1000);
		
		Serial.println(String(cv0) + " " + String(cv1) + " " + String(cv2) + " " + String(millivolts));
		// Serial.println(String(cv0) + " " + String(cv1) + " " + String(cv2));
	}
	
	EventManager::loop();
}
