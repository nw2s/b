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
#include <Trigger.h>
#include <Clock.h>
#include <IO.h>

/*

	This demo sets up a bunch of weird clock divisions/triggers on the Ardcore expansion's D0 - D7. 
	
	There are a number of clocks available to run the triggers which this sketch is meant to illustrate.

*/

using namespace nw2s;

void setup() 
{
	Serial.begin(19200);
	Serial.print("\n\nStarting...\n");

	EventManager::initialize();

	/* Fixed clock running at 75BPM */
	//Clock* democlock = FixedClock::create(75, 16);
	
	/* Variable clock running between 75 and 125BPM based on A0 input value */
	//Clock* democlock = VariableClock::create(25, 125, ARDCORE_IN_A0, 16);

	/* Variable clock running between 75 and 125BPM based on A0 input value */
	Clock* democlock = SlaveClock::create(ARDCORE_CLOCK_IN, 16);

	/* Register the clock with the EventManager */
	EventManager::registerdevice(democlock);
	
	/* Gate running quarter notes on expander port 0 */
	Trigger* trigger0 = Trigger::create(ARDCORE_OUT_EX_D0, DIV_QUARTER);
	democlock->registerdevice(trigger0);

	Trigger* trigger1 = Trigger::create(ARDCORE_OUT_EX_D1, DIV_HALF);
	democlock->registerdevice(trigger1);

	Trigger* trigger2 = Trigger::create(ARDCORE_OUT_EX_D2, DIV_EIGHTH_DOT);
	democlock->registerdevice(trigger2);

	Trigger* trigger3 = Trigger::create(ARDCORE_OUT_EX_D3, DIV_QUARTER_TRIPLET);
	democlock->registerdevice(trigger3);

	Trigger* trigger4 = Trigger::create(ARDCORE_OUT_EX_D4, DIV_SIXTEENTH);
	democlock->registerdevice(trigger4);

	Trigger* trigger5 = Trigger::create(ARDCORE_OUT_EX_D5, DIV_WHOLE);
	democlock->registerdevice(trigger5);

	Trigger* trigger6 = Trigger::create(ARDCORE_OUT_EX_D6, DIV_EIGHTH_TRIPLET);
	democlock->registerdevice(trigger6);

	Trigger* trigger7 = Trigger::create(ARDCORE_OUT_EX_D7, DIV_THIRTYSECOND);
	democlock->registerdevice(trigger7);
	
}

void loop() 
{
	EventManager::loop();
	
}
