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

int average[12][10];
PinAnalogIn inputs[12] = {
	
	DUE_IN_A00,
	DUE_IN_A01,
	DUE_IN_A02,
	DUE_IN_A03,
	DUE_IN_A04,
	DUE_IN_A05,
	DUE_IN_A06,
	DUE_IN_A07,
	DUE_IN_A08,
	DUE_IN_A09,
	DUE_IN_A10,
	DUE_IN_A11
	
};

int ptr = 0;

void setup() 
{
	Serial.begin(19200);
	Serial.println("Starting...");

	/* Setup analog inputs */
	analogReadResolution(12);

	for (int i = 0; i < 12; i++)
	{
		for (int j = 0; j < 10; j++)
		{
			average[i][j] = 0;
		}
	}	
}

void loop() 
{	
	if (millis() % 250 == 0)
	{
		int sum1 = 0;
		int sum2 = 0;
		
		for (int i = 0; i < 12; i++)
		{
			int sum = 0;
			int val = analogRead(inputs[i]);
			
			average[i][ptr] = val;
			
			for (int j = 0; j < 10; j++)
			{
				sum += average[i][j];
			}			

			sum1 += val;
			sum2 += sum / 10;
			
			Serial.print(String(val) + "/" + String(sum / 10) + "/" + String(ANALOG_INPUT_TRANSLATION[sum / 10]) + "\t");
		}

		Serial.println(String(sum1 / 12) + "//" + String(sum2 / 12) + "//" + String(ANALOG_INPUT_TRANSLATION[sum2 / 12]));

		ptr = (ptr + 1) % 10;

		Serial.print("\n");
	}

}
