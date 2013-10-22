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

#include <EventManager.h>
#include <Trigger.h>
#include <Clock.h>
#include <Sequence.h>
#include <IO.h>
#include <SPI.h>

using namespace nw2s;

void setup() 
{

	Serial.begin(19200);
	Serial.print("Starting...\n");

	EventManager::initialize();

	/* Fixed clock running at 75BPM on an Ardcore D0 */
	FixedClock* fixedclock = FixedClock::create(75, 8);
	EventManager::registerdevice(fixedclock);

	/* Set up the note data for the sequence */
	SequenceNote notelist[34] = { {1,1}, {1,3}, {1,5}, {1,1}, {1,3}, {1,5}, {1,1}, {1,5}, 
								  {2,1}, {2,3}, {2,5}, {2,1}, {2,3}, {2,5}, {2,1}, {2,5}, 
								  {3,1}, {3,3}, {3,5}, {3,1}, {3,3}, {3,5}, {3,1}, {3,5}, 
								  {4,1}, {4,3}, {4,5}, {4,1}, {4,3}, {4,5}, {4,1}, {5,1} };
								
	/* Add the raw array to a vector for easier transport */
	NoteSequenceData* notes = new NoteSequenceData(notelist, notelist + 34);

	/* Build our note-based seuqnce */
	NoteSequence* sequence = NoteSequence::create(notes, C, MAJOR, DIV_EIGHTH, DUE_SPI_4822_00);

	/* Add some modifier values */
	sequence->setgate(Gate::create(DUE_OUT_D0, 75));
	sequence->setslew(DecaySlew::create(0.90));
	sequence->seteg(ADSR::create(20, 40, 254, 1250, 1200, false, DUE_SPI_4822_03));

	fixedclock->registerdevice(sequence);
}

void loop() 
{
	EventManager::loop();	
}
