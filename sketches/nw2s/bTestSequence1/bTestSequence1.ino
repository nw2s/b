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


using namespace nw2s;

void setup() 
{
	Serial.begin(19200);
	Serial.print("\n\nStarting...\n");

	EventManager::initialize();

	FixedClock* fixedclock = FixedClock::create(5, 6);

	SequenceNote notelist3[6] = { {0,1}, {1,1}, {2,1}, {3,1}, {4,1}, {5,1} };
	std::vector<SequenceNote>* notes3 = new vector<SequenceNote>(notelist3, notelist3 + 6);

	NoteSequence* s1 = NoteSequence::create(notes3, C, MAJOR, DIV_QUARTER, DUE_SPI_4822_00, false);
	NoteSequence* s2 = NoteSequence::create(notes3, C, MAJOR, DIV_QUARTER, DUE_SPI_4822_00, false);
	NoteSequence* s3 = NoteSequence::create(notes3, C, MAJOR, DIV_QUARTER, DUE_SPI_4822_00, false);
	NoteSequence* s4 = NoteSequence::create(notes3, C, MAJOR, DIV_QUARTER, DUE_SPI_4822_00, false);

	s1->setgate(Gate::create(DUE_OUT_D00, 75));
	s2->setgate(Gate::create(DUE_OUT_D01, 100));
	s3->setgate(Gate::create(DUE_OUT_D02, 125));
	s4->setgate(Gate::create(DUE_OUT_D03, 150));

	fixedclock->registerdevice(s1);
	fixedclock->registerdevice(s2);
	fixedclock->registerdevice(s3);
	fixedclock->registerdevice(s4);

	EventManager::registerdevice(fixedclock);
}

void loop() 
{
	EventManager::loop();

}
