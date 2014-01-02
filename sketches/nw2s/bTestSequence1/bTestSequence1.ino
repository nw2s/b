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

	FixedClock* fixedclock = FixedClock::create(120, 16);

	/* Iterate over octaves on all the outputs for tuning */
	SequenceNote notelist3[6] = { {0,1}, {1,1}, {2,1}, {3,1}, {4,1}, {5,1} };
	std::vector<SequenceNote>* notes3 = new vector<SequenceNote>(notelist3, notelist3 + 6);

	NoteSequencer* s01 = NoteSequencer::create(notes3, C, MAJOR, DIV_QUARTER, DUE_SPI_4822_00, false);
	NoteSequencer* s02 = NoteSequencer::create(notes3, C, MAJOR, DIV_QUARTER, DUE_SPI_4822_01, false);
	NoteSequencer* s03 = NoteSequencer::create(notes3, C, MAJOR, DIV_QUARTER, DUE_SPI_4822_02, false);
	NoteSequencer* s04 = NoteSequencer::create(notes3, C, MAJOR, DIV_QUARTER, DUE_SPI_4822_03, false);
	NoteSequencer* s05 = NoteSequencer::create(notes3, C, MAJOR, DIV_QUARTER, DUE_SPI_4822_04, false);
	NoteSequencer* s06 = NoteSequencer::create(notes3, C, MAJOR, DIV_QUARTER, DUE_SPI_4822_05, false);
	NoteSequencer* s07 = NoteSequencer::create(notes3, C, MAJOR, DIV_QUARTER, DUE_SPI_4822_06, false);
	NoteSequencer* s08 = NoteSequencer::create(notes3, C, MAJOR, DIV_QUARTER, DUE_SPI_4822_07, false);
	NoteSequencer* s09 = NoteSequencer::create(notes3, C, MAJOR, DIV_QUARTER, DUE_SPI_4822_08, false);
	NoteSequencer* s10 = NoteSequencer::create(notes3, C, MAJOR, DIV_QUARTER, DUE_SPI_4822_09, false);
	NoteSequencer* s11 = NoteSequencer::create(notes3, C, MAJOR, DIV_QUARTER, DUE_SPI_4822_10, false);
	NoteSequencer* s12 = NoteSequencer::create(notes3, C, MAJOR, DIV_QUARTER, DUE_SPI_4822_11, false);
	NoteSequencer* s13 = NoteSequencer::create(notes3, C, MAJOR, DIV_QUARTER, DUE_SPI_4822_12, false);
	NoteSequencer* s14 = NoteSequencer::create(notes3, C, MAJOR, DIV_QUARTER, DUE_SPI_4822_13, false);
	NoteSequencer* s15 = NoteSequencer::create(notes3, C, MAJOR, DIV_QUARTER, DUE_SPI_4822_14, false);
	NoteSequencer* s16 = NoteSequencer::create(notes3, C, MAJOR, DIV_QUARTER, DUE_SPI_4822_15, false);
	
	fixedclock->registerdevice(s01);
	fixedclock->registerdevice(s02);
	fixedclock->registerdevice(s03);
	fixedclock->registerdevice(s04);
	fixedclock->registerdevice(s05);
	fixedclock->registerdevice(s06);
	fixedclock->registerdevice(s07);
	fixedclock->registerdevice(s08);
	fixedclock->registerdevice(s09);
	fixedclock->registerdevice(s10);
	fixedclock->registerdevice(s11);
	fixedclock->registerdevice(s12);
	fixedclock->registerdevice(s13);
	fixedclock->registerdevice(s14);
	fixedclock->registerdevice(s15);
	fixedclock->registerdevice(s16);

	EventManager::registerdevice(fixedclock);
	

	/* Setup triggers on all the outputs to make sure they're working */
	Trigger* trigger01 = Trigger::create(DUE_OUT_D00, DIV_WHOLE);
	Trigger* trigger02 = Trigger::create(DUE_OUT_D08, DIV_HALF_DOT);
	Trigger* trigger03 = Trigger::create(DUE_OUT_D01, DIV_HALF);
	Trigger* trigger04 = Trigger::create(DUE_OUT_D09, DIV_QUARTER_DOT);
	Trigger* trigger05 = Trigger::create(DUE_OUT_D02, DIV_QUARTER);
	Trigger* trigger06 = Trigger::create(DUE_OUT_D10, DIV_QUARTER_TRIPLET);
	Trigger* trigger07 = Trigger::create(DUE_OUT_D03, DIV_EIGHTH_DOT);
	Trigger* trigger08 = Trigger::create(DUE_OUT_D11, DIV_EIGHTH);
	Trigger* trigger09 = Trigger::create(DUE_OUT_D04, DIV_EIGHTH_TRIPLET);
	Trigger* trigger10 = Trigger::create(DUE_OUT_D12, DIV_SIXTEENTH_DOT);
	Trigger* trigger11 = Trigger::create(DUE_OUT_D05, DIV_SIXTEENTH);
	Trigger* trigger12 = Trigger::create(DUE_OUT_D13, DIV_SIXTEENTH_TRIPLET);
	Trigger* trigger13 = Trigger::create(DUE_OUT_D06, DIV_THIRTYSECOND_DOT);
	Trigger* trigger14 = Trigger::create(DUE_OUT_D14, DIV_THIRTYSECOND);
	Trigger* trigger15 = Trigger::create(DUE_OUT_D07, DIV_THIRTYSECOND_TRIPLET);
	Trigger* trigger16 = Trigger::create(DUE_OUT_D15, DIV_QUARTER);

	fixedclock->registerdevice(trigger01);
	fixedclock->registerdevice(trigger02);
	fixedclock->registerdevice(trigger03);
	fixedclock->registerdevice(trigger04);
	fixedclock->registerdevice(trigger05);
	fixedclock->registerdevice(trigger06);
	fixedclock->registerdevice(trigger07);
	fixedclock->registerdevice(trigger08);
	fixedclock->registerdevice(trigger09);
	fixedclock->registerdevice(trigger10);
	fixedclock->registerdevice(trigger11);
	fixedclock->registerdevice(trigger12);
	fixedclock->registerdevice(trigger13);
	fixedclock->registerdevice(trigger14);
	fixedclock->registerdevice(trigger15);
	fixedclock->registerdevice(trigger16);
	
	
	//TODO: Setup a simple square wave on two analog outputs for level setting
	
	
	//TODO: Watch analog input changes and write the values that change
	
	
	//TODO: Watch digital input changes and write the values that change
	
	
	
}

void loop() 
{
	EventManager::loop();

}
