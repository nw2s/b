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
#include <SignalData.h>
#include <SD.h>
#include <Loop.h>
#include <Wire.h>
#include <aJSON.h>

/*

	Yet another demo that's getting a little more involved
	
		1. Four simultaneous outputs
		2. CV sequencer
		3. Entropy driven devices
		4. Multiple clocks running at different tempos
		5. Free running sample based loop
		6. Clocked sample based loop
		
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

	Clock* clock1 = FixedClock::create(128, 16);
	
	SequenceNote notelist[34] = { {1,1}, {1,1}, {1,5}, {1,1}, {1,1}, {1,3}, {1,3}, {1,5}, 
								  {2,1}, {2,1}, {2,5}, {2,1}, {2,3}, {2,3}, {2,1}, {2,1}, 
								  {1,1}, {3,1}, {3,1}, {3,5}, {3,1}, {3,1}, {3,3}, {3,1}, 
								  {2,5}, {2,5}, {2,1}, {2,3}, {2,3}, {2,1}, {2,1}, {2,3} };
								
	std::vector<SequenceNote>* notes = new vector<SequenceNote>(notelist, notelist + 34);
	
	Sequencer* sequencer = MorphingNoteSequencer::create(notes, A, MINOR, 25, DIV_SIXTEENTH, DUE_SPI_4822_00);
	sequencer->seteg(ADSR::create(10, 250, 254, 1250, 1200, false, DUE_SPI_4822_01));
	
	clock1->registerDevice(sequencer);
	
	EventManager::registerDevice(clock1);
	
	
	/* VOICE 2 */
	/* VOICE 2 */
	/* VOICE 2 */
	
	Clock* randomclock = RandomTempoClock::create(40, 100, 8);
	
	SequenceNote notelist2[4] = { {0, 1}, {1, 1}, {0, 1}, {1, 1} };
	
	std::vector<SequenceNote>* notes2 = new vector<SequenceNote>(notelist2, notelist2 + 4);
	
	Sequencer* sequencer2 = NoteSequencer::create(notes2, A, MINOR, DIV_WHOLE, DUE_SPI_4822_02);
	sequencer2->setslew(LinearSlew::create(50));
	
	Sequencer* cvsequencer = CVSequencer::create(800, 1000, DIV_QUARTER, DUE_SPI_4822_03);
	cvsequencer->setslew(LinearSlew::create(50));
	
	randomclock->registerDevice(cvsequencer);
	
	EventManager::registerDevice(randomclock);
	
	
	/* MECHANICAL NOISE */
	/* MECHANICAL NOISE */
	/* MECHANICAL NOISE */
	
	/* Setup the mechanical noise as a free running loop */
	SignalData* mechanicalnoise = SignalData::fromSDFile("loops/mech01.raw");
	Looper* looper2 = Looper::create(DUE_DAC0, mechanicalnoise);

	/* DRUM LOOP */
	/* DRUM LOOP */
	/* DRUM LOOP */

	/* Set up the drum loop as a clocked loop. It will reset every two beats */
	FixedClock* loopclock = FixedClock::create(128, 16);
	
	SignalData* drumloop = SignalData::fromSDFile("loops/loop01.raw");
	ClockedLooper* looper1 = ClockedLooper::create(DUE_DAC1, drumloop, 2, DIV_QUARTER);
	
	loopclock->registerDevice(looper1);
	
	EventManager::registerDevice(loopclock);
	
	
}

void loop() 
{
	EventManager::loop();
	
}
