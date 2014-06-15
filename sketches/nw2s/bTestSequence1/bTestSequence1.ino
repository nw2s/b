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

using namespace nw2s;

void setup() 
{
	Serial.begin(19200);
	Serial.println("\n\nStarting...");

	EventManager::initialize();

	Clock* vclock = VariableClock::create(30, 30, DUE_IN_A00, 16);
	//FixedClock* fixedclock = FixedClock::create(120, 16);

	/* Iterate over octaves on all the outputs for tuning */
	SequenceNote notelist3[6] = { {0,1}, {1,1}, {2,1}, {3,1}, {4,1}, {5,1} };
	std::vector<SequenceNote>* notes3 = new vector<SequenceNote>(notelist3, notelist3 + 6);

	NoteSequencer* s01 = NoteSequencer::create(notes3, C, Key::SCALE_MAJOR, DIV_QUARTER, DUE_SPI_4822_00, false);
	NoteSequencer* s02 = NoteSequencer::create(notes3, C, Key::SCALE_MAJOR, DIV_QUARTER, DUE_SPI_4822_01, false);
	NoteSequencer* s03 = NoteSequencer::create(notes3, C, Key::SCALE_MAJOR, DIV_QUARTER, DUE_SPI_4822_02, false);
	NoteSequencer* s04 = NoteSequencer::create(notes3, C, Key::SCALE_MAJOR, DIV_QUARTER, DUE_SPI_4822_03, false);
	NoteSequencer* s05 = NoteSequencer::create(notes3, C, Key::SCALE_MAJOR, DIV_QUARTER, DUE_SPI_4822_04, false);
	NoteSequencer* s06 = NoteSequencer::create(notes3, C, Key::SCALE_MAJOR, DIV_QUARTER, DUE_SPI_4822_05, false);
	NoteSequencer* s07 = NoteSequencer::create(notes3, C, Key::SCALE_MAJOR, DIV_QUARTER, DUE_SPI_4822_06, false);
	NoteSequencer* s08 = NoteSequencer::create(notes3, C, Key::SCALE_MAJOR, DIV_QUARTER, DUE_SPI_4822_07, false);
	NoteSequencer* s09 = NoteSequencer::create(notes3, C, Key::SCALE_MAJOR, DIV_QUARTER, DUE_SPI_4822_08, false);
	NoteSequencer* s10 = NoteSequencer::create(notes3, C, Key::SCALE_MAJOR, DIV_QUARTER, DUE_SPI_4822_09, false);
	NoteSequencer* s11 = NoteSequencer::create(notes3, C, Key::SCALE_MAJOR, DIV_QUARTER, DUE_SPI_4822_10, false);
	NoteSequencer* s12 = NoteSequencer::create(notes3, C, Key::SCALE_MAJOR, DIV_QUARTER, DUE_SPI_4822_11, false);
	NoteSequencer* s13 = NoteSequencer::create(notes3, C, Key::SCALE_MAJOR, DIV_QUARTER, DUE_SPI_4822_12, false);
	NoteSequencer* s14 = NoteSequencer::create(notes3, C, Key::SCALE_MAJOR, DIV_QUARTER, DUE_SPI_4822_13, false);
	NoteSequencer* s15 = NoteSequencer::create(notes3, C, Key::SCALE_MAJOR, DIV_QUARTER, DUE_SPI_4822_14, false);
	NoteSequencer* s16 = NoteSequencer::create(notes3, C, Key::SCALE_MAJOR, DIV_QUARTER, DUE_SPI_4822_15, false);
	
	vclock->registerDevice(s01);
	vclock->registerDevice(s02);
	vclock->registerDevice(s03);
	vclock->registerDevice(s04);
	vclock->registerDevice(s05);
	vclock->registerDevice(s06);
	vclock->registerDevice(s07);
	vclock->registerDevice(s08);
	vclock->registerDevice(s09);
	vclock->registerDevice(s10);
	vclock->registerDevice(s11);
	vclock->registerDevice(s12);
	vclock->registerDevice(s13);
	vclock->registerDevice(s14);
	vclock->registerDevice(s15);
	vclock->registerDevice(s16);

	EventManager::registerDevice(vclock);
	

	/* Setup triggers on all the outputs to make sure they're working */
	Trigger* trigger01 = Trigger::create(DUE_OUT_D00, DIV_WHOLE);
	Trigger* trigger02 = Trigger::create(DUE_OUT_D01, DIV_HALF_DOT);
	Trigger* trigger03 = Trigger::create(DUE_OUT_D02, DIV_HALF);
	Trigger* trigger04 = Trigger::create(DUE_OUT_D03, DIV_QUARTER_DOT);
	Trigger* trigger05 = Trigger::create(DUE_OUT_D04, DIV_QUARTER);
	Trigger* trigger06 = Trigger::create(DUE_OUT_D05, DIV_QUARTER_TRIPLET);
	Trigger* trigger07 = Trigger::create(DUE_OUT_D06, DIV_EIGHTH_DOT);
	Trigger* trigger08 = Trigger::create(DUE_OUT_D07, DIV_EIGHTH);
	Trigger* trigger09 = Trigger::create(DUE_OUT_D08, DIV_EIGHTH_TRIPLET);
	Trigger* trigger10 = Trigger::create(DUE_OUT_D09, DIV_SIXTEENTH_DOT);
	Trigger* trigger11 = Trigger::create(DUE_OUT_D10, DIV_SIXTEENTH);
	Trigger* trigger12 = Trigger::create(DUE_OUT_D11, DIV_SIXTEENTH_TRIPLET);
	Trigger* trigger13 = Trigger::create(DUE_OUT_D12, DIV_THIRTYSECOND_DOT);
	Trigger* trigger14 = Trigger::create(DUE_OUT_D13, DIV_THIRTYSECOND);
	Trigger* trigger15 = Trigger::create(DUE_OUT_D14, DIV_THIRTYSECOND_TRIPLET);
	Trigger* trigger16 = Trigger::create(DUE_OUT_D15, DIV_QUARTER);

	vclock->registerDevice(trigger01);
	vclock->registerDevice(trigger02);
	vclock->registerDevice(trigger03);
	vclock->registerDevice(trigger04);
	vclock->registerDevice(trigger05);
	vclock->registerDevice(trigger06);
	vclock->registerDevice(trigger07);
	vclock->registerDevice(trigger08);
	vclock->registerDevice(trigger09);
	vclock->registerDevice(trigger10);
	vclock->registerDevice(trigger11);
	vclock->registerDevice(trigger12);
	vclock->registerDevice(trigger13);
	vclock->registerDevice(trigger14);
	vclock->registerDevice(trigger15);
	vclock->registerDevice(trigger16);	
}

void loop() 
{
	EventManager::loop();

}
