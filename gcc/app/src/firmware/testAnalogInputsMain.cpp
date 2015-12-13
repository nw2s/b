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

/*

	This sketch is used to tune the bias voltage of the input stage. The input circuit, unlike
	the output circuit has a fixed gain stage with a variable bias. The bias is used to adjust
	the zero point of the bipolar inputs. 

	When tuning this portion of the circuit, first ensure that all of the input pots are turned
	completel counter-clockwise (at zero). 

	Upload the sketch and open the console window. You will see 13 columns of numbers. The values
	look like this:

	2017/2016/-2

		The first number is the instantaneous raw value coming from the DAC. These numbers are 
		12-bit numbers and range from 0 to 4096. 

		The second number is an average of the last 10 raw values. 
	
		The third number is the average value converted to millivolts.

	The first twelve columns represent the 12 individual inputs. The 13th number is the average
	of all of the inputs.

	You should see that all of the inputs are within a few millivolts of each other. Turn the
	INPUT_BIAS pot until you see the millivolt values approach zero. Use the thirteenth column
	to get the average of the inputs as close to zero as possible. If the pot seems to be a little
	sticky and not settle excactly where you need it, simply turn it back and forth a couple of
	times close to the position where it seems that it should be. Make smaller and smaller 
	adjustments and allow the average to settle as you get closer to the target value. 

	Once the bias is adjusted, the circuits should be tested. First, turn each pot completely from
	zero to full and back to zero. You should see the millivolt values go from 0 to 5000 and back to
	0 (or close to zero depending on the resistor errors).

	Then, test the input jacks the same way. When testing the input jack, be sure to turn the pot
	completely to the clockwise position to ensure that the signal is not attenuated at all. It is
	also useful to use a bipolar signal on the input jacks to ensure that both positive and negative
	swings are working. A through-zero modulator is most useful here. Something like the Doepfer 
	A-145 is sufficient. 

	Note that most of these test sketches are not meant to be examples of how best to perform
	any particular task, but rather they are meant to test raw functionality. Please don't 
	use these as sample of Good Things.
	
*/


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

	EventManager::initialize();

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
			int val = ::analogRead(inputs[i]);
			
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
	
	EventManager::loop();

}
