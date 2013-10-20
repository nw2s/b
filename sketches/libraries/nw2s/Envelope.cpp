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

#include "Envelope.h"

using namespace nw2s;

//TODO: Scalable and Shiftable output

ADSR* ADSR::create(unsigned int a, unsigned int d, unsigned int s, unsigned int r, unsigned int gate, bool repeat, PinAnalogOut pin)
{
	return new ADSR(a, d, s, r, gate, repeat, pin);
}

ADSR::ADSR(unsigned int a, unsigned int d, unsigned int s, unsigned int r, unsigned int gate, bool repeat, PinAnalogOut pin)
{
	/* Normalize the input values */
	this->a = a < MIN_ATTACK ? MIN_ATTACK : a;
	this->d = d < MIN_ATTACK ? MIN_ATTACK : d;
	this->s = s > CV_MAX ? CV_MAX : s;
	this->r = r < MIN_ATTACK ? MIN_ATTACK : r;
	this->repeat = repeat;
	this->t_start = 0;
	this->gate = gate;
	
	/* Calculate time of each envelope */
	this->t_a = a;
	this->t_d = (long)a - (((long)d * ((long)s - CV_MAX)) / CV_MAX);	
	
	/* If the decay time is already past how long the gate is open, skip the S and R phases */
	if (this->t_d >= gate)
	{
		this->t_d = a + d;		
		this->t_s = 0;
		this->t_r = 0;
	}
	else
	{
		this->t_s = gate;
		this->t_r = this->gate + this->r;		
	}
	
	/* Start the ADSR at 0 */
	this->output = AnalogOut::create(pin);
	this->output->outputCV(0);
	
}

void ADSR::timer(unsigned long t)
{
	if (this->t_start == 0) this->t_start = t;

	unsigned int t_env = t - this->t_start;
		
	/* Calculate state based on time */
	if (t_env <= this->t_a)
	{
		this->output->outputCV((CV_MAX * t_env) / this->a);
	}
	else if (t_env <= t_d)
	{
		this->output->outputCV(CV_MAX - ((CV_MAX * (t_env - this->a)) / this->d));
	}
	else if (t_env <= t_s)
	{
		this->output->outputCV(s);
	}
	else if (t_env <= t_r)
	{
		this->output->outputCV((int)this->s - (((long)this->s * ((long)t_env - (long)this->gate)) / (long)this->r));
	}
	else if (repeat)
	{
		this->reset();
	}
}

void ADSR::reset()
{
	this->t_start = 0;
	this->output->outputCV(0);
}

