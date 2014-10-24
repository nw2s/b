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
#include <Oscillator.h>
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
	Trigger* trigger = Trigger::create(DUE_OUT_D15, DIV_QUARTER);
	vclock->registerDevice(trigger);

	grid = GridTriggerController::create(DEVICE_GRIDS, 16, 8, DIV_SIXTEENTH, DUE_OUT_D00, DUE_OUT_D01, DUE_OUT_D02, DUE_OUT_D03,DUE_OUT_D04, DUE_OUT_D05, DUE_OUT_D06);
	grid->setProbabilityInput(DUE_IN_A01);

	//grid = GridTriggerController::create(DEVICE_40H_TRELLIS, 16, 8, DIV_SIXTEENTH, DUE_OUT_D00, DUE_OUT_D01, DUE_OUT_D02, DUE_OUT_D03,DUE_OUT_D04, DUE_OUT_D05, DUE_OUT_D06);

	//grid = GridTriggerController::create(DEVICE_SERIES, 8, 8, DIV_SIXTEENTH, DUE_OUT_D00, DUE_OUT_D01, DUE_OUT_D02, DUE_OUT_D03, DUE_OUT_D04, DUE_OUT_D05, DUE_OUT_D06);

	grid->setShuffleToggle(DUE_IN_D0);
	grid->setShuffleScopeInput(DUE_IN_D2);
	grid->setNextPageToggle(DUE_IN_D3);

	vclock->registerDevice(grid);


	VCO* vco = DiscreteNoise::create(DUE_DAC0, DUE_IN_A01);
	EventManager::registerDevice(vco);
	
	Sequencer* cvsequencer1 = CVSequencer::create(0, 5000, DIV_QUARTER, DUE_SPI_4822_11);
	Sequencer* cvsequencer2 = CVSequencer::create(1000, 3000, DIV_EIGHTH, DUE_SPI_4822_13);
	Sequencer* cvsequencer3 = CVSequencer::create(1000, 1500, DIV_SIXTEENTH, DUE_SPI_4822_15);
	Sequencer* cvsequencer4 = CVSequencer::create(1000, 5000, DIV_THIRTYSECOND, DUE_SPI_4822_14);
	Sequencer* cvsequencer5 = CVSequencer::create(1000, 5000, DIV_THIRTYSECOND, DUE_SPI_4822_12);
	Sequencer* cvsequencer6 = CVSequencer::create(1000, 5000, DIV_THIRTYSECOND, DUE_SPI_4822_10);
	
	vclock->registerDevice(cvsequencer1);
	vclock->registerDevice(cvsequencer2);
	vclock->registerDevice(cvsequencer3);
	vclock->registerDevice(cvsequencer4);
	vclock->registerDevice(cvsequencer5);
	vclock->registerDevice(cvsequencer6);
		
	// grid->setClockInput(DUE_IN_D1);
	// EventManager::registerDevice(grid);

	EventManager::registerDevice(vclock);

}

void loop()
{
	EventManager::loop();
	grid->task();
	
}

