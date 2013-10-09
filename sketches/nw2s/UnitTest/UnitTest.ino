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

	/* Fixed clock running at 75BPM on an Ardcore D0 */
	//EventManager::registerdevice(FixedClock::create(75, ARDCORE_OUT_D0));

	/* Random clock pulses between 25 and 125 BPM on D0 */
	//EventManager::registerdevice(RandomClock::create(25,125, ARDCORE_OUT_D0));

	/* Fixed period clock whose period is based on an analog input */
	//EventManager::registerdevice(VariableClock::create(25, 125, ARDCORE_IN_A2, ARDCORE_OUT_D1));
		
	/* Set up a sample sequence that we'll use in the demos */
	const int sequence_length = 8;
	SequenceNote notelist[sequence_length] = { {0,1}, {4,1}, {0,1}, {4,1}, {0,1}, {4,1}, {0,1}, {4,1} };
	std::vector<SequenceNote>* notes = new vector<SequenceNote>(notelist, notelist + sequence_length);

	SequenceNote notelist2[sequence_length] = { {1,1}, {1,3}, {1,5}, {1,1}, {1,3}, {1,5}, {1,1}, {1,5} };
	std::vector<SequenceNote>* notes2 = new vector<SequenceNote>(notelist2, notelist2 + sequence_length);

	/* Fixed note sequence running at 20BPM 16th notes, output on Ardcore DAC, 100ms gate on Pin 13 */
	//EventManager::registerdevice(NoteSequence::create(notes, A, MINOR, 20, ARDCORE_DAC, ARDCORE_OUT_PIN13, 100, false, NULL));
	//EventManager::registerdevice(NoteSequence::create(notes, A, MINOR, 20, ARDCORE_DAC, ARDCORE_OUT_PIN13, 100, false, DecaySlew::create(1000)));
	//EventManager::registerdevice(NoteSequence::create(notes, A, MINOR, 20, ARDCORE_DAC, ARDCORE_OUT_PIN13, 100, false, LinearSlew::create(1000)));

	/* Randomized note sequence running at 160BPM, output on Ardcore DAC */
	//EventManager::registerdevice(NoteSequence::create(notes2, A, MINOR, 40, ARDCORE_DAC, ARDCORE_OUT_D0, 150, true, NULL));
	//EventManager::registerdevice(NoteSequence::create(notes2, A, MINOR, 40, ARDCORE_DAC, ARDCORE_OUT_D0, 150, true, DecaySlew::create(1000)));
	//EventManager::registerdevice(NoteSequence::create(notes2, A, MINOR, 40, ARDCORE_DAC, ARDCORE_OUT_D0, 150, true, LinearSlew::create(1000)));

	/* Randomly play notes from a given key at a fixed BPM */
	//EventManager::registerdevice(RandomNoteSequence::create(A, MINOR, 160, ARDCORE_DAC, ARDCORE_OUT_D0, 50, NULL));
	//EventManager::registerdevice(RandomNoteSequence::create(A, MINOR, 40, ARDCORE_DAC, ARDCORE_OUT_D0, 50, DecaySlew::create(1000)));
	//EventManager::registerdevice(RandomNoteSequence::create(A, MINOR, 40, ARDCORE_DAC, ARDCORE_OUT_D0, 50, LinearSlew::create(1000)));

	/* Randomly time notes from a sequence, played in order */
	//EventManager::registerdevice(RandomTimeSequence::create(notes2, A, MINOR, 20, 240, ARDCORE_DAC, ARDCORE_OUT_D0, 100, false, NULL));
	//EventManager::registerdevice(RandomTimeSequence::create(notes2, A, MINOR, 75, 160, ARDCORE_DAC, ARDCORE_OUT_D0, 100, false, DecaySlew::create(1000)));
	//EventManager::registerdevice(RandomTimeSequence::create(notes2, A, MINOR, 75, 160, ARDCORE_DAC, ARDCORE_OUT_D0, 100, false, LinearSlew::create(1000)));

	/* Randomize the notes in a sequence, and play with a random timing */
	//EventManager::registerdevice(RandomTimeSequence::create(notes2, A, MINOR, 20, 240, ARDCORE_DAC, ARDCORE_OUT_D0, 100, true, NULL));
	//EventManager::registerdevice(RandomTimeSequence::create(notes2, A, MINOR, 75, 160, ARDCORE_DAC, ARDCORE_OUT_D0, 100, true, DecaySlew::create(1000)));
	//EventManager::registerdevice(RandomTimeSequence::create(notes2, A, MINOR, 75, 160, ARDCORE_DAC, ARDCORE_OUT_D0, 100, true, LinearSlew::create(1000)));
	
}

void loop() 
{
	EventManager::loop();
	
	// Key key1 = Key(MAJOR, C);		
	// ScaleNote note1 = key1.getNote(0, 2);
	// ScaleNote note2 = key1.getNote(-1, 2);
	// ScaleNote note3 = key1.getRandomNote();
}
