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
	this->t_d = a + ((d * (s - CV_MAX)) / (-1 * CV_MAX));
	
	/* If the decay time is already past how long the gate is open, skip the S and R phases */
	if (t_d >= gate)
	{
		this->t_d = d;		
		this->t_s = -1;
		this->t_r = -1;
	}
	else
	{
		this->t_s = gate;
		this->t_r = ((gate * CV_MAX) + (r * s)) / CV_MAX;
	}
	
	/* Start the ADSR at 0 */
	this->output = AnalogOut::create(pin);
	this->output->outputCV(0);
	
	// Serial.print("\na: " + String(this->a));
	// Serial.print("\nd: " + String(this->d));
	// Serial.print("\ns: " + String(this->s));
	// Serial.print("\nr: " + String(this->r));
	// Serial.print("\ngate: " + String(this->gate));
	// Serial.print("\nta: " + String(this->t_a));
	// Serial.print("\ntd: " + String(this->t_d));
	// Serial.print("\nts: " + String(this->t_s));
	// Serial.print("\ntr: " + String(this->t_r));
	Serial.print("========");
}

void ADSR::timer(unsigned long t)
{
	if (this->t_start == 0) this->t_start = t;

	unsigned int t_env = t - this->t_start;
		
	/* Calculate state based on time */
	if (t_env <= this->t_a)
	{
		if (t % 1 == 0) Serial.print("\na: " + String(t_env));
		if (t % 1 == 0) Serial.print(" " + String((CV_MAX * t_env) / this->a));
		this->output->outputCV((CV_MAX * t_env) / this->a);
	}
	else if (t_env <= t_d)
	{
		// if (t % 5 == 0) Serial.print("\n" + String(t_env));
		// if (t % 5 == 0) Serial.print(" " + String((CV_MAX * (t_env - this->a)) / this->d));
		this->output->outputCV(CV_MAX - ((CV_MAX * (t_env - this->a)) / this->d));
	}
	else if (t_env <= t_s)
	{
		// if (t % 5 == 0) Serial.print("\n" + String(t_env));
		// if (t % 5 == 0) Serial.print(" " + String(s));
		this->output->outputCV(s);
	}
	else if (t_env <= t_r)
	{
		// if (t % 5 == 0) Serial.print("\n" + String(t_env));
		// if (t % 5 == 0) Serial.print(" " + String((CV_MAX * (t_env - this->gate)) / this->r ));
		this->output->outputCV(s - ( (CV_MAX * (t_env - this->gate)) / this->r ));
	}
	else if (repeat)
	{
		this->reset(t);
	}
}

void ADSR::reset(unsigned long t)
{
	this->t_start = 0;
	this->output->outputCV(0);
}

