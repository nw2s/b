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

#include <EventManager.h>
#include <IO.h>

using namespace nw2s;

PinDigitalIn inputs[9] = {
	
	DUE_IN_DIGITAL_NOISE,
	DUE_IN_D0,
	DUE_IN_D1,
	DUE_IN_D2,
	DUE_IN_D3,
	DUE_IN_D4,
	DUE_IN_D5,
	DUE_IN_D6,
	DUE_IN_D7	
};

void setup() 
{
	Serial.begin(19200);
	Serial.println("Starting...");

	EventManager::initialize();
}

void loop() 
{	
	if (millis() % 250 == 0)
	{
		for (int i = 0; i < 9; i++)
		{			
			Serial.print(String(digitalRead(inputs[i])) + "\t");
		}

		Serial.print("\n");
	}

}
