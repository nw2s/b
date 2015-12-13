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
#include <SignalData.h>
#include <Loop.h>

using namespace nw2s;


void setup() 
{
	Serial.begin(19200);
	Serial.print("\n\nStarting...\n");

	EventManager::initialize();

	/* Setup the mechanical noise as a free running loop */
	Looper* looper2 = Looper::create(DUE_DAC0, "mech", "mech01.raw", SR_24000);

	EventManager::registerDevice(looper2);

	/* Set up the drum loop as a clocked loop. It will reset every two beats */
	FixedClock* fixedclock = FixedClock::create(120, 16);

	//Looper* looper1 = Looper::create(DUE_DAC1, "melodic", "ebgtr120.raw", SR_24000);
	Looper* looper1 = Looper::create(DUE_DAC1, "test", "sin220.raw", SR_24000);

	EventManager::registerDevice(looper1);
}

void loop() 
{
	EventManager::loop();	
}




