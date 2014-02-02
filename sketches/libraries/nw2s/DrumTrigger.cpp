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

#include <Arduino.h>
#include "DrumTrigger.h"

using namespace nw2s;

static const int DRUM_TRIGGER_DURATION = 25;

DrumTrigger* DrumTrigger::create(PinAnalogOut pin, unsigned int amplitude)
{
	return new DrumTrigger(pin, amplitude);
}

DrumTrigger::DrumTrigger(PinAnalogOut pin, unsigned int amplitude)
{
	this->output = AnalogOut::create(pin);
	this->amplitude = amplitude;
	this->last_clock_t = 0;

	/* Make sure we start with the trigger closed */
	this->state = false;
	output->outputCV(0);	
}

void DrumTrigger::timer(unsigned long t)
{
	if (this->last_clock_t == 0)
	{
		this->last_clock_t = t;
	}
	
	if ((this->state == true) && (t - this->last_clock_t >= DRUM_TRIGGER_DURATION))
	{
		this->state = false;
		output->outputCV(0);
	}	
}

void DrumTrigger::reset()
{
	this->state = true;
	output->outputCV(amplitude);
	
	this->last_clock_t = 0;
}
