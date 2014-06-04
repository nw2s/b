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

	/* We want a Turing Machine for some modulation signals and for a little rhythm */
	RandomLoopingShiftRegister* shiftregister = RandomLoopingShiftRegister::create(16, DUE_IN_A01, DIV_SIXTEENTH);

	/* We're only using the outs we are interested in */
	
	/* CV out and delayed CV will be filter cutoff modulators */
	shiftregister->setCVOut(DUE_SPI_4822_00);
	shiftregister->setDelayedCVOut(DUE_SPI_4822_01, 6);

	/* Trigger 1 will be for kick */
	shiftregister->setTriggerOut(1, DUE_OUT_D00);

	/* Logical OR will be for hat */
	shiftregister->setLogicalOrTrigger(DUE_OUT_D01, 1, 2, 4, 8);

	/* We only want the snare to hit rarely */
	shiftregister->setLogicalAndTrigger(DUE_OUT_D02, 1, 2, 4, 8);

	/* And I want a gate to have for something */
	shiftregister->setLogicalAndGate(DUE_OUT_D03, 500, 5, 8);
		
	vclock->registerDevice(shiftregister);


	/* Now set up a CV note sequenver */

	/* Set up the note data for the sequence */
	SequenceNote notelist[34] = { {1,1}, {1,3}, {1,5}, {1,1}, {1,3}, {1,5}, {1,1}, {1,5}, 
								  {2,1}, {2,3}, {2,5}, {2,1}, {2,3}, {2,5}, {2,1}, {2,5}, 
								  {3,1}, {3,3}, {3,5}, {3,1}, {3,3}, {3,5}, {3,1}, {3,5}, 
								  {4,1}, {4,3}, {4,5}, {4,1}, {4,3}, {4,5}, {4,1}, {5,1} };
								
	/* Add the raw array to a vector for easier transport */
	NoteSequenceData* notes = new NoteSequenceData(notelist, notelist + 34);

	/* Build our note-based seuqnce */
	Sequencer* sequencer = CVNoteSequencer::create(notes, C, MAJOR, DUE_SPI_4822_02, DUE_IN_A02);

	/* Add some modifier values */
	// sequencer->setgate(Gate::create(DUE_OUT_D04, 75));
	// sequencer->setslew(DecaySlew::create(0.90));
	// sequencer->seteg(ADSR::create(20, 40, 254, 1250, 1200, false, DUE_SPI_4822_03));


	EventManager::registerDevice(sequencer);
	EventManager::registerDevice(vclock);
}

void loop() 
{
	EventManager::loop();
}

