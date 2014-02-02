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
#include <Wire.h>

/*


*/

using namespace nw2s;

void setup() 
{
	Serial.begin(19200);
	Serial.print("\n\nStarting...\n");

	EventManager::initialize();

	Clock* vclock = VariableClock::create(20, 240, DUE_IN_A00, 16);

	/* Trigger Sequence */
	int snaretriggers[16] =   { 0, 0, 1,  1,  75,  1,  1,  0, 0, 1,  20,  1,  75,  1, 1, 15 };
	int snarevelocities[16] = { 0, 0, 75, 75, 400, 75, 75, 0, 0, 75, 200, 75, 400, 1, 1, 75 };
	
	int kicklist[16] =        { 90,  1,  1,  5,  0, 1,  10,  1,  75,  1,  1,  1,  0, 1,  15,  1 };
	int kickvelocities[16] =  { 500, 75, 75, 75, 0, 75, 200, 75, 400, 75, 75, 75, 0, 75, 150, 75 };
	
	int hatlist[16] =         { 85,  25,  85,  25,  85,  25,  85,  50,  25,  85,  25,  85,  25,  85,  25,  85 };
	int hatvelocities[16] =   { 100, 250, 100, 250, 100, 250, 100, 200, 250, 100, 250, 100, 250, 100, 200, 100 };
	
	int crashlist[16] =         { 10,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  10,  0,  10,  0,  0 };
	int crashvelocities[16] =   { 100, 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  100, 0,  100, 0,  0 };
	
	TriggerSequenceData* snaretriggerdata = new TriggerSequenceData(snaretriggers, snaretriggers + 16);
	TriggerSequenceData* snarevelocitydata = new TriggerSequenceData(snarevelocities, snarevelocities + 16);
	TriggerSequenceData* kicktriggerdata = new TriggerSequenceData(kicklist, kicklist + 16);
	TriggerSequenceData* kickvelocitydata = new TriggerSequenceData(kickvelocities, kickvelocities + 16);
	TriggerSequenceData* hattriggerdata = new TriggerSequenceData(hatlist, hatlist + 16);
	TriggerSequenceData* hatvelocitydata = new TriggerSequenceData(hatvelocities, hatvelocities + 16);
	TriggerSequenceData* crashtriggerdata = new TriggerSequenceData(crashlist, crashlist + 16);
	TriggerSequenceData* crashvelocitydata = new TriggerSequenceData(crashvelocities, crashvelocities + 16);
	
	ProbabilityDrumTriggerSequencer* snaresequencer = ProbabilityDrumTriggerSequencer::create(snaretriggerdata, snarevelocitydata, 25, DIV_SIXTEENTH, DUE_SPI_4822_00);
	ProbabilityDrumTriggerSequencer* kicksequencer = ProbabilityDrumTriggerSequencer::create(kicktriggerdata, kickvelocitydata, 10, DIV_SIXTEENTH, DUE_SPI_4822_01);
	ProbabilityDrumTriggerSequencer* hatsequencer = ProbabilityDrumTriggerSequencer::create(hattriggerdata, hatvelocitydata, 25, DIV_SIXTEENTH, DUE_SPI_4822_02);
	ProbabilityDrumTriggerSequencer* crashsequencer = ProbabilityDrumTriggerSequencer::create(crashtriggerdata, crashvelocitydata, 100, DIV_SIXTEENTH, DUE_SPI_4822_03);
	
	snaresequencer->setProbabilityModifier(DUE_IN_A01);
	kicksequencer->setProbabilityModifier(DUE_IN_A02);
	hatsequencer->setProbabilityModifier(DUE_IN_A03);	
	crashsequencer->setProbabilityModifier(DUE_IN_A04);	
	
	vclock->registerDevice(snaresequencer);
	vclock->registerDevice(kicksequencer);
	vclock->registerDevice(hatsequencer);
	vclock->registerDevice(crashsequencer);

	EventManager::registerDevice(vclock);
}

void loop() 
{
	EventManager::loop();
	
}


