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

namespace nw2s
{
	class RandomLoopingShiftRegister;
}

class nw2s::RandomLoopingShiftRegister : public nw2s::BeatDevice
{
	public:
		static RandomLoopingShiftRegister* create(int size, PinAnalogIn control, int clockdivision);
		
		virtual void timer(unsigned long t);
		virtual void reset();
		void setCVOut(PinAnalogOut pinout);
		// void setDelayedCVOutput(PinAnalogOut pinout, int ticks);
		// void setKey(Key key);
		// void setWriteZero(PinDigitalIn pinin);
		// void setWriteOne(PinDigitalIn pinin);
		// void setNoteOutput(PinAnalogOut pinout);
		// void setDelayedNoteOutput(PinAnalogOut pinout, int ticks);
		void setTriggerOut(int position, PinDigitalOut pinout);
		void setGateOut(int position, PinDigitalOut pinout, int duration);
		// void setLogicalOrTrigger1(PinDigitalOut pinout, int p1, int p2, int p3 = -1, int p4 = -1);
		// void setLogicalOrTrigger2(PinDigitalOut pinout, int p1, int p2, int p3 = -1, int p4 = -1);
		// void setLogicalOrGate1(PinDigitalOut pinout, int p1, int p2, int p3 = -1, int p4 = -1);
		// void setLogicalOrGate2(PinDigitalOut pinout, int p1, int p2, int p3 = -1, int p4 = -1);
		// void setLogicalAndTrigger1(PinDigitalOut pinout, int p1, int p2, int p3 = -1, int p4 = -1);
		// void setLogicalAndTrigger2(PinDigitalOut pinout, int p1, int p2, int p3 = -1, int p4 = -1);
		// void setLogicalAndGate1(PinDigitalOut pinout, int p1, int p2, int p3 = -1, int p4 = -1);
		// void setLogicalAndGate2(PinDigitalOut pinout, int p1, int p2, int p3 = -1, int p4 = -1);
		// void setSequencerInputs(PinAnalogIn p1, PinAnalogIn p2, PinAnalogIn p3, PinAnalogIn p4, PinAnalogIn p5, PinAnalogIn p6, PinAnalogIn p7, PinAnalogIn p8);
		// void setNoteSequencer(PinAnalogOut pinout, PinAnalogIn p1, PinAnalogIn p2, PinAnalogIn p3, PinAnalogIn p4, PinAnalogIn p5, PinAnalogIn p6, PinAnalogIn p7, PinAnalogIn p8);

	private:
		std::vector<bool> shiftregister;
		PinAnalogIn controlpin;
		PinDigitalIn writeone;
		PinDigitalIn writezero;
		AnalogOut* cvout;
		AnalogOut* noteout;
		AnalogOut* delatedcvout;
		AnalogOut* delayednoteout;
		// int notedelay;
		// int cvdelay;
		// Key key;
		Gate* gate[8];
		Trigger* trigger[8];
		// PinDigitalOut or_trigger1;
		// PinDigitalOut or_trigger2;
		// PinDigitalOut and_trigger1;
		// PinDigitalOut and_trigger2;
		// PinDigitalOut or_gate1;
		// PinDigitalOut or_gate2;
		// PinDigitalOut and_gate1;
		// PinDigitalOut and_gate2;
		// int or_trigger1_term1;
		// int or_trigger1_term2;
		// int or_trigger1_term3;
		// int or_trigger1_term4;
		// int or_trigger2_term1;
		// int or_trigger2_term2;
		// int or_trigger2_term3;
		// int or_trigger2_term4;
		// int and_trigger1_term1;
		// int and_trigger1_term2;
		// int and_trigger1_term3;
		// int and_trigger1_term4;
		// int and_trigger2_term1;
		// int and_trigger2_term2;
		// int and_trigger2_term3;
		// int and_trigger2_term4;
		// int or_gate1_term1;
		// int or_gate1_term2;
		// int or_gate1_term3;
		// int or_gate1_term4;
		// int or_gate2_term1;
		// int or_gate2_term2;
		// int or_gate2_term3;
		// int or_gate2_term4;
		// int and_gate1_term1;
		// int and_gate1_term2;
		// int and_gate1_term3;
		// int and_gate1_term4;
		// int and_gate2_term1;
		// int and_gate2_term2;
		// int and_gate2_term3;
		// int and_gate2_term4;
		// 
		// 
		RandomLoopingShiftRegister(int size, PinAnalogIn control, int clockdivision);
		int getCVfromShiftRegister();
};


#endif