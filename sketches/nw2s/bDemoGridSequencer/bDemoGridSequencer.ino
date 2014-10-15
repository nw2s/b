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
#include <GridNoteSequencer.h>
#include <Grid.h>
#include <Usb.h>

using namespace nw2s;


GridNoteSequencer* grid;

void setup() 
{
	Serial.begin(19200);
	Serial.print("\n\nStarting...\n");

	EventManager::initialize();

	/* Setup a variable clock */
	Clock* vclock = VariableClock::create(10, 240, DUE_IN_A00, 16);
	Trigger* trigger = Trigger::create(DUE_OUT_D15, DIV_QUARTER);
	vclock->registerDevice(trigger);

	int notes0[][2] = { { 1, 1 }, { 1, 3 }, { 1, 4 }, { 1, 5 }, { 1, 7 }, { 2, 1 }, { 2, 3 } };
	int notes1[][2] = { { 2, 1 }, { 2, 3 }, { 2, 4 }, { 2, 5 }, { 2, 7 }, { 3, 1 }, { 3, 3 } };
	
	grid = GridNoteSequencer::create(DEVICE_SERIES, 8, 8, DIV_SIXTEENTH, C, Key::SCALE_MAJOR, DUE_OUT_D00, DUE_SPI_4822_00, notes0, DUE_OUT_D01, DUE_SPI_4822_01, notes1, DIGITAL_OUT_NONE, ANALOG_OUT_NONE, NULL, DIGITAL_OUT_NONE, ANALOG_OUT_NONE, NULL);

	vclock->registerDevice(grid);

	EventManager::registerDevice(vclock);
}

void loop()
{
	EventManager::loop();
	grid->task();	
}




