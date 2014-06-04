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
#include <Wire.h>
#include <aJSON.h>
#include <SD.h>

/*

	This is a second sequence demo which is in place to demonstrate the following:
	
		1. Four simultaneous outputs
		2. CV sequencer
		3. Entropy driven devices
		4. Multiple clocks running at different tempos
		
	It's envisioned as a two voice sketch, but that's just the way I hooked it up.
	
	Voice 1 is driven by a slowly morphing sequencer that periodically swaps 
	positions of notes in the sequence. Pitch CV is on A00. Envelope CV is on
	A01. 
	
	Voice 2 is a standard fixed sequence playing a slower rate. Pitch CV is on A02.
	A CV Sequence is set up to modulate an aspect of the voice at an unrelated 
	tempo.

*/

using namespace nw2s;

void setup() 
{
	Serial.begin(19200);
	Serial.print("\n\nStarting...\n");

	EventManager::initialize();

	/* VOICE 1 */
	/* VOICE 1 */
	/* VOICE 1 */

	Clock* clock1 = FixedClock::create(125, 8);
	
	SequenceNote notelist[34] = { {1,1}, {1,1}, {1,5}, {1,1}, {1,1}, {1,3}, {1,3}, {1,5}, 
								  {2,1}, {2,1}, {2,5}, {2,1}, {2,3}, {2,3}, {2,1}, {2,1}, 
								  {1,1}, {3,1}, {3,1}, {3,5}, {3,1}, {3,1}, {3,3}, {3,1}, 
								  {2,5}, {2,5}, {2,1}, {2,3}, {2,3}, {2,1}, {2,1}, {2,3} };
								
	std::vector<SequenceNote>* notes = new vector<SequenceNote>(notelist, notelist + 34);
	
	Sequencer* sequencer = MorphingNoteSequencer::create(notes, C, MAJOR, 25, DIV_SIXTEENTH, DUE_SPI_4822_00);
	sequencer->seteg(ADSR::create(10, 250, 254, 1250, 1200, false, DUE_SPI_4822_01));
	
	clock1->registerDevice(sequencer);
	
	EventManager::registerDevice(clock1);
	
	
	/* VOICE 2 */
	/* VOICE 2 */
	/* VOICE 2 */
	
	Clock* fixedclock = FixedClock::create(50, 8);
	
	SequenceNote notelist2[4] = { {0, 1}, {0, 5}, {0, 1}, {0, 4} };
	
	std::vector<SequenceNote>* notes2 = new vector<SequenceNote>(notelist, notelist + 4);
	
	Sequencer* sequencer2 = NoteSequencer::create(notes2, C, MAJOR, DIV_WHOLE, DUE_SPI_4822_02);
	
	fixedclock->registerDevice(sequencer2);
	
	EventManager::registerDevice(fixedclock);
	
	/* VOICE 2 MODULATION SEQUENCE */
	/* VOICE 2 MODULATION SEQUENCE */
	/* VOICE 2 MODULATION SEQUENCE */
	
	Clock* randomclock = RandomTempoClock::create(10, 100, 8);
	
	Sequencer* cvsequencer = CVSequencer::create(800, 1000, DIV_QUARTER, DUE_SPI_4822_03);
	cvsequencer->setslew(LinearSlew::create(50));
	
	randomclock->registerDevice(cvsequencer);
	
	EventManager::registerDevice(randomclock);
}

void loop() 
{
	EventManager::loop();
	
}
