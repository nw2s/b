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

#ifndef DrumTrigger_h
#define DrumTrigger_h

#include "IO.h"

namespace nw2s
{		
	class DrumTrigger;

}

class nw2s::DrumTrigger
{
	public:
		static DrumTrigger* create(PinAnalogOut pin, unsigned int amplitude);
		void timer(unsigned long t);
		void reset();
		void setAmplitude(int amplitude);

	private:
		AnalogOut* output;
		unsigned int amplitude;
		unsigned long last_clock_t;
		bool state;

		DrumTrigger(PinAnalogOut pin, unsigned int amplitude);
};


#endif
