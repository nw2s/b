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

void setup() 
{
	Serial.begin(19200);
	Serial.println("\n\nStarting...");

	/* Setup analog inputs */
	analogReadResolution(12);
	
}

void loop() 
{	
	if (millis() % 100 == 0)
	{
		Serial.print(String(analogRead(A0)) + "\t");
		Serial.print(String(analogRead(A1)) + "\t");
		Serial.print(String(analogRead(A2)) + "\t");
		Serial.print(String(analogRead(A3)) + "\t");
		Serial.print(String(analogRead(A4)) + "\t");
		Serial.print(String(analogRead(A5)) + "\t");
		Serial.print(String(analogRead(A6)) + "\t");
		Serial.print(String(analogRead(A7)) + "\t");
		Serial.print(String(analogRead(A8)) + "\t");
		Serial.print(String(analogRead(A9)) + "\t");
		Serial.print(String(analogRead(A10)) + "\t");
		Serial.print(String(analogRead(A11)) + "\n");
	}

}
