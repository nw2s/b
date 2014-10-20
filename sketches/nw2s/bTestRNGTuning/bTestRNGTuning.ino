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
#include <aJSON.h>
#include <Usb.h>

using namespace nw2s;

/*

	This shetch is used to tune the noise circuit to produce a voltage that will 
	produce a random sequence of ones and zeros. Because we are using the analog
	inputs for more useful tasks, we're just using the noise circuit to generate 
	digital noise. 
	
	To tune the circuit, load this sketch with the serial monitor running. The 
	monitor will show a the ratio of zeros to ones. With a twenty-five-turn
	pot, the difference between all zeros and all ones is only about a half turn,
	so watch closely.
	
	As long as the ratio of ones to zeros is about 33 to 66%, then the mix 
	will be sufficient to generate whiteish noise.
	
*/


void setup() 
{
	Serial.begin(19200);
	Serial.print("\n\nStarting...\n");

	EventManager::initialize();
}

void loop() 
{	
	unsigned long zeros = 0;
	unsigned long ones = 0;
	unsigned long t = micros();
	
	/* Limit the frequency of these tests to make it readable */
	if (millis() % 100 == 0)
	{
		for (int i = 0; i < 10000; i++)
		{
			if (digitalRead(DUE_IN_DIGITAL_NOISE) == HIGH)
			{
				ones++;
			}
			else
			{
				zeros++;
			}
		}
		
		int ratio = (zeros * 100) / (zeros + ones);

		Serial.println("zeros: " + String(zeros) + "\tones: " + String(ones) + "\tratio: " + String(ratio) + "%");
	}		
}
