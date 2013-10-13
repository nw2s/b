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

ADSR* ADSR::create(unsigned int a, unsigned int d, unsigned int s, unsigned int r, bool repeat, PinAnalogOut pin)
{
	return new ADSR(a, d, s, r, repeat, pin);
}

ADSR::ADSR(unsigned int a, unsigned int d, unsigned int s, unsigned int r, bool repeat, PinAnalogOut pin)
{
	this->a = a;
	this->d = d;
	this->s = s;
	this->r = r;
	this->repeat = repeat;

	/* Start the ADSR at 0 */
	this->output = AnalogOut::create(pin);
	this->output->outputCV(0);
}

void ADSR::timer(unsigned int t)
{
	
}

void ADSR::reset()
{
	
}

