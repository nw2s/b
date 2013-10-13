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

	/* Make sure we start with the gate closed */
	this->state = false;
	digitalWrite(this->pin, LOW);	
}

void Gate::timer(unsigned int t)
{
	if ((this->state == false) && (t < this->duration))
	{
		this->state = true;
		digitalWrite(this->pin, HIGH);
	}
	else if ((this->state == true) && (t >= duration))
	{
		this->state = false;
		digitalWrite(this->pin, LOW);
	}	
}
