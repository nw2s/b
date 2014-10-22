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
#include <GridOto.h>
#include <Grid.h>
#include <Usb.h>

using namespace nw2s;


GridOto* grid;

void setup() 
{
	Serial.begin(19200);
	Serial.print("\n\nStarting...\n");

	EventManager::initialize();

	/* Setup a variable clock */
	Clock* vclock = VariableClock::create(10, 240, DUE_IN_A00, 16);

	int notes0[][2] = { { 3, 1 }, { 3, 2 }, { 3, 3 }, { 3, 4 }, { 3, 5 }, { 3, 6 }, { 3, 7 }, { 4, 8 }, { 3, 1 }, { 3, 2 }, { 3, 3 }, { 3, 4 }, { 3, 5 }, { 3, 6 }, { 3, 7 }, { 4, 8 } };
	int notes1[][2] = { { 1, 1 }, { 1, 3 }, { 1, 5 }, { 2, 1 }, { 2, 3 }, { 2, 5 }, { 3, 1 }, { 3, 3 }, { 3, 1 }, { 3, 2 }, { 3, 3 }, { 3, 4 }, { 3, 5 }, { 3, 6 }, { 3, 7 }, { 4, 8 } };
	int notes2[][2] = { { 1, 1 }, { 0, 1 }, { 0, 5 }, { 1, 1 }, { 1, 5 }, { 2, 1 }, { 2, 5 }, { 3, 1 }, { 3, 1 }, { 3, 2 }, { 3, 3 }, { 3, 4 }, { 3, 5 }, { 3, 6 }, { 3, 7 }, { 4, 8 } };
	int notes3[][2] = { { 2, 1 }, { 1, 1 }, { 1, 1 }, { 1, 1 }, { 2, 1 }, { 2, 1 }, { 2, 1 }, { 1, 1 }, { 3, 1 }, { 3, 2 }, { 3, 3 }, { 3, 4 }, { 3, 5 }, { 3, 6 }, { 3, 7 }, { 4, 8 } };
	
	// grid = GridOto::create(DEVICE_SERIES, 8, 8, DIV_SIXTEENTH, C, Key::SCALE_MINOR, DUE_OUT_D14, DUE_SPI_4822_14, notes0, DUE_OUT_D15, DUE_SPI_4822_15, notes1, DUE_OUT_D12, DUE_SPI_4822_12, notes2, DUE_OUT_D13, DUE_SPI_4822_13, notes3);
	grid = GridOto::create(DEVICE_GRIDS, 16, 16, DIV_SIXTEENTH, C, Key::SCALE_MINOR, DUE_OUT_D14, DUE_SPI_4822_14, notes0, DUE_OUT_D15, DUE_SPI_4822_15, notes1, DUE_OUT_D12, DUE_SPI_4822_12, notes2, DUE_OUT_D13, DUE_SPI_4822_13, notes3);

	Sequencer* cvsequencer4 = CVSequencer::create(1000, 1000, DIV_EIGHTH, DUE_SPI_4822_09);
	Sequencer* cvsequencer5 = CVSequencer::create(1000, 5000, DIV_EIGHTH, DUE_SPI_4822_10);
	Sequencer* cvsequencer6 = CVSequencer::create(1000, 5000, DIV_EIGHTH, DUE_SPI_4822_11);

	vclock->registerDevice(grid);
	vclock->registerDevice(cvsequencer4);
	vclock->registerDevice(cvsequencer5);
	vclock->registerDevice(cvsequencer6);

	EventManager::registerDevice(vclock);
}

void loop()
{
	EventManager::loop();
	grid->task();	
}




