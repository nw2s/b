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
#include <Key.h>
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

/*

	This demo sets up a bunch of weird clock divisions/triggers on the nw2s::b's D8 - D15. 

	The first thing that every nw2s::b sketch needs is an EventManager. 
	
	There are a number of clocks available to run the triggers which this sketch is meant to illustrate.
	Simply uncomment the one you'd like to test. 

	The EventManager is the top-level object that is what makes things happen. To the event manager, you
	attach time based devices such as clocks, random CV generators, and so forth. 
	
	In this case, we're attaching a clock. Clocks in turn generate beats and to them, you attach
	beat-based devices such as triggers and sequencers. 

*/

using namespace nw2s;

void setup() 
{
	Serial.begin(19200);

	EventManager::initialize();

	/* Fixed clock running at 75BPM */
	//Clock* democlock = FixedClock::create(75, 16);
	
	/* Variable clock running between 75 and 125BPM based on A0 input value */
	//Clock* democlock = VariableClock::create(25, 125, DUE_IN_A0, 16);

	/* Random clock running between 75 and 80BPM */
	//Clock* democlock = RandomTempoClock::create(75, 80, 16);

	/* Clock slaving to the Ardcore clock input */
	Clock* democlock = TapTempoClock::create(DUE_IN_D0, DUE_IN_D1, 16);

	/* Clock that just passes through resets() */
	//Clock* democlock = PassthruClock::create(DUE_IN_D0, 16);

	/* Register the clock with the EventManager */
	EventManager::registerDevice(democlock);
	
	/* Gate running quarter notes on expander port 0 */
	Trigger* trigger0 = Trigger::create(DUE_OUT_D08, DIV_QUARTER);
	democlock->registerDevice(trigger0);

	Trigger* trigger1 = Trigger::create(DUE_OUT_D09, DIV_HALF);
	democlock->registerDevice(trigger1);
	
	Trigger* trigger2 = Trigger::create(DUE_OUT_D10, DIV_EIGHTH_DOT);
	democlock->registerDevice(trigger2);
	
	Trigger* trigger3 = Trigger::create(DUE_OUT_D11, DIV_QUARTER_TRIPLET);
	democlock->registerDevice(trigger3);
	
	Trigger* trigger4 = Trigger::create(DUE_OUT_D12, DIV_SIXTEENTH);
	democlock->registerDevice(trigger4);
	
	Trigger* trigger5 = Trigger::create(DUE_OUT_D13, DIV_WHOLE);
	democlock->registerDevice(trigger5);
	
	Trigger* trigger6 = Trigger::create(DUE_OUT_D14, DIV_EIGHTH_TRIPLET);
	democlock->registerDevice(trigger6);
	
	Trigger* trigger7 = Trigger::create(DUE_OUT_D15, DIV_THIRTYSECOND);
	democlock->registerDevice(trigger7);
	
}

void loop() 
{
	EventManager::loop();
	
}
