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

NoteSequence* NoteSequence::create(vector<SequenceNote>* notes, NoteName key, ScaleType scale, int clockdivision, PinAnalogOut output, bool randomize_seq)
{
	return new NoteSequence(notes, key, scale, clockdivision, output, randomize_seq);
}

RandomNoteSequence* RandomNoteSequence::create(NoteName key, ScaleType scale, int clockdivision, PinAnalogOut output)
{
	return new RandomNoteSequence(key, scale, clockdivision, output);
}

CVNoteSequence* CVNoteSequence::create(vector<SequenceNote>* notes, NoteName key, ScaleType scale, PinAnalogOut output, PinAnalogIn input)
{
	return new CVNoteSequence(notes, key, scale, output, input);
}

Sequence::Sequence()
{
	this->gate = NULL;
	this->slew = NULL;
	this->envelope = NULL;
}

void Sequence::setgate(Gate* gate)
{
	this->gate = gate;
}

void Sequence::setslew(Slew* slew)
{
	this->slew = slew;
}

void Sequence::seteg(Envelope* envelope)
{
	this->envelope = envelope;
}


NoteSequence::NoteSequence(vector<SequenceNote>* notes, NoteName key, ScaleType scale, int clockdivision, PinAnalogOut pin, bool randomize_seq)
{
	this->key = new Key(scale, key);
	this->output = output;
	this->sequence_index = 0;
	this->randomize_seq = randomize_seq;
	this->clock_division = clockdivision;
		
	/* Copy the sequence to our own memory */
	this->notes = new vector<SequenceNote>();
	copy(notes->begin(), notes->end(), back_inserter(*this->notes));
	
	int noteindex = (randomize_seq) ? random(this->notes->size()) : 0;
	
	/* Output the first note of the sequence */
	int startdegree = (*this->notes)[noteindex].degree;
	int startoctave = (*this->notes)[noteindex].octave;
	this->output = AnalogOut::create(pin);
	this->output->outputNoteCV(this->key->getNote(startoctave, startdegree));		
}

void NoteSequence::timer(unsigned long t)
{
	if (this->slew != NULL) this->output->outputSlewedNoteCV(this->key->getNote(this->current_octave, this->current_degree), this->slew);
	if (this->gate != NULL) this->gate->timer(t);
	if (this->envelope != NULL) this->envelope->timer(t);	
}

void NoteSequence::reset()
{
	int noteindex = (randomize_seq) ? random(this->notes->size()) : ++(this->sequence_index) % this->notes->size();
	this->current_degree = (*this->notes)[noteindex].degree;
	this->current_octave = (*this->notes)[noteindex].octave;		

	if (this->slew == NULL) this->output->outputNoteCV(this->key->getNote(this->current_octave, this->current_degree));

	if (this->gate != NULL) this->gate->reset();
	if (this->envelope != NULL) this->envelope->reset();
}

RandomNoteSequence::RandomNoteSequence(NoteName key, ScaleType scale, int clockdivision, PinAnalogOut pin)
{
	this->key = new Key(scale, key);
	this->output = output;
	this->clock_division = clockdivision;
		
	/* Start the CV at a random note in the sequence */
	this->current_note = this->key->getRandomNote();
	this->output = AnalogOut::create(pin);
	this->output->outputNoteCV(this->current_note);
}

void RandomNoteSequence::timer(unsigned long t)
{	
	if (this->slew != NULL) this->output->outputSlewedNoteCV(this->current_note, this->slew);	
	if (this->gate != NULL) this->gate->timer(t);
	if (this->envelope != NULL) this->envelope->timer(t);
}

void RandomNoteSequence::reset()
{
	this->current_note = this->key->getRandomNote();		
	if (this->slew == NULL) this->output->outputNoteCV(this->current_note);		
	if (this->envelope != NULL) this->envelope->reset();
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
	int period_t = t - this->last_note_t;

	if ((this->slew == NULL) && (t % 50 == 0)) 
	{
		/* Save some cycles, only do this every 50ms */
	
		/* Read the input and calculate the position in the sequence */
		unsigned long noteindex = (((analogRead(cv_in) * 1000UL) / 1023UL) * this->notes->size()) / 1000UL;
	
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
		int noteindex = (((analogRead(cv_in) * 1000UL) / 1023UL) * this->notes->size()) / 1000UL;
	
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







