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

#ifndef Triggerk_h
#define Trigger_h

#include "IO.h"
#include "EventManager.h"

namespace nw2s
{		
	class Trigger;

	//class TimeBasedDevice;
	
	const int TRIGGER_TIME = 20;
}

class nw2s::Trigger : public nw2s::TimeBasedDevice
{
	public:
		static Trigger* create(PinDigitalOut output);
		virtual void timer(unsigned long t);
		virtual void reset();

	private:
		volatile int state;
		unsigned long t_start;
		PinDigitalOut output;
		
		Trigger(PinDigitalOut output);		
};

#endif

