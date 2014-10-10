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
#include <IO.h>
#include <Trigger.h>
#include <Clock.h>
#include <Slew.h>
#include <Sequence.h>
#include <SPI.h>
#include <SD.h>
#include <Wire.h>
#include <aJSON.h>
#include <Gate.h>
#include <GridTrigger.h>
#include <Grid.h>
#include <Usb.h>

using namespace nw2s;


GridTriggerController* grid;

void setup() 
{
	Serial.begin(19200);
	Serial.print("\n\nStarting...\n");

	EventManager::initialize();

	/* Setup a variable clock */
	Clock* vclock = VariableClock::create(10, 240, DUE_IN_A00, 16);

	grid = GridTriggerController::create(16, 8, DIV_SIXTEENTH, DUE_OUT_D00, DUE_OUT_D01, DUE_OUT_D02, DUE_OUT_D03, DUE_OUT_D04, DUE_OUT_D05, DUE_OUT_D06);

	grid->setShuffleToggle(DUE_IN_D0);

	vclock->registerDevice(grid);
	EventManager::registerDevice(vclock);

}

void loop()
{
	EventManager::loop();
	grid->task();
	
}

