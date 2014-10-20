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
#include <aJSON.h>
#include <Usb.h>

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
	
	-5000,
	-4000,
	-3000,
	-2000,
	-1000,
	0,
	1000,
	2000,
	3000,
	4000,
	5000
};

bool checkToggle = false;
long nextToggleTime = 0;

AnalogOut* outputs[16];
int counter = 0;

void setup() 
{
	Serial.begin(19200);
	Serial.println("Starting...");

	/* Always set up the gain mode first - use HIGH if you are biased for -10V - +10V */
	b::cvGainMode = CV_GAIN_LOW;

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
	
	if (t > nextToggleTime)
	{
	
		/* Only change from one to the next if the toggle is on */
		if (!checkToggle && digitalRead(DUE_IN_D0))
		{
			/* Stop the bounce */
			nextToggleTime = t + 100;
			checkToggle = true;

			for (int i = 0; i < 16; i++)
			{
				outputs[i]->outputCV(values[counter]);
			}
		
			Serial.println("output val: " + String(values[counter]));

			counter = (counter + 1) % 11;		
		}
	
		/* Reset the toggle trigger if it's off */
		if (checkToggle && !digitalRead(DUE_IN_D0))
		{
			/* Stop the bounce */
			nextToggleTime = t + 100;
			checkToggle = false;
		}
	}
	
	EventManager::loop();
}
