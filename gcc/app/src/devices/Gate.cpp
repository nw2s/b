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
#include "Gate.h"

using namespace nw2s;

Gate* Gate::create(PinDigitalOut pin, unsigned int duration)
{
	return new Gate(pin, duration);
}

Gate::Gate(PinDigitalOut pin, unsigned int duration)
{
	this->pin = pin;
	this->duration = duration;
	this->last_clock_t = 0;

	/* Make sure we start with the gate closed */
	this->state = false;
	if (this->pin != DIGITAL_OUT_NONE) digitalWrite(this->pin, LOW);	
}

void Gate::timer(unsigned long t)
{
	if ((this->state == false) && (this->last_clock_t == 0))
	{
		this->state = true;
		if (this->pin != DIGITAL_OUT_NONE) digitalWrite(this->pin, HIGH);
		
		this->last_clock_t = t;
	}
	else if ((this->state == true) && (t - this->last_clock_t >= this->duration))
	{
		this->state = false;
		if (this->pin != DIGITAL_OUT_NONE) digitalWrite(this->pin, LOW);
	}	
}

void Gate::reset()
{
	this->last_clock_t = 0;
	this->state = false;
	if (this->pin != DIGITAL_OUT_NONE) digitalWrite(this->pin, LOW);
}
