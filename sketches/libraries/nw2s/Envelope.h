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

#ifndef Envelope_h
#define Envelope_h

#include "IO.h"

namespace nw2s
{
	enum EnvelopeState
	{
		ENVELOPE_STATE_A = 0,
		ENVELOPE_STATE_D = 1,
		ENVELOPE_STATE_S = 2,
		ENVELOPE_STATE_R = 3,
	};
	
	class Envelope;
	class ADSR;
}

class nw2s::Envelope
{
	public:
		virtual void timer(unsigned int t) = 0;
		virtual void reset() = 0;
	};

class nw2s::ADSR : public Envelope
{
	public:
		static ADSR* create(unsigned int a, unsigned int d, unsigned int s, unsigned int r, bool repeat, PinAnalogOut pin);
		virtual void timer(unsigned int t);
		virtual void reset();
		
	private:
		unsigned int a;
		unsigned int d;
		unsigned int s;
		unsigned int r;
		bool repeat;
		AnalogOut* output;
		EnvelopeState state;
		
		ADSR(unsigned int a, unsigned int d, unsigned int s, unsigned int r, bool repeat, PinAnalogOut pin);
	};


#endif