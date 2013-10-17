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


#include "Trigger.h"
#include "IO.h"

using namespace nw2s;

Trigger* Trigger::create(PinDigitalOut output)
{	
	return new Trigger(output);
}

Trigger::Trigger(PinDigitalOut output)
{
	/* Make sure the pin is low */
	this->output = output;
	this->state = LOW;
	this->t_start = 0;
	digitalWrite(this->output, LOW);		
}

void Trigger::reset()
{
	/* Reset turns the trigger on */
	this->t_start = 0;
	this->state = HIGH;
	digitalWrite(this->output, HIGH);		
}

void Trigger::timer(unsigned long t)
{
	/* If the state is low, nothing else to do */
	if (this->state == LOW) return;
	
	if (this->t_start == 0) this->t_start = t;
	
	unsigned int t_trig = t - t_start;
	
	if (t_trig > TRIGGER_TIME)
	{
		this->state = LOW;
		digitalWrite(this->output, LOW);
	}	
}



