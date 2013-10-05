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

#ifndef Sequence_h
#define Sequence_h

#include "IO.h"
#include "Slew.h"

namespace nw2s
{
	struct SequenceNote
	{
		int octave;
		int degree;
	};
	
	class NoteSequence;
	class RandomNoteSequence;
	class RandomTimeSequence;
	class GaussianTimeSeuqnce;
	class GaussianNoteSequence;
	
	class TimeBasedDevice;
}

//TODO: Add a generalized input map for transpose, tempo, trigger length
//TODO: Modulatable parameters EM.reg(new CVmodsource(input1, notesequence1.pitchmodulator))
//TODO: fix random timings to beat
//TODO: speed params on slews
//TODO: RandomTimeSeq based on scale

class nw2s::NoteSequence : public nw2s::TimeBasedDevice
{
	public:
		static NoteSequence* create(std::vector<SequenceNote>* notes, NoteName key, ScaleType scale, int tempo, AnalogOut output, DigitalOut gate_out, int gate_duration, bool randomize_seq, Slew* slew);
		virtual void timer(unsigned long t);
	
	private:
		bool randomize_seq;
		int current_degree;
		int current_octave;
		int period;
		int gate_duration;
		volatile int sequence_index;
		volatile int gate_state;
		std::vector<SequenceNote>* notes;
		Key* key;
		AnalogOut output;
		DigitalOut gate_out;
		Slew* slew;
		
		NoteSequence(std::vector<SequenceNote>* notes, NoteName key, ScaleType scale, int tempo, AnalogOut output, DigitalOut gate_out, int gate_duration, bool randomize_seq, Slew* slew);
};

class nw2s::RandomNoteSequence : public nw2s::TimeBasedDevice
{
	public:
		static RandomNoteSequence* create(NoteName key, ScaleType scale, int tempo, AnalogOut output, DigitalOut gate_out, int gate_duration, Slew* slew);
		virtual void timer(unsigned long t);
	
	private:
		Key* key;
		int period;
		AnalogOut output;
		DigitalOut gate_out;
		volatile int gate_state;
		int gate_duration;
		Slew* slew;
		ScaleNote current_note;
		
		RandomNoteSequence(NoteName key, ScaleType scale, int tempo, AnalogOut output, DigitalOut gate_out, int gate_duration, Slew* slew);
};

class nw2s::RandomTimeSequence : public nw2s::TimeBasedDevice
{
	public:
		static RandomTimeSequence* create(std::vector<SequenceNote>* notes, NoteName key, ScaleType scale, int mintempo, int maxtempo, AnalogOut output, DigitalOut gate_out, int gate_duration, bool randomize_seq, Slew* slew);
		static RandomTimeSequence* create(NoteName key, ScaleType scale, int mintempo, int maxtempo, AnalogOut output, DigitalOut gate_out, int gate_duration, Slew* slew);
		virtual void timer(unsigned long t);
	
	private:
		std::vector<SequenceNote>* notes;
		Key* key;
		int mintempo;
		int maxtempo;
		bool randomize_seq;
		AnalogOut output;
		volatile int sequence_index;
		volatile unsigned long last_t;
		volatile unsigned long next_t;
		DigitalOut gate_out;
		volatile int gate_state;
		int gate_duration;
		Slew* slew;
		ScaleNote current_note;
		
		RandomTimeSequence(std::vector<SequenceNote>* notes, NoteName key, ScaleType scale, int mintempo, int maxtempo, AnalogOut output, DigitalOut gate_out, int gate_duration, bool randomize_seq, Slew* slew);
		RandomTimeSequence(NoteName key, ScaleType scale, int mintempo, int maxtempo, AnalogOut output, DigitalOut gate_out, int gate_duration, Slew* slew);
		void calculate_next_t(unsigned long t);
		
};




#endif

