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

using namespace nw2s;


void setup() 
{
	Serial.begin(19200);
	Serial.print("\n\nStarting...\n");

	EventManager::initialize();

	/* Setup a variable clock */
	Clock* vclock = VariableClock::create(10, 240, DUE_IN_A00, 16);

	/* Trigger Sequence */
	int snarelist[16] = { 0, 1, 1, 1, 75, 1, 1, 1, 0, 1, 1, 1, 75, 1, 1, 1 };
	int kicklist[16] = { 90, 1, 1, 1, 0, 1, 10, 1, 75, 1, 1, 1, 0, 1, 15, 1 };
	int hatlist[16] = { 85, 25, 85, 25, 85, 25, 85, 25, 85, 25, 85, 25, 85, 25, 85, 25 };

	TriggerSequenceData* snaretriggers = new TriggerSequenceData(snarelist, snarelist + 16);
	TriggerSequenceData* kicktriggers = new TriggerSequenceData(kicklist, kicklist + 16);
	TriggerSequenceData* hattriggers = new TriggerSequenceData(hatlist, hatlist + 16);
	
	ProbabilityTriggerSequencer* snaresequencer = ProbabilityTriggerSequencer::create(snaretriggers, DIV_SIXTEENTH, DUE_OUT_D00);
	ProbabilityTriggerSequencer* kicksequencer = ProbabilityTriggerSequencer::create(kicktriggers, DIV_SIXTEENTH, DUE_OUT_D01);
	ProbabilityTriggerSequencer* hatsequencer = ProbabilityTriggerSequencer::create(hattriggers, DIV_SIXTEENTH, DUE_OUT_D02);

	snaresequencer->setProbabilityModifier(DUE_IN_A01);
	kicksequencer->setProbabilityModifier(DUE_IN_A02);
	hatsequencer->setProbabilityModifier(DUE_IN_A03);	
	
	vclock->registerDevice(snaresequencer);
	vclock->registerDevice(kicksequencer);
	vclock->registerDevice(hatsequencer);
	
	EventManager::registerDevice(vclock);
}

void loop() 
{
	EventManager::loop();
	
}
