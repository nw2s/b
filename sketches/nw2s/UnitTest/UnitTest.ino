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

	Clock* fixedclock = RandomDropoutClock::create(125, 8, 20);

	const int sequence_length = 34;
	SequenceNote notelist[sequence_length] = { {1,1}, {1,3}, {1,5}, {1,1}, {1,3}, {1,5}, {1,1}, {1,5}, 
												{2,1}, {2,3}, {2,5}, {2,1}, {2,3}, {2,5}, {2,1}, {2,5}, 
												{3,1}, {3,3}, {3,5}, {3,1}, {3,3}, {3,5}, {3,1}, {3,3}, 
												{2,5}, {2,1}, {2,3}, {2,1}, {2,5}, {2,1}, {2,3}, {2,1} };
	std::vector<SequenceNote>* notes = new vector<SequenceNote>(notelist, notelist + sequence_length);

	Sequencer* sequencer = MorphingNoteSequencer::create(notes, C, MAJOR, 25, DIV_SIXTEENTH, ARDCORE_DAC);
	sequencer->setslew(LinearSlew::create(50));
	sequence1->seteg(ADSR::create(10, 250, 254, 1250, 1200, false, DUE_SPI_4822_00));
	fixedclock->registerdevice(sequencer);

	EventManager::registerdevice(fixedclock);
	
	
	// /* Fixed clock running at 75BPM on an Ardcore D0 */
	// FixedClock* fixedclock = FixedClock::create(75, 16);
	// EventManager::registerdevice(fixedclock);
	// 
	// /* Gate running quarter notes on expander port 0 */
	// Trigger* trigger0 = Trigger::create(ARDCORE_OUT_EX_D0, DIV_QUARTER);
	// fixedclock->registerdevice(trigger0);
	// 
	// Trigger* trigger1 = Trigger::create(ARDCORE_OUT_EX_D1, DIV_HALF);
	// fixedclock->registerdevice(trigger1);
	// 
	// Trigger* trigger2 = Trigger::create(ARDCORE_OUT_EX_D2, DIV_EIGHTH_DOT);
	// fixedclock->registerdevice(trigger2);
	// 
	// Trigger* trigger3 = Trigger::create(ARDCORE_OUT_EX_D3, DIV_QUARTER_TRIPLET);
	// fixedclock->registerdevice(trigger3);
	// 
	// Trigger* trigger4 = Trigger::create(ARDCORE_OUT_EX_D4, DIV_SIXTEENTH);
	// fixedclock->registerdevice(trigger4);
	// 
	// Trigger* trigger5 = Trigger::create(ARDCORE_OUT_EX_D5, DIV_WHOLE);
	// fixedclock->registerdevice(trigger5);
	// 
	// Trigger* trigger6 = Trigger::create(ARDCORE_OUT_EX_D6, DIV_EIGHTH_TRIPLET);
	// fixedclock->registerdevice(trigger6);
	// 
	// Trigger* trigger7 = Trigger::create(ARDCORE_OUT_EX_D7, DIV_THIRTYSECOND);
	// fixedclock->registerdevice(trigger7);
	// 
	// /* Random clock pulses between 25 and 125 BPM on D0 */
	// //EventManager::registerdevice(RandomClock::create(25,125, ARDCORE_OUT_D0));
	// 
	// /* Fixed period clock whose period is based on an analog input */
	// //EventManager::registerdevice(VariableClock::create(25, 125, ARDCORE_IN_A2, ARDCORE_OUT_D1));
	// 	
	// /* Set up a sample sequence that we'll use in the demos */
	// const int sequence_length = 34;
	// // SequenceNote notelist[sequence_length] = { {0,1}, {4,1}, {0,1}, {4,1}, {0,1}, {4,1}, {0,1}, {4,1} };
	// // std::vector<SequenceNote>* notes = new vector<SequenceNote>(notelist, notelist + sequence_length);
	// // 
	// SequenceNote notelist2[sequence_length] = { {1,1}, {1,3}, {1,5}, {1,1}, {1,3}, {1,5}, {1,1}, {1,5}, 
	// 											{2,1}, {2,3}, {2,5}, {2,1}, {2,3}, {2,5}, {2,1}, {2,5}, 
	// 											{3,1}, {3,3}, {3,5}, {3,1}, {3,3}, {3,5}, {3,1}, {3,5}, 
	// 											{4,1}, {4,3}, {4,5}, {4,1}, {4,3}, {4,5}, {4,1}, {5,1} };
	// std::vector<SequenceNote>* notes2 = new vector<SequenceNote>(notelist2, notelist2 + sequence_length);
	// 
	// SequenceNote notelist3[6] = { {0,1}, {1,1}, {2,1}, {3,1}, {4,1}, {5,1} };
	// std::vector<SequenceNote>* notes3 = new vector<SequenceNote>(notelist3, notelist3 + 6);
	// 
	// /* Fixed note sequence running at 20BPM, output on Ardcore DAC, 100ms gate on Pin 13 */
	// //EventManager::registerdevice(NoteSequence::create(notes3, C, MAJOR, 20, ARDCORE_DAC, ARDCORE_OUT_PIN13, 100, false, NULL));
	// //EventManager::registerdevice(NoteSequence::create(notes3, C, MAJOR, 20, ARDCORE_DAC, ARDCORE_OUT_PIN13, 100, false, DecaySlew::create(1000)));
	// //EventManager::registerdevice(NoteSequence::create(notes3, C, MAJOR, 20, ARDCORE_DAC, ARDCORE_OUT_PIN13, 100, false, LinearSlew::create(1000)));
	// 
	// //EventManager::registerdevice(NoteSequence::create(notes3, C, MAJOR, 20, DUE_SPI_4822_0, DUE_OUT_D0, 200, false, NULL));
	// 
	// /* Randomized note sequence running at 160BPM, output on Ardcore DAC */
	// //EventManager::registerdevice(NoteSequence::create(notes2, A, MINOR, 40, ARDCORE_DAC, ARDCORE_OUT_D0, 150, true, NULL));
	// //EventManager::registerdevice(NoteSequence::create(notes2, A, MINOR, 40, ARDCORE_DAC, ARDCORE_OUT_D0, 150, true, DecaySlew::create(1000)));
	// //EventManager::registerdevice(NoteSequence::create(notes2, A, MINOR, 40, ARDCORE_DAC, ARDCORE_OUT_D0, 150, true, LinearSlew::create(1000)));
	// 
	// /* Randomly play notes from a given key at a fixed BPM */
	// //EventManager::registerdevice(RandomNoteSequence::create(A, MINOR, 160, ARDCORE_DAC, ARDCORE_OUT_D0, 50, NULL));
	// //EventManager::registerdevice(RandomNoteSequence::create(A, MINOR, 40, ARDCORE_DAC, ARDCORE_OUT_D0, 50, DecaySlew::create(1000)));
	// //EventManager::registerdevice(RandomNoteSequence::create(A, MINOR, 40, ARDCORE_DAC, ARDCORE_OUT_D0, 50, LinearSlew::create(1000)));
	// 
	// /* Randomly time notes from a sequence, played in order */
	// //EventManager::registerdevice(RandomTimeSequence::create(notes2, A, MINOR, 20, 240, ARDCORE_DAC, ARDCORE_OUT_D0, 100, false, NULL));
	// //EventManager::registerdevice(RandomTimeSequence::create(notes2, A, MINOR, 75, 160, ARDCORE_DAC, ARDCORE_OUT_D0, 100, false, DecaySlew::create(1000)));
	// //EventManager::registerdevice(RandomTimeSequence::create(notes2, A, MINOR, 75, 160, ARDCORE_DAC, ARDCORE_OUT_D0, 100, false, LinearSlew::create(1000)));
	// 
	// /* Randomize the notes in a sequence, and play with a random timing */
	// //EventManager::registerdevice(RandomTimeSequence::create(notes2, A, MINOR, 20, 240, ARDCORE_DAC, ARDCORE_OUT_D0, 100, true, NULL));
	// //EventManager::registerdevice(RandomTimeSequence::create(notes2, A, MINOR, 75, 160, ARDCORE_DAC, ARDCORE_OUT_D0, 100, true, DecaySlew::create(1000)));
	// //EventManager::registerdevice(RandomTimeSequence::create(notes2, A, MINOR, 75, 160, ARDCORE_DAC, ARDCORE_OUT_D0, 100, true, LinearSlew::create(1000)));
	// //EventManager::registerdevice(RandomTimeSequence::create(notes2, A, MINOR, 75, 160, DUE_SPI_4822_0, DUE_OUT_D0, 100, true, DecaySlew::create(1000)));
	// 
	// /* Move through a sequence based on a CV input */
	// //EventManager::registerdevice(CVNoteSequence::create(notes3, C, MAJOR, ARDCORE_DAC, ARDCORE_IN_A0, ARDCORE_OUT_D0, 200, NULL));
	// 
	// //CVNoteSequence* sequence1 = CVNoteSequence::create(notes2, C, MAJOR, ANALOG_OUT_NONE, ARDCORE_IN_A0);
	// // NoteSequence* sequence1 = NoteSequence::create(notes2, C, MAJOR, 20, ANALOG_OUT_NONE, false);
	// // sequence1->setgate(Gate::create(ARDCORE_OUT_D0, 200));
	// // sequence1->setslew(LinearSlew::create(1000));
	// // sequence1->seteg(ADSR::create(100, 250, 254, 1250, 1200, false, ARDCORE_DAC));
	// // EventManager::registerdevice(sequence1);
	// 
	// 
	// 
	// //EventManager::registerdevice(CVNoteSequence::create(notes3, C, MAJOR, 20, DUE_SPI_4822_0, DUE_OUT_D0, 200, false, NULL));
	


	
}

void loop() 
{
	EventManager::loop();
	
}
