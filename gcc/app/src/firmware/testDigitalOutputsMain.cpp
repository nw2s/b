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
#include <Clock.h>

using namespace nw2s;

PinDigitalOut outputs[16] = {
	
	DUE_OUT_D00,
	DUE_OUT_D01,
	DUE_OUT_D02,
	DUE_OUT_D03,
	DUE_OUT_D04,
	DUE_OUT_D05,
	DUE_OUT_D06,
	DUE_OUT_D07,
	DUE_OUT_D08,
	DUE_OUT_D09,
	DUE_OUT_D10,
	DUE_OUT_D11,
	DUE_OUT_D12,
	DUE_OUT_D13,
	DUE_OUT_D14,
	DUE_OUT_D15,

};

int counter = 0;
int lastcounter = 15;

void setup() 
{
	Serial.begin(19200);
	Serial.println("Starting...");

	EventManager::initialize();
}

void loop() 
{	
	if (millis() % 100 == 0)
	{
		digitalWrite(outputs[lastcounter], LOW);
		digitalWrite(outputs[counter], HIGH);
		
		Serial.println(String(counter) + " " + String(outputs[counter]));
		
		lastcounter = counter;
		counter = (counter + 1) % 16;
	}

	EventManager::loop();
}
