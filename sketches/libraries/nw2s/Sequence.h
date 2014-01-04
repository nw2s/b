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
#include "Trigger.h"

namespace nw2s
{
	struct SequenceNote
	{
		int octave;
		int degree;
	};
	
	static const SequenceNote HOLD = {0, 0};
	
	typedef std::vector<SequenceNote> NoteSequenceData;
	typedef std::vector<int> TriggerSequenceData;

	class Sequencer;
	class NoteSequencer;
	class RandomNoteSequencer;
	class RandomTimeSequencer;
	class CVNoteSequencer;
	class CVSequencer;
	class MorphingNoteSequencer;
	class TriggerSequencer;
	class ProbabilityTriggerSequencer;
	
	class TimeBasedDevice;
}

class nw2s::Sequencer : public nw2s::BeatDevice
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
		
		Sequencer();
};


class nw2s::TriggerSequencer : public nw2s::Sequencer
{
	public: 
		static TriggerSequencer* create(std::vector<int>* triggers, int clockdivision, PinDigitalOut output);
		virtual void timer(unsigned long t);
		virtual void reset();
		
	protected:
		TriggerSequencer(std::vector<int>* triggers, int clockdivision, PinDigitalOut output);
		Trigger* trigger;
		std::vector<int>* triggers;
		volatile int sequence_index;
		volatile bool state;
};

class nw2s::ProbabilityTriggerSequencer : public nw2s::TriggerSequencer
{
	public:
		static ProbabilityTriggerSequencer* create(std::vector<int>* triggers, int clockdivision, PinDigitalOut output);
		virtual void reset();
		virtual void calculate();
		void setProbabilityModifier(PinAnalogIn pin);
	
	private:
		bool resetnext;
		PinAnalogIn modifierpin;
		ProbabilityTriggerSequencer(std::vector<int>* triggers, int clockdivision, PinDigitalOut output);
};

class nw2s::NoteSequencer : public nw2s::Sequencer
{
	public:
		static NoteSequencer* create(std::vector<SequenceNote>* notes, NoteName key, ScaleType scale, int clockdivision, PinAnalogOut output, bool randomize_seq = false);
		virtual void timer(unsigned long t);
		virtual void reset();
	
	private:
		bool randomize_seq;
		int current_degree;
		int current_octave;
		Key* key;
	 	AnalogOut* output;

	
	protected:
		volatile int sequence_index;
		std::vector<SequenceNote>* notes;
		
		NoteSequencer(std::vector<SequenceNote>* notes, NoteName key, ScaleType scale, int clockdivision, PinAnalogOut output, bool randomize_seq);
};

class nw2s::RandomNoteSequencer : public nw2s::Sequencer
{
	public:
		static RandomNoteSequencer* create(NoteName key, ScaleType scale, int clockdivision, PinAnalogOut output);
		virtual void timer(unsigned long t);
		virtual void reset();
	
	private:
		Key* key;
		AnalogOut* output;
		ScaleNote current_note;
		
		RandomNoteSequencer(NoteName key, ScaleType scale, int clockdivision, PinAnalogOut output);
};

class nw2s::CVNoteSequencer : public nw2s::Sequencer
{
	public:
		static CVNoteSequencer* create(std::vector<SequenceNote>* notes, NoteName key, ScaleType scale, PinAnalogOut output, PinAnalogIn input);
		virtual void timer(unsigned long t);
		virtual void reset();
	
	private:
		volatile int sequence_index;
		std::vector<SequenceNote>* notes;
		Key* key;
	 	AnalogOut* output;
		PinAnalogIn cv_in;
		unsigned long last_note_t;
		
		CVNoteSequencer(std::vector<SequenceNote>* notes, NoteName key, ScaleType scale, PinAnalogOut output, PinAnalogIn input);
};

class  nw2s::CVSequencer : public Sequencer
{
	public:
		static CVSequencer* create(std::vector<int>* values, int clockdivision, PinAnalogOut output, bool randomize_seq = false);
		static CVSequencer* create(int clockdivision, PinAnalogOut output);
		static CVSequencer* create(int min, int max, int clockdivision, PinAnalogOut output);
		virtual void timer(unsigned long t);
		virtual void reset();

	private:
		bool randomize_seq;
		int min;
		int max;
		volatile int sequence_index;
		volatile int current_value;
		std::vector<int>* values;
	 	AnalogOut* output;
	
		CVSequencer(std::vector<int>* values, int clockdivision, PinAnalogOut output, bool randomize_seq);
		CVSequencer(int min, int max, int clockdivision, PinAnalogOut output);
};

class nw2s::MorphingNoteSequencer : public NoteSequencer
{
	public:
		static MorphingNoteSequencer* create(std::vector<SequenceNote>* notes, NoteName key, ScaleType scale, int chaos, int clockdivision, PinAnalogOut output);
		virtual void reset();
		
	private:
		int chaos;

		MorphingNoteSequencer(std::vector<SequenceNote>* notes, NoteName key, ScaleType scale, int chaos, int clockdivision, PinAnalogOut output);
		//static NoteSequencer* create(std::vector<SequenceNote>* notes, NoteName key, ScaleType scale, int clockdivision, PinAnalogOut output, bool randomize_seq = false);
};

#endif

