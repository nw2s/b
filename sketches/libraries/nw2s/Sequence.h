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
#include "DrumTrigger.h"

namespace nw2s
{
	struct SequenceNote
	{
		int octave;
		int degree;
	};
		
	typedef std::vector<SequenceNote> NoteSequenceData;
	typedef std::vector<int> TriggerSequenceData;
	typedef std::vector<int> CVSequenceData;

	class Sequencer;
	class NoteSequencer;
	class CVNoteSequencer;
	class CVSequencer;
	class MorphingNoteSequencer;
	class TriggerSequencer;
	class DrumTriggerSequencer;
	class ProbabilityTriggerSequencer;
	class ProbabilityDrumTriggerSequencer;
	
	class TimeBasedDevice;
	
	NoteSequenceData* noteSequenceFromJSON(aJsonObject* data);
}

class nw2s::Sequencer : public nw2s::BeatDevice
{
	public: 
		void setgate(Gate* gate);
		virtual void timer(unsigned long t) = 0;
		virtual void reset() = 0;
	
	protected:
		Gate* gate;	
		
		Sequencer();
};


class nw2s::TriggerSequencer : public nw2s::Sequencer
{
	public: 
		static TriggerSequencer* create(std::vector<int>* triggers, int clockdivision, PinDigitalOut output);
		static TriggerSequencer* create(aJsonObject* data);		
		virtual void timer(unsigned long t);
		virtual void reset();
		
	protected:
		TriggerSequencer(std::vector<int>* triggers, int clockdivision, PinDigitalOut output);
		Trigger* trigger;
		std::vector<int>* triggers;
		volatile int sequence_index;
		volatile bool state;
};

class nw2s::DrumTriggerSequencer : public nw2s::Sequencer
{
	public: 
		static DrumTriggerSequencer* create(std::vector<int>* triggers, int clockdivision, PinAnalogOut output);
		virtual void timer(unsigned long t);
		virtual void reset();
		
	protected:
		DrumTriggerSequencer(std::vector<int>* triggers, int clockdivision, PinAnalogOut output);
		DrumTrigger* trigger;
		std::vector<int>* triggers;
		volatile int sequence_index;
		volatile bool state;
};

class nw2s::ProbabilityDrumTriggerSequencer : public nw2s::Sequencer
{
	public: 
		static ProbabilityDrumTriggerSequencer* create(std::vector<int>* triggers, std::vector<int>* velocities, int velocityrange, int clockdivision, PinAnalogOut output);
		virtual void timer(unsigned long t);
		virtual void reset();
		virtual void calculate();
		void setProbabilityModifier(PinAnalogIn pin);
		void setVelocityModifier(PinAnalogIn pin);
		
	private:
		ProbabilityDrumTriggerSequencer(std::vector<int>* triggers, std::vector<int>* velocities, int velocityrange, int clockdivision, PinAnalogOut output);
		DrumTrigger* trigger;
		std::vector<int>* triggers;
		std::vector<int>* velocities;
		PinAnalogIn velocitymodifierpin;
		PinAnalogIn probabilitymodifierpin;
		int velocityrange;
		int nextvelocity;
		volatile int sequence_index;
		volatile bool state;
		bool resetnext;
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
		static NoteSequencer* create(NoteSequenceData* notes, NoteName key, Scale scale, int clockdivision, PinAnalogOut output, bool randomize_seq = false);
		static NoteSequencer* create(aJsonObject* data);
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
		NoteSequenceData* notes;
		
		NoteSequencer(NoteSequenceData* notes, NoteName key, Scale scale, int clockdivision, PinAnalogOut output, bool randomize_seq);
};

class nw2s::CVNoteSequencer : public nw2s::Sequencer
{
	public:
		static CVNoteSequencer* create(NoteSequenceData* notes, NoteName key, Scale scale, PinAnalogOut output, PinAnalogIn input, bool randomize_seq = false);
		static CVNoteSequencer* create(aJsonObject* data);		
		virtual void timer(unsigned long t);
		virtual void reset();
	
	private:
		bool randomize_seq;
		volatile int sequence_index;
		std::vector<SequenceNote>* notes;
		Key* key;
	 	AnalogOut* output;
		PinAnalogIn cv_in;
		unsigned long last_note_t;
		int calculatePosition();
		
		CVNoteSequencer(NoteSequenceData* notes, NoteName key, Scale scale, PinAnalogOut output, PinAnalogIn input, bool randomize_seq);
};

class  nw2s::CVSequencer : public Sequencer
{
	public:
		static CVSequencer* create(CVSequenceData* values, int clockdivision, PinAnalogOut output, bool randomize_seq);
		static CVSequencer* create(int clockdivision, PinAnalogOut output);
		static CVSequencer* create(int min, int max, int clockdivision, PinAnalogOut output);
		static CVSequencer* create(aJsonObject* data);
		
		virtual void timer(unsigned long t);
		virtual void reset();

	private:
		bool randomize_seq;
		int min;
		int max;
		volatile int sequence_index;
		volatile int current_value;
		CVSequenceData* values;
	 	AnalogOut* output;
	
		CVSequencer(CVSequenceData* values, int clockdivision, PinAnalogOut output, bool randomize_seq);
		CVSequencer(int min, int max, int clockdivision, PinAnalogOut output);
};

class nw2s::MorphingNoteSequencer : public NoteSequencer
{
	public:
		static MorphingNoteSequencer* create(NoteSequenceData* notes, NoteName key, Scale scale, int chaos, int clockdivision, PinAnalogOut output, PinDigitalIn reset);
		static MorphingNoteSequencer* create(aJsonObject* data);
		virtual void reset();
		
	private:
		int chaos;
		NoteSequenceData* notesOriginal;
		PinDigitalIn resetPin;

		MorphingNoteSequencer(NoteSequenceData* notes, NoteName key, Scale scale, int chaos, int clockdivision, PinAnalogOut output, PinDigitalIn reset);
};

#endif

