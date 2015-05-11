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
#include <IO.h>
#include <SPI.h>
#include <SignalData.h>
#include <SD.h>
#include <Loop.h>
#include <Wire.h>
#include <aJSON.h>
#include <Usb.h>
#include <Trigger.h>

using namespace nw2s;


void setup() 
{
	Serial.begin(19200);
	Serial.print("\n\nStarting...\n");

	EventManager::initialize();

	TapTempoClock* tempoclock = TapTempoClock::create(DUE_IN_D0, 16);

	/* Creating a trigger on the quarter note just to test the tap tempo */
	Trigger* quarter = Trigger::create(DUE_OUT_D00, DIV_QUARTER);

	tempoclock->registerDevice(quarter);

	EventManager::registerDevice(tempoclock);
}

void loop() 
{
	EventManager::loop();	
}




