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

/*


*/

using namespace nw2s;

void setup() 
{
	Serial.begin(19200);
	Serial.print("\n\nStarting...\n");

	EventManager::initialize();
	
	Clock* fixedclock = FixedClock::create(120, 16);
	fixedclock->setSwing(DIV_EIGHTH, 100);
	
	SequenceNote notelist[4] = { {2, 1}, {2, 5}, {2, 1}, {1, 4} };
	
	NoteSequenceData* notes = new NoteSequenceData(notelist, notelist + 4);
	
	Sequencer* sequencer = NoteSequencer::create(notes, C, MAJOR, DIV_SIXTEENTH, DUE_SPI_4822_00);
	
	//fixedclock->registerdevice(sequencer);

	
	/* Trigger Sequence */
	int snarelist[4] = { 0, 1, 0, 1 };
	int kicklist[4] = { 1, 0, 1, 0};
	int hatlist[8] = { 1, 1, 1, 1, 1, 1, 1, 1 };

	TriggerSequenceData* snaretriggers = new TriggerSequenceData(snarelist, snarelist + 4);
	TriggerSequenceData* kicktriggers = new TriggerSequenceData(kicklist, snarelist + 4);
	TriggerSequenceData* hattriggers = new TriggerSequenceData(hatlist, snarelist + 8);

	Sequencer* snaresequencer = TriggerSequencer::create(snaretriggers, DIV_QUARTER, DUE_OUT_D00);
	Sequencer* kicksequencer = TriggerSequencer::create(kicktriggers, DIV_QUARTER, DUE_OUT_D01);
	Sequencer* hatsequencer = TriggerSequencer::create(hattriggers, DIV_EIGHTH, DUE_OUT_D02);

	fixedclock->registerdevice(snaresequencer);
	fixedclock->registerdevice(kicksequencer);
	fixedclock->registerdevice(hatsequencer);
	
	EventManager::registerdevice(fixedclock);
}

void loop() 
{
	EventManager::loop();
	
}
