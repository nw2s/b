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

#include "Slew.h"
#include "Arduino.h"

using namespace nw2s;

DecaySlew* DecaySlew::create(float timeconstant)
{
	return new DecaySlew(timeconstant);
}

LinearSlew* LinearSlew::create(int speed)
{
	return new LinearSlew(speed);
}


DecaySlew::DecaySlew(float timeconstant)
{
	/* Normalize input to 0.1 < timeconstant < 0.99 */
	this->timeconstant = (timeconstant < 0.1) ? 0.1 : ((timeconstant > 0.99) ? 0.99 : timeconstant);
	this->initialized = false;
}

CVTYPE DecaySlew::calculate_value(CVTYPE input)
{	
	if (!initialized)
	{
		this->lastvalue = input; 
		this->initialized = true;
	}

	/* Don't get caught on a rounding error */
	if ((this->lastvalue >= input - 0.1) && (this->lastvalue <= input + 0.1))
	{
		this->lastvalue = input;
		return input;
	}
	
	this->lastvalue = input + ((this->lastvalue - (1.0 * input)) * timeconstant);
		
	return (int)this->lastvalue;
}

LinearSlew::LinearSlew(int speed)
{
	/* Normalize input to 1 < speed < 100 */
	this->speed = (speed < 1) ? 1 : ((speed > 100) ? 100 : speed);
	this->initialized = false;
}

CVTYPE LinearSlew::calculate_value(CVTYPE input)
{	
	if (!initialized)
	{
		this->lastvalue_scaled = input * 100; 
		this->lastvalue = input; 
		initialized = true;
	}

	if (this->lastvalue == input)
	{
		return input;
	}

	int factor = (input < this->lastvalue) ? -1L : 1L;
	
	this->lastvalue_scaled = (this->lastvalue_scaled) + (factor * (long)this->speed);
	this->lastvalue = this->lastvalue_scaled / 100;

	return this->lastvalue;
}









