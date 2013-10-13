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


using namespace std;
using namespace nw2s;

NoteSequence* NoteSequence::create(vector<SequenceNote>* notes, NoteName key, ScaleType scale, int tempo, PinAnalogOut output, bool randomize_seq)
{
	return new NoteSequence(notes, key, scale, tempo, output, randomize_seq);
}

RandomNoteSequence* RandomNoteSequence::create(NoteName key, ScaleType scale, int tempo, PinAnalogOut output)
{
	return new RandomNoteSequence(key, scale, tempo, output);
}

RandomTimeSequence* RandomTimeSequence::create(std::vector<SequenceNote>* notes, NoteName key, ScaleType scale, int mintempo, int maxtempo, PinAnalogOut output, bool randomize_seq)
{
	return new RandomTimeSequence(notes, key, scale, mintempo, maxtempo, output, randomize_seq);
}

RandomTimeSequence* RandomTimeSequence::create(NoteName key, ScaleType scale, int mintempo, int maxtempo, PinAnalogOut output)
{
	return new RandomTimeSequence(key, scale, mintempo, maxtempo, output);
}

CVNoteSequence* CVNoteSequence::create(vector<SequenceNote>* notes, NoteName key, ScaleType scale, PinAnalogOut output, PinAnalogIn input)
{
	return new CVNoteSequence(notes, key, scale, output, input);
}

void Sequence::setgate(Gate* gate)
{
	this->gate = gate;
}

void Sequence::setslew(Slew* slew)
{
	this->slew = slew;
}

NoteSequence::NoteSequence(vector<SequenceNote>* notes, NoteName key, ScaleType scale, int tempo, PinAnalogOut pin, bool randomize_seq)
{
	this->key = new Key(scale, key);
	this->output = output;
	this->sequence_index = 0;
	this->randomize_seq = randomize_seq;
	this->gate = NULL;
		
	/* Copy the sequence to our own memory */
	this->notes = new vector<SequenceNote>();
	copy(notes->begin(), notes->end(), back_inserter(*this->notes));
	
	int noteindex = (randomize_seq) ? random(this->notes->size()) : 0;
	
	/* Output the first note of the sequence */
	int startdegree = (*this->notes)[noteindex].degree;
	int startoctave = (*this->notes)[noteindex].octave;
	this->output = AnalogOut::create(pin);
	this->output->outputNoteCV(this->key->getNote(startoctave, startdegree));
		
	/* The fixed sequence operates on a regular period based on the tempo */
	int normalized_tempo = (tempo < 1) ? 1 : (tempo > 500) ? 500 : tempo;
	float periodms = (1.0 / normalized_tempo) * 60000.0;
	this->period = round(periodms);	
}

void NoteSequence::timer(unsigned long t)
{
	unsigned int period_t = t % this->period;
	
	if (period_t == 0)
	{
		int noteindex = (randomize_seq) ? random(this->notes->size()) : ++(this->sequence_index) % this->notes->size();
		this->current_degree = (*this->notes)[noteindex].degree;
		this->current_octave = (*this->notes)[noteindex].octave;		

		if (this->slew == NULL) this->output->outputNoteCV(this->key->getNote(this->current_octave, this->current_degree));
	}

	if (this->slew != NULL)
	{
		this->output->outputSlewedNoteCV(this->key->getNote(this->current_octave, this->current_degree), this->slew, period_t);
	}
	
	if (this->gate != NULL)
	{
		this->gate->timer(period_t);
	}
	
}

RandomNoteSequence::RandomNoteSequence(NoteName key, ScaleType scale, int tempo, PinAnalogOut pin)
{
	this->key = new Key(scale, key);
	this->output = output;
	this->gate = NULL;
		
	/* Start the CV at a random note in the sequence */
	this->current_note = this->key->getRandomNote();
	this->output = AnalogOut::create(pin);
	this->output->outputNoteCV(this->current_note);

	/* The fixed sequence operates on a regular period based on the tempo */
	//TODO: Convert to integers
	int normalized_tempo = (tempo < 1) ? 1 : (tempo > 500) ? 500 : tempo;
	float periodms = (1.0 / normalized_tempo) * 60000.0;
	this->period = round(periodms);	
}

void RandomNoteSequence::timer(unsigned long t)
{	
	unsigned int period_t = t % this->period;
	
	if (period_t == 0)
	{
		this->current_note = this->key->getRandomNote();
		
		if (this->slew == NULL) this->output->outputNoteCV(this->current_note);		
	}

	if (this->slew != NULL)
	{
		this->output->outputSlewedNoteCV(this->current_note, this->slew, t % this->period);
	}
	
	if (this->gate != NULL)
	{
		this->gate->timer(period_t);
	}

}

RandomTimeSequence::RandomTimeSequence(vector<SequenceNote>* notes, NoteName key, ScaleType scale, int mintempo, int maxtempo, PinAnalogOut pin, bool randomize_seq)
{
	this->key = new Key(scale, key);
	this->output = output;
	this->gate = NULL;
	this->sequence_index = 0;
	this->randomize_seq = randomize_seq;
	this->last_t = 0;
	this->next_t = 0;
		
	/* Copy the sequence to our own memory */
	this->notes = new vector<SequenceNote>();
	copy(notes->begin(), notes->end(), back_inserter(*this->notes));

	int noteindex = (randomize_seq) ? random(this->notes->size()) : 0;

	/* Start at either the first or a random item in the sequence */
	int startdegree = (*this->notes)[noteindex].degree;
	int startoctave = (*this->notes)[noteindex].octave;
	this->output = AnalogOut::create(pin);
	this->output->outputNoteCV(this->key->getNote(startoctave, startdegree));

	/* The random time sequence operates on a random period based on the tempo */
	this->mintempo = (mintempo < 1) ? 1 : (mintempo > 500) ? 500 : mintempo;
	this->maxtempo = (maxtempo < 1) ? 1 : (maxtempo > 500) ? 500 : maxtempo;
}

void RandomTimeSequence::timer(unsigned long t)
{	
	if (this->next_t == 0)
	{
		this->calculate_next_t(t);
	}
	else if (t >= next_t)
	{
		int noteindex = (randomize_seq) ? random(this->notes->size()) : ++(this->sequence_index) % this->notes->size();
		int degree = (*this->notes)[noteindex].degree;
		int octave = (*this->notes)[noteindex].octave;
		
		this->current_note = this->key->getNote(octave, degree);
		
		if (this->slew == NULL) this->output->outputNoteCV(this->current_note);
		
		/* Calculation of the current tempo is only done on clock tick */
		this->last_t = t;	
		this->calculate_next_t(t);		
	}

	if (this->slew != NULL)
	{
		this->output->outputSlewedNoteCV(this->current_note, this->slew, t - last_t);
	}

	if (this->gate != NULL)
	{
		this->gate->timer(t - last_t);
	}
}

void RandomTimeSequence::calculate_next_t(unsigned long t)
{
	int tempo = random(mintempo, maxtempo);
	
	//TODO: Convert to integers
	float periodms = (1.0 / tempo) * 60000.0;

	this->next_t = (this->last_t + round(periodms));
	
	Serial.print("\n t - " + String(this->next_t));
}

CVNoteSequence::CVNoteSequence(vector<SequenceNote>* notes, NoteName key, ScaleType scale, PinAnalogOut pin, PinAnalogIn input)
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

void CVNoteSequence::timer(unsigned long t)
{			
	if ((this->slew == NULL) && (t % 50 == 0)) 
	{
		/* Save some cycles, only do this every 50ms */
	
		/* Read the input and calculate the position in the sequence */
		unsigned long noteindex = (((analogRead(cv_in) * 1000UL) / 1023UL) * this->notes->size()) / 1000UL;
	
		/* If the note is still the same, just be done */
		if (this->sequence_index == noteindex) return;
		
		this->sequence_index = noteindex;
		this->last_note_t = t;
	
		this->sequence_index = noteindex;
		int degree = (*this->notes)[noteindex].degree;
		int octave = (*this->notes)[noteindex].octave;		
			
		this->output->outputNoteCV(this->key->getNote(octave, degree));
	}
	else if (this->slew != NULL)
	{
		/* Read the input and calculate the position in the sequence */
		int noteindex = (((analogRead(cv_in) * 1000UL) / 1023UL) * this->notes->size()) / 1000UL;
	
		if (this->sequence_index != noteindex)
		{
			this->sequence_index = noteindex;
			this->last_note_t = t;
		}
	
		this->sequence_index = noteindex;
		int degree = (*this->notes)[noteindex].degree;
		int octave = (*this->notes)[noteindex].octave;		
	
		this->output->outputSlewedNoteCV(this->key->getNote(octave, degree), this->slew, t - this->last_note_t);
	}

	if (this->gate != NULL)
	{
		this->gate->timer(t - this->last_note_t);
	}
	
}







