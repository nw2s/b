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
#include <ShiftRegister.h>

using namespace nw2s;

void setup() 
{
	Serial.begin(19200);
	Serial.println("\n\nStarting...");

	EventManager::initialize();

	/* Setup a variable clock */
	Clock* vclock = VariableClock::create(60, 240, DUE_IN_A00, 16);

	RandomLoopingShiftRegister* shiftregister = RandomLoopingShiftRegister::create(16, DUE_IN_A01, DIV_QUARTER);
	
	shiftregister->setCVOut(DUE_SPI_4822_00);
	shiftregister->setDelayedCVOut(DUE_SPI_4822_01, 2);
	
	shiftregister->setTriggerOut(1, DUE_OUT_D00);
	shiftregister->setTriggerOut(2, DUE_OUT_D01);
	shiftregister->setTriggerOut(3, DUE_OUT_D02);
	shiftregister->setTriggerOut(4, DUE_OUT_D03);
	shiftregister->setTriggerOut(5, DUE_OUT_D04);
	shiftregister->setTriggerOut(6, DUE_OUT_D05);
	shiftregister->setTriggerOut(7, DUE_OUT_D06);
	shiftregister->setTriggerOut(8, DUE_OUT_D07);

	shiftregister->setGateOut(1, DUE_OUT_D08, 250);
	shiftregister->setGateOut(2, DUE_OUT_D09, 250);
	shiftregister->setGateOut(3, DUE_OUT_D10, 250);
	shiftregister->setGateOut(4, DUE_OUT_D11, 250);

	
	vclock->registerdevice(shiftregister);
	
	EventManager::registerdevice(vclock);
}

void loop() 
{
	EventManager::loop();
}
