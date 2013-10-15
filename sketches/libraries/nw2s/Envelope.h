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
#ifdef __AVR__
	static const int CV_MAX = 254;
#else
	static const int CV_MAX = 4096;
#endif

	static const unsigned char MIN_ATTACK = 5;
	
	class Envelope;
	class ADSR;
}

class nw2s::Envelope
{
	public:
		virtual void timer(unsigned long t) = 0;
		virtual void reset(unsigned long t) = 0;
	};

class nw2s::ADSR : public Envelope
{
	public:
		static ADSR* create(unsigned int a, unsigned int d, unsigned int s, unsigned int r, unsigned int gate, bool repeat, PinAnalogOut pin);
		virtual void timer(unsigned long t);
		virtual void reset(unsigned long t);
		
	private:
		unsigned long t_start;
		unsigned int a;
		unsigned int d;
		unsigned int s;
		unsigned int r;
		unsigned int gate;
		unsigned int t_a;
		unsigned int t_d;
		unsigned int t_s;
		unsigned int t_r;
		bool repeat;
		AnalogOut* output;
		
		ADSR(unsigned int a, unsigned int d, unsigned int s, unsigned int r, unsigned int gate, bool repeat, PinAnalogOut pin);
	};


#endif