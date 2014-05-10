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


using namespace std;
using namespace nw2s;

TriggerSequencer* TriggerSequencer::create(vector<int>* triggers, int clockdivision, PinDigitalOut output)
{
	return new TriggerSequencer(triggers, clockdivision, output);
}

DrumTriggerSequencer* DrumTriggerSequencer::create(std::vector<int>* triggers, int clockdivision, PinAnalogOut output)
{
	return new DrumTriggerSequencer(triggers, clockdivision, output);
}

ProbabilityTriggerSequencer* ProbabilityTriggerSequencer::create(vector<int>* triggers, int clockdivision, PinDigitalOut output)
{
	return new ProbabilityTriggerSequencer(triggers, clockdivision, output);
}

ProbabilityDrumTriggerSequencer* ProbabilityDrumTriggerSequencer::create(vector<int>* triggers, vector<int>* velocities, int velocityrange, int clockdivision, PinAnalogOut output)
{
	return new ProbabilityDrumTriggerSequencer(triggers, velocities, velocityrange, clockdivision, output);
}

NoteSequencer* NoteSequencer::create(vector<SequenceNote>* notes, NoteName key, ScaleType scale, int clockdivision, PinAnalogOut output, bool randomize_seq)
{
	return new NoteSequencer(notes, key, scale, clockdivision, output, randomize_seq);
}

MorphingNoteSequencer* MorphingNoteSequencer::create(vector<SequenceNote>* notes, NoteName key, ScaleType scale, int chaos, int clockdivision, PinAnalogOut output)
{
	return new MorphingNoteSequencer(notes, key, scale, chaos, clockdivision, output);
}

RandomNoteSequencer* RandomNoteSequencer::create(NoteName key, ScaleType scale, int clockdivision, PinAnalogOut output)
{
	return new RandomNoteSequencer(key, scale, clockdivision, output);
}

CVNoteSequencer* CVNoteSequencer::create(NoteSequenceData* notes, NoteName key, ScaleType scale, PinAnalogOut output, PinAnalogIn input)
{
	return new CVNoteSequencer(notes, key, scale, output, input);
}

CVSequencer* CVSequencer::create(vector<int>* values, int clockdivision, PinAnalogOut output, bool randomize_seq)
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


Sequencer::Sequencer()
{
	this->gate = NULL;
	this->slew = NULL;
	this->envelope = NULL;
}

void Sequencer::setgate(Gate* gate)
{
	this->gate = gate;
}

void Sequencer::setslew(Slew* slew)
{
	this->slew = slew;
}

void Sequencer::seteg(Envelope* envelope)
{
	this->envelope = envelope;
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


NoteSequencer::NoteSequencer(vector<SequenceNote>* notes, NoteName key, ScaleType scale, int clockdivision, PinAnalogOut pin, bool randomize_seq)
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
	this->output->outputNoteCV(this->key->getNote(startoctave, startdegree));		
}

void NoteSequencer::timer(unsigned long t)
{
	if (this->slew != NULL) this->output->outputSlewedNoteCV(this->key->getNote(this->current_octave, this->current_degree), this->slew);
	if (this->gate != NULL) this->gate->timer(t);
	if (this->envelope != NULL) this->envelope->timer(t);	
}

void NoteSequencer::reset()
{
	this->sequence_index = (randomize_seq) ? random(this->notes->size()) : ++(this->sequence_index) % this->notes->size();

	/* If there's a HOLD in the sequence, then we don't change the note or trigger any events */
	if ((*this->notes)[this->sequence_index].degree != 0)
	{
		this->current_degree = (*this->notes)[this->sequence_index].degree;
		this->current_octave = (*this->notes)[this->sequence_index].octave;		

		if (this->slew == NULL) this->output->outputNoteCV(this->key->getNote(this->current_octave, this->current_degree));

		if (this->gate != NULL) this->gate->reset();
		if (this->envelope != NULL) this->envelope->reset();
	}
}

RandomNoteSequencer::RandomNoteSequencer(NoteName key, ScaleType scale, int clockdivision, PinAnalogOut pin)
{
	this->key = new Key(scale, key);
	this->output = output;
	this->clock_division = clockdivision;
		
	/* Start the CV at a random note in the sequence */
	this->current_note = this->key->getRandomNote();
	this->output = AnalogOut::create(pin);
	this->output->outputNoteCV(this->current_note);
}

void RandomNoteSequencer::timer(unsigned long t)
{	
	if (this->slew != NULL) this->output->outputSlewedNoteCV(this->current_note, this->slew);	
	if (this->gate != NULL) this->gate->timer(t);
	if (this->envelope != NULL) this->envelope->timer(t);
}

void RandomNoteSequencer::reset()
{
	this->current_note = this->key->getRandomNote();		
	if (this->slew == NULL) this->output->outputNoteCV(this->current_note);		
	if (this->envelope != NULL) this->envelope->reset();
}

CVNoteSequencer::CVNoteSequencer(NoteSequenceData* notes, NoteName key, ScaleType scale, PinAnalogOut pin, PinAnalogIn input)
{	
	this->key = new Key(scale, key);
	this->output = output;
	this->sequence_index = 0;
	this->gate = NULL;
	this->last_note_t = 0;
	this->cv_in = input;
		
	/* Copy the sequence to our own memory */
	this->notes = new vector<SequenceNote>();
	copy(notes->begin(), notes->end(), back_inserter(*this->notes));

	/* Read the input and calculate the position in the sequence */	
	unsigned long noteindex = (((analogRead(cv_in) * 1000UL) / 1023UL) * this->notes->size()) / 1000UL;
	this->sequence_index = noteindex;

	/* Output the first note of the sequence */
	int startdegree = (*this->notes)[noteindex].degree;
	int startoctave = (*this->notes)[noteindex].octave;
	this->output = AnalogOut::create(pin);
	this->output->outputNoteCV(this->key->getNote(startoctave, startdegree));	
}

void CVNoteSequencer::timer(unsigned long t)
{			
	int period_t = t - this->last_note_t;

	if ((this->slew == NULL) && (t % 50 == 0)) 
	{
		//TODO: Optimize even more
		/* Save some cycles, only do this every 50ms */
	
		/* Read the input and calculate the position in the sequence */
		//TODO: This is buggy - fix!
		unsigned long noteindex = (((analogRead(cv_in) * 1000UL) / 3700UL) * this->notes->size()) / 1000UL;
	
		/* If the note is still the same, just be done */
		if (this->sequence_index == noteindex) return;
		
		this->sequence_index = noteindex;
		this->last_note_t = t;
		period_t = 0;

		this->sequence_index = noteindex;
		int degree = (*this->notes)[noteindex].degree;
		int octave = (*this->notes)[noteindex].octave;		
			
		this->output->outputNoteCV(this->key->getNote(octave, degree));

		if (this->envelope != NULL) this->envelope->reset();		
	}
	else if (this->slew != NULL)
	{		
		/* Read the input and calculate the position in the sequence */
		int noteindex = (((analogRead(cv_in) * 1000UL) / 3600UL) * this->notes->size()) / 1000UL;
	
		if (this->sequence_index != noteindex)
		{
			this->sequence_index = noteindex;
			this->last_note_t = t;
		}
		
		this->sequence_index = noteindex;
		int degree = (*this->notes)[noteindex].degree;
		int octave = (*this->notes)[noteindex].octave;		
	
		this->output->outputSlewedNoteCV(this->key->getNote(octave, degree), this->slew);
	}

	if (this->gate != NULL) this->gate->timer(t);
	if (this->envelope != NULL) this->envelope->timer(t);	
}

void CVNoteSequencer::reset()
{
	
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
	this->current_value = 
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
	this->randomize_seq = randomize_seq;
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
	if (this->slew != NULL) 
	{
		this->output->outputSlewedCV(this->current_value, this->slew);
	}
	if (this->gate != NULL) this->gate->timer(t);
	if (this->envelope != NULL) this->envelope->timer(t);	
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
		this->current_value = random(0, 5000);		
	}

	if (this->slew == NULL) this->output->outputCV(this->current_value);

	if (this->gate != NULL) this->gate->reset();
	if (this->envelope != NULL) this->envelope->reset();
}

MorphingNoteSequencer::MorphingNoteSequencer(vector<SequenceNote>* notes, NoteName key, ScaleType scale, int chaos, int clockdivision, PinAnalogOut output) : NoteSequencer(notes, key, scale, clockdivision, output, false)
{
	this->chaos = chaos;	
}

void MorphingNoteSequencer::reset()
{	
	/* Theres a chance that we want to randomly (permanently) swap this note with another from the sequence */
	if (this->chaos >= random(100))
	{
		int target = random(this->notes->size());
		SequenceNote targetNote = (*this->notes)[target];
		(*this->notes)[target] = (*this->notes)[this->sequence_index];
		(*this->notes)[this->sequence_index] = targetNote;
	}
	
	NoteSequencer::reset();
}




