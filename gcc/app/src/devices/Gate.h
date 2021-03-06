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

#ifndef Gate_h
#define Gate_h

#include "IO.h"
#include "EventManager.h"

namespace nw2s
{		
	class Gate;

}

class nw2s::Gate : public nw2s::TimeBasedDevice
{
	public:
		static Gate* create(PinDigitalOut pin, unsigned int duration);
		virtual void timer(unsigned long t);
		void reset();
		void updateDuration();

	private:
		PinDigitalOut pin;
		unsigned int duration;
		unsigned long last_clock_t;
		bool state;

		Gate(PinDigitalOut pin, unsigned int duration);
};


#endif
