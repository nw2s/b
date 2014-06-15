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
	
	// Clock* fixedclock = FixedClock::create(120, 16);
	// fixedclock->setSwing(DIV_EIGHTH, 15);
	// 
	Clock* fixedclock2 = FixedClock::create(120, 16);
	fixedclock2->setSwing(DIV_EIGHTH, 10);

	Clock* fixedclock3 = FixedClock::create(120, 16);

	SequenceNote notelist[4] = { {2, 1}, {2, 5}, {2, 1}, {1, 4} };
	
	NoteSequenceData* notes = new NoteSequenceData(notelist, notelist + 4);
	
	Sequencer* sequencer = NoteSequencer::create(notes, C, Key::SCALE_MAJOR, DIV_SIXTEENTH, DUE_SPI_4822_00);
	
	//fixedclock->registerdevice(sequencer);

	
	/* Trigger Sequence */
	int snarelist[4] = { 0, 1, 0, 1 };
	int kicklist[4] = { 1, 0, 1, 0};
	int hatlist[8] = { 1, 1, 1, 1, 1, 1, 1, 1 };
	int hatlist2[16] = { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 };

	TriggerSequenceData* snaretriggers = new TriggerSequenceData(snarelist, snarelist + 4);
	TriggerSequenceData* kicktriggers = new TriggerSequenceData(kicklist, kicklist + 4);
	TriggerSequenceData* hattriggers = new TriggerSequenceData(hatlist, hatlist + 8);
	TriggerSequenceData* hattriggers2 = new TriggerSequenceData(hatlist2, hatlist2 + 16);

	TriggerSequenceData* hattriggers3 = new TriggerSequenceData(hatlist, hatlist + 8);
	TriggerSequenceData* hattriggers4 = new TriggerSequenceData(hatlist2, hatlist2 + 16);

	// Sequencer* snaresequencer = TriggerSequencer::create(snaretriggers, DIV_QUARTER, DUE_OUT_D00);
	// Sequencer* kicksequencer = TriggerSequencer::create(kicktriggers, DIV_QUARTER, DUE_OUT_D01);
	// Sequencer* hatsequencer = TriggerSequencer::create(hattriggers, DIV_EIGHTH, DUE_OUT_D02);
	// Sequencer* hatsequencer2 = TriggerSequencer::create(hattriggers2, DIV_SIXTEENTH, DUE_OUT_D03);
	// 
	Sequencer* hatsequencer3 = TriggerSequencer::create(hattriggers3, DIV_EIGHTH, DUE_OUT_D04);
	// Sequencer* hatsequencer4 = TriggerSequencer::create(hattriggers4, DIV_SIXTEENTH, DUE_OUT_D05);
	
	Sequencer* hatsequencer5 = TriggerSequencer::create(hattriggers, DIV_EIGHTH, DUE_OUT_D06);
	// Sequencer* hatsequencer6 = TriggerSequencer::create(hattriggers2, DIV_SIXTEENTH, DUE_OUT_D07);

	// fixedclock->registerdevice(snaresequencer);
	// fixedclock->registerdevice(kicksequencer);
	// fixedclock->registerdevice(hatsequencer);
	// fixedclock->registerdevice(hatsequencer2);
	fixedclock2->registerdevice(hatsequencer3);
	// fixedclock2->registerdevice(hatsequencer4);
	fixedclock3->registerdevice(hatsequencer5);
	// fixedclock3->registerdevice(hatsequencer6);
	
	// EventManager::registerdevice(fixedclock);
	EventManager::registerdevice(fixedclock2);
	EventManager::registerdevice(fixedclock3);
}

void loop() 
{
	EventManager::loop();
	
}
