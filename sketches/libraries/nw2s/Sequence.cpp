

#include "Key.h"
#include "IO.h"
#include <iterator>
#include <vector>
#include "EventManager.h"
#include "Sequence.h"
#include <math.h>


using namespace std;
using namespace nw2s;

NoteSequence* NoteSequence::create(vector<SequenceNote>* notes, NoteName key, ScaleType scale, int tempo, AnalogOut output, DigitalOut gate_out, int gate_duration, bool randomize_seq, Slew* slew)
{
	return new NoteSequence(notes, key, scale, tempo, output, gate_out, gate_duration, randomize_seq, slew);
}

RandomNoteSequence* RandomNoteSequence::create(NoteName key, ScaleType scale, int tempo, AnalogOut output, DigitalOut gate_out, int gate_duration, Slew* slew)
{
	return new RandomNoteSequence(key, scale, tempo, output, gate_out, gate_duration, slew);
}

RandomTimeSequence* RandomTimeSequence::create(std::vector<SequenceNote>* notes, NoteName key, ScaleType scale, int mintempo, int maxtempo, AnalogOut output, DigitalOut gate_out, int gate_duration, bool randomize_seq, Slew* slew)
{
	return new RandomTimeSequence(notes, key, scale, mintempo, maxtempo, output, gate_out, gate_duration, randomize_seq, slew);
}

RandomTimeSequence* RandomTimeSequence::create(NoteName key, ScaleType scale, int mintempo, int maxtempo, AnalogOut output, DigitalOut gate_out, int gate_duration, Slew* slew)
{
	return new RandomTimeSequence(key, scale, mintempo, maxtempo, output, gate_out, gate_duration, slew);
}

NoteSequence::NoteSequence(vector<SequenceNote>* notes, NoteName key, ScaleType scale, int tempo, AnalogOut output, DigitalOut gate_out, int gate_duration, bool randomize_seq, Slew* slew)
{
	this->key = new Key(scale, key);
	this->output = output;
	this->sequence_index = 0;
	this->randomize_seq = randomize_seq;
	this->gate_out = gate_out;
	this->gate_duration = gate_duration;
	this->slew = slew;
		
	/* Copy the sequence to our own memory */
	this->notes = new vector<SequenceNote>();
	copy(notes->begin(), notes->end(), back_inserter(*this->notes));

	int noteindex = (randomize_seq) ? random(this->notes->size()) : 0;

	/* Output the first note of the sequence */
	int startdegree = (*this->notes)[noteindex].degree;
	int startoctave = (*this->notes)[noteindex].octave;
	ArduinoIO::dacOutputInitialize(output);
	ArduinoIO::dacOutputNote(output, this->key->getNote(startoctave, startdegree));

	/* Make sure the clock pin is high since we started the sequence */
	gate_state = HIGH;
    pinMode(this->gate_out, OUTPUT);
	digitalWrite(this->gate_out, HIGH);

	/* The fixed sequence operates on a regular period based on the tempo */
	int normalized_tempo = (tempo < 1) ? 1 : (tempo > 500) ? 500 : tempo;
	float periodms = (1.0 / normalized_tempo) * 60000.0;
	this->period = round(periodms);	
}

void NoteSequence::timer(unsigned long t)
{
	int period_t = t % this->period;
	
	if (period_t == 0)
	{
		int noteindex = (randomize_seq) ? random(this->notes->size()) : ++(this->sequence_index) % this->notes->size();
		this->current_degree = (*this->notes)[noteindex].degree;
		this->current_octave = (*this->notes)[noteindex].octave;		

		if (this->slew == NULL) ArduinoIO::dacOutputNote(output, this->key->getNote(this->current_octave, this->current_degree));
	}

	if (this->slew != NULL)
	{
		ArduinoIO::dacOutputSlewedNote(output, this->key->getNote(this->current_octave, this->current_degree), this->slew, period_t);
	}
	
	if ((this->gate_state == LOW) && (t % this->period < gate_duration))
	{
		this->gate_state = HIGH;
		digitalWrite(this->gate_out, HIGH);
	}
	else if ((this->gate_state == HIGH) && (t % this->period > gate_duration))
	{
		this->gate_state = LOW;
		digitalWrite(this->gate_out, LOW);
	}	
}

RandomNoteSequence::RandomNoteSequence(NoteName key, ScaleType scale, int tempo, AnalogOut output, DigitalOut gate_out, int gate_duration, Slew* slew)
{
	this->key = new Key(scale, key);
	this->output = output;
	this->gate_out = gate_out;
	this->gate_duration = gate_duration;
	this->slew = slew;
		
	/* Start the CV at a random note in the sequence */
	this->current_note = this->key->getRandomNote();
	ArduinoIO::dacOutputInitialize(output);
	ArduinoIO::dacOutputNote(output, this->current_note);

	/* Make sure the clock pin is high since we started the sequence */
	this->output = output;
	gate_state = HIGH;
	digitalWrite(this->output, HIGH);

	/* The fixed sequence operates on a regular period based on the tempo */
	//TODO: Convert to integers
	int normalized_tempo = (tempo < 1) ? 1 : (tempo > 500) ? 500 : tempo;
	float periodms = (1.0 / normalized_tempo) * 60000.0;
	this->period = round(periodms);	
}

void RandomNoteSequence::timer(unsigned long t)
{	
	if (t % this->period == 0)
	{
		this->current_note = this->key->getRandomNote();
		
		if (this->slew == NULL) ArduinoIO::dacOutputNote(output, this->current_note);		
	}

	if (this->slew != NULL)
	{
		ArduinoIO::dacOutputSlewedNote(output, this->current_note, this->slew, t % this->period);
	}

	if ((this->gate_state == LOW) && (t % this->period < gate_duration))
	{
		this->gate_state = HIGH;
		digitalWrite(this->gate_out, HIGH);
	}
	else if ((this->gate_state == HIGH) && (t % this->period > gate_duration))
	{
		this->gate_state = LOW;
		digitalWrite(this->gate_out, LOW);
	}	
}

RandomTimeSequence::RandomTimeSequence(vector<SequenceNote>* notes, NoteName key, ScaleType scale, int mintempo, int maxtempo, AnalogOut output, DigitalOut gate_out, int gate_duration, bool randomize_seq, Slew* slew)
{
	this->key = new Key(scale, key);
	this->output = output;
	this->gate_out = gate_out;
	this->gate_duration = gate_duration;
	this->sequence_index = 0;
	this->randomize_seq = randomize_seq;
	this->last_t = 0;
	this->next_t = 0;
	this->slew = slew;
		
	/* Copy the sequence to our own memory */
	this->notes = new vector<SequenceNote>();
	copy(notes->begin(), notes->end(), back_inserter(*this->notes));

	int noteindex = (randomize_seq) ? random(this->notes->size()) : 0;

	/* Start at either the first or a random item in the sequence */
	int startdegree = (*this->notes)[noteindex].degree;
	int startoctave = (*this->notes)[noteindex].octave;
	ArduinoIO::dacOutputInitialize(output);
	ArduinoIO::dacOutputNote(output, this->key->getNote(startoctave, startdegree));

	/* Make sure the clock pin is high since we started the sequence */
	gate_state = HIGH;
	digitalWrite(this->gate_out, HIGH);

	/* The fixed sequence operates on a regular period based on the tempo */
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
		
		if (this->slew == NULL) ArduinoIO::dacOutputNote(output, this->current_note);
		
		/* Calculation of the current tempo is only done on clock tick */
		this->last_t = t;	
		this->calculate_next_t(t);		
	}

	if (this->slew != NULL)
	{
		ArduinoIO::dacOutputSlewedNote(output, this->current_note, this->slew, next_t - t);
	}

	if ((gate_state == LOW) && (t >= this->next_t))
	{
		this->gate_state = HIGH;
		digitalWrite(this->gate_out, HIGH);
	}
	else if ((gate_state == HIGH) && (t > this->last_t + gate_duration))
	{
		this->gate_state = LOW;
		digitalWrite(this->gate_out, LOW);
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


