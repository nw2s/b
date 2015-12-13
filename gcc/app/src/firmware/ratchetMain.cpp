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
#include <RatchetDivider.h>

using namespace nw2s;


void setup() 
{
	Serial.begin(19200);
	Serial.print("\n\nStarting...\n");

	EventManager::initialize();

	TapTempoClock* tempoclock = TapTempoClock::create(DUE_IN_D0, DUE_IN_D1, 16);

	/* Creating a trigger on the quarter note just to test the tap tempo */
	Trigger* quarter = Trigger::create(DUE_OUT_D00, DIV_QUARTER);

	tempoclock->registerDevice(quarter);

	EventManager::registerDevice(tempoclock);
	
	RatchetDivider* divider1 = RatchetDivider::create(RATCHET_LIMIT_OFF, DUE_IN_A00, DUE_IN_A01, DUE_OUT_D01);
	RatchetDivider* divider2 = RatchetDivider::create(RATCHET_LIMIT_ODD, DUE_IN_A02, DUE_IN_A03, DUE_OUT_D02);
	RatchetDivider* divider3 = RatchetDivider::create(RATCHET_LIMIT_EVEN, DUE_IN_A02, DUE_IN_A03, DUE_OUT_D03);
	RatchetDivider* divider4 = RatchetDivider::create(RATCHET_LIMIT_PRIMES, DUE_IN_A04, DUE_IN_A05, DUE_OUT_D04);
	RatchetDivider* divider5 = RatchetDivider::create(RATCHET_LIMIT_TRIP, DUE_IN_A06, DUE_IN_A07, DUE_OUT_D05);
	RatchetDivider* divider6 = RatchetDivider::create(RATCHET_LIMIT_FIBONACCI, DUE_IN_A08, DUE_IN_A09, DUE_OUT_D06);
	RatchetDivider* divider7 = RatchetDivider::create(RATCHET_LIMIT_POWEROF2, DUE_IN_A10, DUE_IN_A11, DUE_OUT_D07);

	tempoclock->registerDevice(divider1);
	tempoclock->registerDevice(divider2);
	tempoclock->registerDevice(divider3);
	tempoclock->registerDevice(divider4);
	tempoclock->registerDevice(divider5);
	tempoclock->registerDevice(divider6);
	tempoclock->registerDevice(divider7);
}

void loop() 
{
	EventManager::loop();	
}




