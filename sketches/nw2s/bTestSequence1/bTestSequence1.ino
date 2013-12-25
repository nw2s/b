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

using namespace nw2s;

void setup() 
{
	Serial.begin(19200);
	Serial.println("\n\nStarting...");

	EventManager::initialize();

	FixedClock* fixedclock = FixedClock::create(20, 16);

	SequenceNote notelist3[6] = { {0,1}, {1,1}, {2,1}, {3,1}, {4,1}, {5,1} };
	std::vector<SequenceNote>* notes3 = new vector<SequenceNote>(notelist3, notelist3 + 6);

	NoteSequencer* s01 = NoteSequencer::create(notes3, C, MAJOR, DIV_QUARTER, DUE_SPI_4822_00, false);
	NoteSequencer* s02 = NoteSequencer::create(notes3, C, MAJOR, DIV_QUARTER, DUE_SPI_4822_01, false);
	NoteSequencer* s03 = NoteSequencer::create(notes3, C, MAJOR, DIV_QUARTER, DUE_SPI_4822_02, false);
	NoteSequencer* s04 = NoteSequencer::create(notes3, C, MAJOR, DIV_QUARTER, DUE_SPI_4822_03, false);
	NoteSequencer* s05 = NoteSequencer::create(notes3, C, MAJOR, DIV_QUARTER, DUE_SPI_4822_04, false);
	NoteSequencer* s06 = NoteSequencer::create(notes3, C, MAJOR, DIV_QUARTER, DUE_SPI_4822_05, false);
	// NoteSequencer* s07 = NoteSequencer::create(notes3, C, MAJOR, DIV_QUARTER, DUE_SPI_4822_06, false);
	// NoteSequencer* s08 = NoteSequencer::create(notes3, C, MAJOR, DIV_QUARTER, DUE_SPI_4822_07, false);
	// NoteSequencer* s09 = NoteSequencer::create(notes3, C, MAJOR, DIV_QUARTER, DUE_SPI_4822_08, false);
	// NoteSequencer* s10 = NoteSequencer::create(notes3, C, MAJOR, DIV_QUARTER, DUE_SPI_4822_09, false);
	// NoteSequencer* s11 = NoteSequencer::create(notes3, C, MAJOR, DIV_QUARTER, DUE_SPI_4822_10, false);
	// NoteSequencer* s12 = NoteSequencer::create(notes3, C, MAJOR, DIV_QUARTER, DUE_SPI_4822_11, false);
	// NoteSequencer* s13 = NoteSequencer::create(notes3, C, MAJOR, DIV_QUARTER, DUE_SPI_4822_12, false);
	// NoteSequencer* s14 = NoteSequencer::create(notes3, C, MAJOR, DIV_QUARTER, DUE_SPI_4822_13, false);
	// NoteSequencer* s15 = NoteSequencer::create(notes3, C, MAJOR, DIV_QUARTER, DUE_SPI_4822_14, false);
	// NoteSequencer* s16 = NoteSequencer::create(notes3, C, MAJOR, DIV_QUARTER, DUE_SPI_4822_15, false);
	// 
	fixedclock->registerdevice(s01);
	fixedclock->registerdevice(s02);
	fixedclock->registerdevice(s03);
	fixedclock->registerdevice(s04);
	fixedclock->registerdevice(s05);
	fixedclock->registerdevice(s06);
	// fixedclock->registerdevice(s07);
	// fixedclock->registerdevice(s08);
	// fixedclock->registerdevice(s09);
	// fixedclock->registerdevice(s10);
	// fixedclock->registerdevice(s11);
	// fixedclock->registerdevice(s12);
	// fixedclock->registerdevice(s13);
	// fixedclock->registerdevice(s14);
	// fixedclock->registerdevice(s15);
	// fixedclock->registerdevice(s16);

	EventManager::registerdevice(fixedclock);
}

void loop() 
{
	EventManager::loop();

}
