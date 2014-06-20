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

#include "Key.h"
#include "IO.h"
#include <iterator>
#include <vector>
#include "EventManager.h"
#include "Sequence.h"
#include <math.h>
#include "Entropy.h"
#include "../aJSON/aJSON.h"
#include "JSONUtil.h"


using namespace std;
using namespace nw2s;


NoteSequenceData* nw2s::noteSequenceFromJSON(aJsonObject* data)
{
	/* Allocate enough space to store our vector or notes */
	NoteSequenceData* notes = new NoteSequenceData(aJson.getArraySize(data));
	
	/* Iterate over the set of notes in the sequence */
	for (int i = 0; i < aJson.getArraySize(data); i++)
	{
		aJsonObject* noteNode = aJson.getArrayItem(data, i);
		
		if (aJson.getArraySize(noteNode) != 2)
		{
			static const char nodeError[] = "Malformed note sequence. Should be of format [ [0,1], [1,1], [2,1] ]. Returning as much as I've parsed so far.";
			Serial.println(nodeError);
			return(notes);
		}
		
		aJsonObject* octaveNode = aJson.getArrayItem(noteNode, 0);
		aJsonObject* degreeNode = aJson.getArrayItem(noteNode, 1);
		
		SequenceNote note = { octaveNode->valueint, degreeNode->valueint };
		
		(*notes)[i] = note;
	}
				
	return notes;
}

TriggerSequencer* TriggerSequencer::create(vector<int>* triggers, int clockdivision, PinDigitalOut output)
{
	return new TriggerSequencer(triggers, clockdivision, output);
}

TriggerSequencer* TriggerSequencer::create(aJsonObject* data)
{
	static const char triggersNodeName[] = "triggers";
	static const char gateNodeName[] = "triggerOutput";
	
	TriggerSequenceData* triggers = getIntCollectionFromJSON(data, triggersNodeName);
	int clockdivision = getDivisionFromJSON(data);
	PinDigitalOut triggerPin = getDigitalOutputFromJSON(data, gateNodeName);
	
	TriggerSequencer* seq = new TriggerSequencer(triggers, clockdivision, triggerPin);
			
	return seq;
}


DrumTriggerSequencer* DrumTriggerSequencer::create(std::vector<int>* triggers, int clockdivision, PinAnalogOut output)
{	
	return new DrumTriggerSequencer(triggers, clockdivision, output);
}

DrumTriggerSequencer* DrumTriggerSequencer::create(aJsonObject* data)
{
	static const char triggersNodeName[] = "triggers";
	
	TriggerSequenceData* triggers = getIntCollectionFromJSON(data, triggersNodeName);
	int clockdivision = getDivisionFromJSON(data);
	PinAnalogOut triggerPin = getAnalogOutputFromJSON(data);
	
	DrumTriggerSequencer* seq = new DrumTriggerSequencer(triggers, clockdivision, triggerPin);
			
	return seq;
}



ProbabilityTriggerSequencer* ProbabilityTriggerSequencer::create(TriggerSequenceData* triggers, int clockdivision, PinDigitalOut output)
{
	return new ProbabilityTriggerSequencer(triggers, clockdivision, output);
}

ProbabilityTriggerSequencer* ProbabilityTriggerSequencer::create(aJsonObject* data)
{
	static const char triggersNodeName[] = "triggers";
	static const char outputNodeName[] = "digitalOutput";
	
	TriggerSequenceData* triggers = getIntCollectionFromJSON(data, triggersNodeName);
	int clockdivision = getDivisionFromJSON(data);
	PinDigitalOut triggerPin = getDigitalOutputFromJSON(data, outputNodeName);
	PinAnalogIn probabilityPin = getAnalogInputFromJSON(data);
	
	ProbabilityTriggerSequencer* seq = new ProbabilityTriggerSequencer(triggers, clockdivision, triggerPin);
			
	if (probabilityPin != DUE_IN_A_NONE)
	{
		seq->setProbabilityModifier(probabilityPin);
	}		
	
	return seq;
}

ProbabilityDrumTriggerSequencer* ProbabilityDrumTriggerSequencer::create(TriggerSequenceData* triggers, TriggerSequenceData* velocities, int velocityrange, int clockdivision, PinAnalogOut output)
{
	return new ProbabilityDrumTriggerSequencer(triggers, velocities, velocityrange, clockdivision, output);
}

ProbabilityDrumTriggerSequencer* ProbabilityDrumTriggerSequencer::create(aJsonObject* data)
{
	static const char probabilityNodeName[] = "probabilityModifier";
	static const char triggersNodeName[] = "triggers";
	static const char velocitiesNodeName[] = "velocities";
	static const char velocityRangeNodeName[] = "velocityRange";
	
	TriggerSequenceData* triggers = getIntCollectionFromJSON(data, triggersNodeName);
	TriggerSequenceData* velocities = getIntCollectionFromJSON(data, velocitiesNodeName);
	int clockdivision = getDivisionFromJSON(data);
	int velocityRange = getIntFromJSON(data, velocityRangeNodeName, 0, 1, 5000);
	PinAnalogOut triggerPin = getAnalogOutputFromJSON(data);
	PinAnalogIn probabilityPin = getAnalogInputFromJSON(data, probabilityNodeName);
	
	ProbabilityDrumTriggerSequencer* seq = new ProbabilityDrumTriggerSequencer(triggers, velocities, velocityRange, clockdivision, triggerPin);
			
	if (probabilityPin != DUE_IN_A_NONE)
	{
		seq->setProbabilityModifier(probabilityPin);
	}		

	return seq;
}


NoteSequencer* NoteSequencer::create(vector<SequenceNote>* notes, NoteName key, Scale scale, int clockdivision, PinAnalogOut output, bool randomize_seq)
{
	return new NoteSequencer(notes, key, scale, clockdivision, output, randomize_seq);
}

NoteSequencer* NoteSequencer::create(aJsonObject* data)
{
	static const char randomizeNodeName[] = "randomize";
	static const char gateNodeName[] = "gateOutput";
	static const char durationNodeName[] = "gateLength";
	
	bool randomize = getBoolFromJSON(data, randomizeNodeName, false);	
	NoteSequenceData* notes = getNotesFromJSON(data);
	Scale scale = getScaleFromJSON(data);
	NoteName root = getRootFromJSON(data);
	int clockdivision = getDivisionFromJSON(data);
	PinAnalogOut output = getAnalogOutputFromJSON(data);
	PinDigitalOut gatePin = getDigitalOutputFromJSON(data, gateNodeName);
	int gateDuration = getIntFromJSON(data, durationNodeName, 20, 1, 1000);
	
	NoteSequencer* seq = new NoteSequencer(notes, root, scale, clockdivision, output, randomize);
		
	if (gatePin != DIGITAL_OUT_NONE) seq->setgate(Gate::create(gatePin, gateDuration));
	
	return seq;
}

MorphingNoteSequencer* MorphingNoteSequencer::create(vector<SequenceNote>* notes, NoteName key, Scale scale, int chaos, int clockdivision, PinAnalogOut output, PinDigitalIn resetPin)
{
	return new MorphingNoteSequencer(notes, key, scale, chaos, clockdivision, output, resetPin);
}

MorphingNoteSequencer* MorphingNoteSequencer::create(aJsonObject* data)
{
	static const char resetNodeName[] = "reset";
	static const char chaosNodeName[] = "chaos";
	static const char gateNodeName[] = "gateOutput";
	static const char durationNodeName[] = "gateLength";
	
	NoteSequenceData* notes = getNotesFromJSON(data);
	Scale scale = getScaleFromJSON(data);
	NoteName root = getRootFromJSON(data);
	int clockdivision = getDivisionFromJSON(data);
	PinAnalogOut output = getAnalogOutputFromJSON(data);
	PinDigitalOut gatePin = getDigitalOutputFromJSON(data, gateNodeName);
	PinDigitalIn reset = getDigitalInputFromJSON(data, resetNodeName);
	int gateDuration = getIntFromJSON(data, durationNodeName, 20, 1, 1000);
	int chaos = getIntFromJSON(data, chaosNodeName, 20, 1, 100);
	
	MorphingNoteSequencer* seq = new MorphingNoteSequencer(notes, root, scale, chaos, clockdivision, output, reset);
		
	if (gatePin != DIGITAL_OUT_NONE) seq->setgate(Gate::create(gatePin, gateDuration));
	
	return seq;
}


CVNoteSequencer* CVNoteSequencer::create(NoteSequenceData* notes, NoteName key, Scale scale, PinAnalogOut output, PinAnalogIn input, bool randomize)
{
	return new CVNoteSequencer(notes, key, scale, output, input, randomize);
}

CVNoteSequencer* CVNoteSequencer::create(aJsonObject* data)
{
	static const char randomizeNodeName[] = "randomize";
	static const char gateNodeName[] = "gateOutput";
	static const char durationNodeName[] = "gateLength";
	
	bool randomize = getBoolFromJSON(data, randomizeNodeName, false);	
	NoteSequenceData* notes = getNotesFromJSON(data);
	Scale scale = getScaleFromJSON(data);
	NoteName root = getRootFromJSON(data);
	PinAnalogOut output = getAnalogOutputFromJSON(data);
	PinAnalogIn input = getAnalogInputFromJSON(data);
	PinDigitalOut gatePin = getDigitalOutputFromJSON(data, gateNodeName);
	int gateDuration = getIntFromJSON(data, durationNodeName, 20, 1, 1000);
	
	CVNoteSequencer* seq = new CVNoteSequencer(notes, root, scale, output, input, randomize);
		
	if (gatePin != DIGITAL_OUT_NONE) seq->setgate(Gate::create(gatePin, gateDuration));
	
	return seq;
}


CVSequencer* CVSequencer::create(CVSequenceData* values, int clockdivision, PinAnalogOut output, bool randomize_seq)
{
	return new CVSequencer(values, clockdivision, output, randomize_seq);
}

CVSequencer* CVSequencer::create(int clockdivision, PinAnalogOut output)
{
	return new CVSequencer(0, 5000, clockdivision, output);
}

CVSequencer* CVSequencer::create(int min, int max, int clockdivision, PinAnalogOut output)
{
	return new CVSequencer(min, max, clockdivision, output);
}

CVSequencer* CVSequencer::create(aJsonObject* data)
{
	static const char randomizeNodeName[] = "randomize";
	static const char minNodeName[] = "min";
	static const char maxNodeName[] = "max";
	static const char valuesNodeName[] = "values";
	
	CVSequenceData* values = getIntCollectionFromJSON(data, valuesNodeName);
	bool randomize = getBoolFromJSON(data, randomizeNodeName, false);	
	int clockdivision = getDivisionFromJSON(data);
	PinAnalogOut output = getAnalogOutputFromJSON(data);
	int min = getIntFromJSON(data, minNodeName, 0, -10000, 10000);
	int max = getIntFromJSON(data, maxNodeName, 5000, -10000, 10000);
	
	if (values != NULL)
	{
		return new CVSequencer(values, clockdivision, output, randomize);
	}
	else
	{
		return new CVSequencer(min, max, clockdivision, output);
	}			
}

Sequencer::Sequencer()
{
	this->gate = NULL;
}

void Sequencer::setgate(Gate* gate)
{
	this->gate = gate;
}

TriggerSequencer::TriggerSequencer(vector<int>* triggers, int clockdivision, PinDigitalOut pin)
{
	this->triggers = new vector<int>();
	copy(triggers->begin(), triggers->end(), back_inserter(*this->triggers));
	
	this->clock_division = clockdivision;
	this->sequence_index = 0;
	this->state = false;
	this->trigger = Trigger::create(pin, clockdivision);
	digitalWrite(pin, LOW);	
}

void TriggerSequencer::timer(unsigned long t)
{
	this->trigger->timer(t);
}

void TriggerSequencer::reset()
{	
	this->sequence_index = ++(this->sequence_index) % this->triggers->size();

	if ((*this->triggers)[this->sequence_index] != 0)
	{
		this->trigger->reset();
	}	
}


DrumTriggerSequencer::DrumTriggerSequencer(vector<int>* triggers, int clockdivision, PinAnalogOut pin)
{
	this->triggers = new vector<int>();
	copy(triggers->begin(), triggers->end(), back_inserter(*this->triggers));
	
	this->clock_division = clockdivision;
	this->sequence_index = 0;
	this->state = false;
	this->trigger = DrumTrigger::create(pin, 850);
}

void DrumTriggerSequencer::timer(unsigned long t)
{
	this->trigger->timer(t);
}

void DrumTriggerSequencer::reset()
{	
	this->sequence_index = ++(this->sequence_index) % this->triggers->size();
	
	if ((*this->triggers)[this->sequence_index] != 0)
	{
		this->trigger->reset();
	}	
}


ProbabilityDrumTriggerSequencer::ProbabilityDrumTriggerSequencer(std::vector<int>* triggers, std::vector<int>* velocities, int velocityrange, int clockdivision, PinAnalogOut output)
{
	this->triggers = new vector<int>();
	copy(triggers->begin(), triggers->end(), back_inserter(*this->triggers));
	
	this->velocities = new vector<int>();
	copy(velocities->begin(), velocities->end(), back_inserter(*this->velocities));
	
	this->clock_division = clockdivision;
	this->sequence_index = 0;
	this->state = false;
	this->trigger = DrumTrigger::create(output, 850);
	this->velocitymodifierpin = DUE_IN_A_NONE;
	this->probabilitymodifierpin = DUE_IN_A_NONE;
	this->velocityrange = velocityrange;
	this->resetnext = false;
}

void ProbabilityDrumTriggerSequencer::setProbabilityModifier(PinAnalogIn pin)
{
	this->probabilitymodifierpin = pin;
}

void ProbabilityDrumTriggerSequencer::setVelocityModifier(PinAnalogIn pin)
{
	this->velocitymodifierpin = pin;
}

void ProbabilityDrumTriggerSequencer::calculate()
{
	this->sequence_index = (this->sequence_index + 1) % this->triggers->size();
	
	int currentvalue = (*this->triggers)[this->sequence_index];
	int currentvelocity = (*this->velocities)[this->sequence_index];
	
	if (currentvalue != 0)
	{		
		int rnd = Entropy::getValue(100);
		
		if (this->probabilitymodifierpin == DUE_IN_A_NONE)
		{
			this->resetnext = currentvalue >= rnd;	
			this->nextvelocity = currentvelocity;
		}
		else
		{
			int rawval = analogReadmV(probabilitymodifierpin);
			int factor = 100;
					
			/* Scale the factor based on 0-100 and 100-2000 */
			if (rawval < 2400)
			{
				factor = (rawval * 100) / 2400;
			}
			else if (rawval > 2600)
			{
				factor = 100 + ((rawval * 2600UL) / 2500);
			}
			
			this->resetnext = ((currentvalue * factor) / 100) >= rnd;
			
			if (this->resetnext)
			{
				//TODO: where do we scale velocities from 50 - 850???
				/* If we're triggering a hit next beat, calculate a velocity for it */
				if (this->velocityrange != 0)
				{
					int valuerange = (currentvalue * this->velocityrange) / 100;
					this->nextvelocity = Entropy::getValue(valuerange) - (valuerange / 2) + currentvelocity; 
				}
				else
				{
					this->nextvelocity = currentvelocity;
				}
			}
		}
	}	
}

void ProbabilityDrumTriggerSequencer::reset()
{		
	if (this->resetnext)
	{
		this->trigger->setAmplitude(this->nextvelocity);
		this->trigger->reset();
		this->resetnext = false;
	}	
}

void ProbabilityDrumTriggerSequencer::timer(unsigned long t)
{
	this->trigger->timer(t);
}



ProbabilityTriggerSequencer::ProbabilityTriggerSequencer(vector<int>* triggers, int clockdivision, PinDigitalOut pin) : nw2s::TriggerSequencer(triggers, clockdivision, pin)
{
	this->modifierpin = DUE_IN_A_NONE;
	this->sequence_index = 0;
	this->resetnext = false;
}

void ProbabilityTriggerSequencer::setProbabilityModifier(PinAnalogIn pin)
{
	this->modifierpin = pin;
}

void ProbabilityTriggerSequencer::calculate()
{
	this->sequence_index = ++(this->sequence_index) % this->triggers->size();

	int currentvalue = (*this->triggers)[this->sequence_index];

	if (currentvalue != 0)
	{		
		int rnd = Entropy::getValue(100);

		if (this->modifierpin == DUE_IN_A_NONE)
		{
			this->resetnext = currentvalue >= rnd;		
		}
		else
		{
			int rawval = analogReadmV(modifierpin);
			int factor = 100;
		
			//TODO: encapsulate scaling
			/* Scale the factor based on 0-100 and 100-2000 */
			if (rawval < 2400)
			{
				factor = (rawval * 100) / 2400;
			}
			else if (rawval > 2600)
			{
				factor = 100 + ((rawval * 2600UL) / 2400);
			}
			
			this->resetnext = ((currentvalue * factor) / 100) >= rnd;
		}
	}	
}

void ProbabilityTriggerSequencer::reset()
{	
	if (this->resetnext) this->trigger->reset();
}


NoteSequencer::NoteSequencer(vector<SequenceNote>* notes, NoteName key, Scale scale, int clockdivision, PinAnalogOut pin, bool randomize_seq)
{
	this->key = new Key(scale, key);
	this->output = AnalogOut::create(pin);
	this->sequence_index = 0;
	this->randomize_seq = randomize_seq;
	this->clock_division = clockdivision;
		
	/* Copy the sequence to our own memory */
	this->notes = new vector<SequenceNote>();
	copy(notes->begin(), notes->end(), back_inserter(*this->notes));
	
	this->sequence_index = (randomize_seq) ? random(this->notes->size()) : 0;
	
	/* Output the first note of the sequence */
	int startdegree = (*this->notes)[this->sequence_index].degree;
	int startoctave = (*this->notes)[this->sequence_index].octave;
	this->output = AnalogOut::create(pin);
	this->output->outputCV(this->key->getNoteMillivolt(startoctave, startdegree));		
}

void NoteSequencer::timer(unsigned long t)
{
	if (this->gate != NULL) this->gate->timer(t);
}

void NoteSequencer::reset()
{
	//TODO: move to calculate
	this->sequence_index = (randomize_seq) ? random(this->notes->size()) : ++(this->sequence_index) % this->notes->size();

	/* If there's a HOLD in the sequence, then we don't change the note or trigger any events */
	if ((*this->notes)[this->sequence_index].degree != 0)
	{
		this->current_degree = (*this->notes)[this->sequence_index].degree;
		this->current_octave = (*this->notes)[this->sequence_index].octave;		

		this->output->outputCV(this->key->getNoteMillivolt(this->current_octave, this->current_degree));		

		if (this->gate != NULL) this->gate->reset();
	}
}

CVNoteSequencer::CVNoteSequencer(NoteSequenceData* notes, NoteName key, Scale scale, PinAnalogOut pin, PinAnalogIn input, bool randomize_seq)
{	
	this->key = new Key(scale, key);
	this->output = output;
	this->sequence_index = 0;
	this->gate = NULL;
	this->last_note_t = 0;
	this->cv_in = input;
	this->randomize_seq = randomize_seq;
		
	/* Copy the sequence to our own memory */
	this->notes = new vector<SequenceNote>();
	copy(notes->begin(), notes->end(), back_inserter(*this->notes));

	/* Read the input and calculate the position in the sequence */	
	this->sequence_index = calculatePosition();

	/* Output the first note of the sequence */
	int startdegree = (*this->notes)[this->sequence_index].degree;
	int startoctave = (*this->notes)[this->sequence_index].octave;
	this->output = AnalogOut::create(pin);
	this->output->outputCV(this->key->getNoteMillivolt(startoctave, startdegree));	
}

void CVNoteSequencer::timer(unsigned long t)
{			
	int period_t = t - this->last_note_t;

	/* Only check the analog input every 50ms */
	if (t % 50 == 0)
	{
		/* Read the input and calculate the position in the sequence */
		int noteindex = calculatePosition();
	
		/* If the note is still the same, just be done */
		if (this->sequence_index == noteindex) return;
		
		this->sequence_index = noteindex;
		this->last_note_t = t;
		period_t = 0;

		int currentindex = (this->randomize_seq) ? random(this->notes->size()) : this->sequence_index;

		int degree = (*this->notes)[currentindex].degree;
		int octave = (*this->notes)[currentindex].octave;		
			
		this->output->outputCV(this->key->getNoteMillivolt(octave, degree));

		if (this->gate != NULL) this->gate->reset();		
	}

	if (this->gate != NULL) this->gate->timer(t);
}

void CVNoteSequencer::reset()
{
	
}

int CVNoteSequencer::calculatePosition()
{
	/* Get the input in mV, normalized 0-5V */
	return analogReadmV(cv_in, 0, 5000) / (5000 / (this->notes->size() - 1));
}



CVSequencer::CVSequencer(vector<int>* values, int clockdivision, PinAnalogOut pin, bool randomize_seq)
{
	this->output = output;
	this->sequence_index = 0;
	this->randomize_seq = randomize_seq;
	this->clock_division = clockdivision;

	/* Copy the sequence to our own memory */
	this->values = new vector<int>();
	copy(values->begin(), values->end(), back_inserter(*this->values));

	this->sequence_index = (randomize_seq) ? random(this->values->size()) : 0;
	this->current_value = (*this->values)[this->sequence_index];
	
	/* Output the first note of the sequence */
	this->output = AnalogOut::create(pin);
	this->output->outputCV(this->current_value);		
}

CVSequencer::CVSequencer(int min, int max, int clockdivision, PinAnalogOut pin)
{
	this->values = NULL;
	this->output = output;
	this->sequence_index = 0;
	this->randomize_seq = true;
	this->clock_division = clockdivision;

	this->min = (min < 0) ? 0 : min;
	this->max = (max > 5000) ? 5000 : max;

	this->current_value = random(this->min, this->max);
	
	/* Output the first note of the sequence */
	this->output = AnalogOut::create(pin);
	this->output->outputCV(this->current_value);		
}

void CVSequencer::timer(unsigned long t)
{	
	if (this->gate != NULL) this->gate->timer(t);
}

void CVSequencer::reset()
{
	if (values != NULL)
	{
		this->sequence_index = (randomize_seq) ? random(this->values->size()) : ++(this->sequence_index) % this->values->size();
		this->current_value = (*this->values)[this->sequence_index];
	}
	else
	{
		this->current_value = Entropy::getValue(this->min, this->max);		
	}

	Serial.println("value: " + String(this->current_value) + " " + String(this->sequence_index));
	
	this->output->outputCV(this->current_value);

	if (this->gate != NULL) this->gate->reset();
}

MorphingNoteSequencer::MorphingNoteSequencer(NoteSequenceData* notes, NoteName key, Scale scale, int chaos, int clockdivision, PinAnalogOut output, PinDigitalIn resetPin) : NoteSequencer(notes, key, scale, clockdivision, output, false)
{
	this->chaos = chaos;
	this->resetPin = resetPin;
	this->notesOriginal = new NoteSequenceData(this->notes->size());
	
	for (int i = 0; i < this->notes->size(); i++)
	{
		(*this->notesOriginal)[i] = (*this->notes)[i];
	}
}

void MorphingNoteSequencer::reset()
{	
	/* Theres a chance that we want to randomly (permanently) swap this note with another from the sequence */
	if ((!digitalRead(this->resetPin)) && (this->chaos >= random(100)))
	{
		int target = random(this->notes->size());
		SequenceNote targetNote = (*this->notes)[target];
		(*this->notes)[target] = (*this->notes)[this->sequence_index];
		(*this->notes)[this->sequence_index] = targetNote;
	}
	
	if (digitalRead(this->resetPin))
	{
		for (int i = 0; i < this->notes->size(); i++)
		{
			(*this->notes)[i] = (*this->notesOriginal)[i];
		}
	}
	
	NoteSequencer::reset();
}




