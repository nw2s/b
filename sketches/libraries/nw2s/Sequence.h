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
#include "Gate.h"
#include "Envelope.h"
#include "Clock.h"

namespace nw2s
{
	
	struct SequenceNote
	{
		int octave;
		int degree;
	};
	
	typedef std::vector<SequenceNote> NoteSequenceData;

	class Sequence;
	class NoteSequence;
	class RandomNoteSequence;
	class RandomTimeSequence;
	class CVNoteSequence;
	class GaussianTimeSeuqnce;
	class GaussianNoteSequence;
	
	class TimeBasedDevice;
}

class nw2s::Sequence : public nw2s::BeatDevice
{
	public: 
		void setgate(Gate* gate);
		void setslew(Slew* slew);
		void seteg(Envelope* eg);
		virtual void timer(unsigned long t) = 0;
		virtual void reset() = 0;
	
	protected:
		Gate* gate;	
		Slew* slew;
		Envelope* envelope;
		
		Sequence();
};


class nw2s::NoteSequence : public nw2s::Sequence
{
	public:
		static NoteSequence* create(std::vector<SequenceNote>* notes, NoteName key, ScaleType scale, int clockdivision, PinAnalogOut output, bool randomize_seq = false);
		virtual void timer(unsigned long t);
		virtual void reset();
	
	private:
		bool randomize_seq;
		int current_degree;
		int current_octave;
		volatile int sequence_index;
		std::vector<SequenceNote>* notes;
		Key* key;
	 	AnalogOut* output;
		
		NoteSequence(std::vector<SequenceNote>* notes, NoteName key, ScaleType scale, int clockdivision, PinAnalogOut output, bool randomize_seq);
};

class nw2s::RandomNoteSequence : public nw2s::Sequence
{
	public:
		static RandomNoteSequence* create(NoteName key, ScaleType scale, int clockdivision, PinAnalogOut output);
		virtual void timer(unsigned long t);
		virtual void reset();
	
	private:
		Key* key;
		AnalogOut* output;
		ScaleNote current_note;
		
		RandomNoteSequence(NoteName key, ScaleType scale, int clockdivision, PinAnalogOut output);
};

class nw2s::CVNoteSequence : public nw2s::Sequence
{
	public:
		static CVNoteSequence* create(std::vector<SequenceNote>* notes, NoteName key, ScaleType scale, PinAnalogOut output, PinAnalogIn input);
		virtual void timer(unsigned long t);
		virtual void reset();
	
	private:
		volatile int sequence_index;
		std::vector<SequenceNote>* notes;
		Key* key;
	 	AnalogOut* output;
		PinAnalogIn cv_in;
		unsigned long last_note_t;
		
		CVNoteSequence(std::vector<SequenceNote>* notes, NoteName key, ScaleType scale, PinAnalogOut output, PinAnalogIn input);
};

#endif

