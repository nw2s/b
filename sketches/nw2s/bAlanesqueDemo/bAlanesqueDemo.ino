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
#include <ShiftRegister.h>
#include <aJSON.h>

using namespace nw2s;

void setup() 
{
	Serial.begin(19200);
	Serial.println("\n\nStarting...");

	EventManager::initialize();

	/* Setup a variable clock */
	Clock* vclock = VariableClock::create(20, 240, DUE_IN_A00, 16);

	RandomLoopingShiftRegister* shiftregister = RandomLoopingShiftRegister::create(16, DUE_IN_A01, DIV_SIXTEENTH);
	
	shiftregister->setCVOut(DUE_SPI_4822_02);
	shiftregister->setDelayedCVOut(DUE_SPI_4822_08, 2);

	shiftregister->setKey(C, MAJOR);
	shiftregister->setNoteOut(DUE_SPI_4822_04);
	shiftregister->setDelayedNoteOut(DUE_SPI_4822_09, 1);
	
	shiftregister->setGateOut(1, DUE_OUT_D00, 250);
	shiftregister->setGateOut(2, DUE_OUT_D08, 250);
	shiftregister->setGateOut(3, DUE_OUT_D01, 250);
	shiftregister->setGateOut(4, DUE_OUT_D09, 250);
	shiftregister->setGateOut(5, DUE_OUT_D02, 250);
	shiftregister->setGateOut(6, DUE_OUT_D10, 250);
	shiftregister->setGateOut(7, DUE_OUT_D03, 250);
	shiftregister->setGateOut(8, DUE_OUT_D11, 250);
	
	shiftregister->setTriggerOut(1, DUE_OUT_D04);
	shiftregister->setTriggerOut(2, DUE_OUT_D12);
	shiftregister->setTriggerOut(3, DUE_OUT_D05);
	shiftregister->setTriggerOut(4, DUE_OUT_D13);
	
	shiftregister->setLogicalOrTrigger(DUE_OUT_D06, 1, 2, 4, 8);
	shiftregister->setLogicalAndTrigger(DUE_OUT_D14, 1, 2, 4, 8);
	shiftregister->setLogicalOrGate(DUE_OUT_D07, 500, 2, 7);
	shiftregister->setLogicalAndGate(DUE_OUT_D15, 500, 5, 8);
	
	shiftregister->setSequencerInputs(DUE_IN_A02, DUE_IN_A03, DUE_IN_A04, DUE_IN_A05, DUE_IN_A08, DUE_IN_A09, DUE_IN_A10, DUE_IN_A11);
	shiftregister->setSequencerScaleInput(DUE_IN_A06);
	shiftregister->setSequencerCVOut(DUE_SPI_4822_07);
	shiftregister->setSequencerNoteOut(DUE_SPI_4822_15);
	
	vclock->registerDevice(shiftregister);
	
	EventManager::registerDevice(vclock);
}

void loop() 
{
	EventManager::loop();
}
