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
#include <Entropy.h>


using namespace nw2s;

void setup() 
{
	Serial.begin(19200);
	Serial.print("\n\nStarting...\n");

	for (int i = 0; i < 16; i++)
	{
		pinMode(22 + i, OUTPUT);
	}
}

unsigned long t_global = 0;

void loop() 
{
	unsigned long t = millis();

	for (int i = 0; i < 16; i++)
	{
		if (t % 1000 == i * 20)
		{
			digitalWrite(22 + i, HIGH);
			Serial.println(String(22 + i));
		}
		else if (t % 500 == i * 20)
		{
			digitalWrite(22 + i, LOW);
		}
	}
}
