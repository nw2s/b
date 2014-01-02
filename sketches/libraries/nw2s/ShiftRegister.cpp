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

#include "ShiftRegister.h"
#include "Entropy.h"


//using namepsace nw2s;

nw2s::RandomLoopingShiftRegister* RandomLoopingShiftRegister::create(int size, PinAnalogIn control, int clockdivision)
{
	return new RandomLoopingShiftRegister(size, control, clockdivision);
}

RandomLoopingShiftRegister::RandomLoopingShiftRegister(int size, PinAnalogIn control, int clockdivision)
{
	this->controlpin = control;
	this->clock_division = clockdivision;

	/* Fill up the shiftregister with random bits */
	for (int i = 0; i < size; i++)
	{
		shiftregister.push_back(Entropy::getBit());
	}
	
	this->cvout = NULL;
	this->noteout = NULL;
	
	for (int i = 0; i < 8; i++) 
	{
		this->trigger[i] = NULL;
		this->gate[i] = NULL;
	}
}

void RandomLoopingShiftRegister::setCVOut(PinAnalogOut pinout)
{
	this->cvout = AnalogOut::create(pinout);
}

void RandomLoopingShiftRegister::setDelayedCVOut(PinAnalogOut pinout, int ticks)
{
	this->delayedcvout = AnalogOut::create(pinout);
	
	for (int i = 0; i < ticks; i++) this->cvdelayline.push_back(0);
}

void RandomLoopingShiftRegister::setTriggerOut(int position, PinDigitalOut pinout)
{
	this->trigger[position - 1] = Trigger::create(pinout, 0);
}

void RandomLoopingShiftRegister::setGateOut(int position, PinDigitalOut pinout, int duration)
{
	this->gate[position - 1] = Gate::create(pinout, duration);
}

void RandomLoopingShiftRegister::timer(unsigned long t)
{
	//TODO: Set up slews

	for (int i = 0; i < 8; i++)
	{
		if (trigger[i] != NULL) trigger[i]->timer(t);
		if (gate[i] != NULL) gate[i]->timer(t);
	}
}

void RandomLoopingShiftRegister::reset()
{
	/* Shift a bit from the back to the front, randomly negating it */
	int rawval = analogRead(this->controlpin);
	int factor = 50;
	
	/* Scale the factor based on 0-50 and 50-100 */
	if ((rawval < 1700) || (rawval > 1800))
	{
		factor = (rawval * 50) / 1700;
	}

	/* Decide if we flip the bit as we rotate it, and do the rotation */
	bool negate = (factor == 0) ? false : (factor == 100) ? true : (Entropy::getValue(100) <= factor) ? true : false;
	bool newfront = this->shiftregister.back() ^ negate;

	/* Shift all of the bits back by one */
	for (int i = this->shiftregister.size() - 1; i > 0; i--)
	{
		this->shiftregister[i] = this->shiftregister[i - 1];
	}

	/* And put the new one in front */
	this->shiftregister[0] = newfront;

	/* Calculate the CV of the current state */
	int cv = getCVfromShiftRegister();
	
	/* CV Output */
	if (this->cvout != NULL) this->cvout->outputCV(cv);
	
	for (int i = 0; i < 8; i++)
	{
		if ((this->trigger[i] != NULL) && this->shiftregister[i]) this->trigger[i]->reset();
		if ((this->gate[i] != NULL) && this->shiftregister[i]) this->gate[i]->reset();
	}
	
	/* Delay Lines */
	if (this->cvdelayline.size() > 0)
	{
		for (int i = this->cvdelayline.size() - 1; i > 0; i--)
		{
			this->cvdelayline[i] = this->cvdelayline[i - 1];
		}
		
		this->cvdelayline[0] = cv;
		
		this->delayedcvout->outputCV(this->cvdelayline.back());
	}
}

int RandomLoopingShiftRegister::getCVfromShiftRegister()
{
	int val = 0;
	
	for (int i = 0; i < 12; i++)
	{
		if (i < this->shiftregister.size())
		{
			val = val << 1;
			val |= this->shiftregister[i];
		}
		else
		{
			val = val << 1;
		}
	}
	
	return val;
}

