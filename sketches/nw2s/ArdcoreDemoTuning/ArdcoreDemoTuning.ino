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


/* 

	This is a simple demonstration of the CVNoteSequence. It uses the Ardcore's
	potentiometer input to iterate over a set of octaves ues to test the output
	voltages. You should be able to measure the voltages as 1V, 2V, 3V, 4V, and
	5V at each octave. 

*/


using namespace nw2s;

void setup() 
{
	Serial.begin(19200);
	Serial.print("\n\nStarting...\n");

	EventManager::initialize();

	SequenceNote notelist[6] = { {0,1}, {1,1}, {2,1}, {3,1}, {4,1}, {5,1} };
	std::vector<SequenceNote>* notes = new vector<SequenceNote>(notelist, notelist + 6);

	EventManager::registerdevice(CVNoteSequencer::create(notes, C, MAJOR, ARDCORE_DAC, ARDCORE_IN_A0));
}

void loop() 
{
	EventManager::loop();
	
}

