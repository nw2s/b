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

#ifndef ShiftRegister_h
#define ShiftRegister_h

#include "IO.h"
#include "Clock.h"
#include "Trigger.h"
#include "Gate.h"
#include "../aJSON/aJSON.h"

namespace nw2s
{
	class RandomLoopingShiftRegister;
}

class nw2s::RandomLoopingShiftRegister : public nw2s::BeatDevice
{
	public:
		static RandomLoopingShiftRegister* create(int size, PinAnalogIn control, int clockdivision);
		static RandomLoopingShiftRegister* create(aJsonObject* data);
		virtual void calculate();
		virtual void timer(unsigned long t);
		virtual void reset();
		void setCVOut(PinAnalogOut pinout);
		void setDelayedCVOut(PinAnalogOut pinout, int ticks);
		void setKey(NoteName root, Scale scale);
		void setNoteOut(PinAnalogOut pinout);
		void setDelayedNoteOut(PinAnalogOut pinout, int ticks);
		void setTriggerOut(int position, PinDigitalOut pinout);
		void setGateOut(int position, PinDigitalOut pinout, int duration);
		void setLogicalOrTrigger(PinDigitalOut pinout, int p1, int p2, int p3 = -1, int p4 = -1);
		void setLogicalAndTrigger(PinDigitalOut pinout, int p1, int p2, int p3 = -1, int p4 = -1);
		void setLogicalOrGate(PinDigitalOut pinout, int duration, int p1, int p2, int p3 = -1, int p4 = -1);
		void setLogicalAndGate(PinDigitalOut pinout, int duration, int p1, int p2, int p3 = -1, int p4 = -1);
		void setSequencerInputs(PinAnalogIn p1, PinAnalogIn p2, PinAnalogIn p3, PinAnalogIn p4, PinAnalogIn p5, PinAnalogIn p6, PinAnalogIn p7, PinAnalogIn p8);
		void setSequencerScaleInput(PinAnalogIn pin);
		void setSequencerCVOut(PinAnalogOut pinout);
		void setSequencerNoteOut(PinAnalogOut pinout);
		// void setWriteZero(PinDigitalIn pinin);
		// void setWriteOne(PinDigitalIn pinin);

	private:
		int nextCV;
		int nextNote;
		bool next_or_gate;
		bool next_and_gate;
		bool next_or_trigger;
		bool next_and_trigger;
		int next_sequencercv;
		int next_sequencernote;
		
		std::vector<bool> shiftregister;
		PinAnalogIn controlpin;
		PinDigitalIn writeone;
		PinDigitalIn writezero;
		AnalogOut* cvout;
		AnalogOut* noteout;
		AnalogOut* delayedcvout;
		AnalogOut* delayednoteout;
		AnalogOut* sequencercvout;
		AnalogOut* sequencernoteout;
		std::vector<int> notedelayline;
		std::vector<int> cvdelayline;
		Key* key;
		Gate* gate[8];
		Trigger* trigger[8];
		Trigger* or_trigger;
		Gate* or_gate;
		Trigger* and_trigger;
		Gate* and_gate;
		vector<int> or_trigger_terms;
		vector<int> and_trigger_terms;
		vector<int> or_gate_terms;
		vector<int> and_gate_terms;
		PinAnalogIn sequencerinput[8];
		PinAnalogIn sequencerscale;

		RandomLoopingShiftRegister(int size, PinAnalogIn control, int clockdivision);
		int getCVfromShiftRegister();
};


#endif