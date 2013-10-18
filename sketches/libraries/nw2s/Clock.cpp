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


#include "Clock.h"
#include <Arduino.h>

using namespace std;
using namespace nw2s;

int BeatDevice::getclockdivision()
{
	return this->clock_division;
}

FixedClock* FixedClock::create(int tempo, unsigned char beats_per_measure)
{	
	return new FixedClock(tempo, beats_per_measure);
}

VariableClock* VariableClock::create(int mintempo, int maxtempo, PinAnalogIn input, unsigned char beats_per_measure)
{	
	return new VariableClock(mintempo, maxtempo, input, beats_per_measure);
}

RandomClock* RandomClock::create(int mintempo, int maxtempo, unsigned char beats_per_measure)
{	
	return new RandomClock(mintempo, maxtempo, beats_per_measure);
}

void Clock::registerdevice(BeatDevice* device)
{
	this->devices.push_back(device);	
}

FixedClock::FixedClock(int tempo, unsigned char beats_per_measure)
{
	/* The fixed clock operates on a regular period based on the tempo */
	int normalized_tempo = (tempo < 1) ? 1 : (tempo > 500) ? 500 : tempo;
		
	this->period = 60000 / normalized_tempo;
	this->beats_per_measure = beats_per_measure;
}

void FixedClock::timer(unsigned long t)
{
	if (t % this->period == 0)
	{
		//TODO: Update the clock display on Due based boards
	}
	
	for (int i = 0; i < this->devices.size(); i++)
	{
		if (t % (((unsigned long)this->devices[i]->getclockdivision() * (unsigned long)this->period) / 1000UL) == 0)
		{
			this->devices[i]->reset();
		}
		
		this->devices[i]->timer(t);
	}	
}

VariableClock::VariableClock(int mintempo, int maxtempo, PinAnalogIn input, unsigned char beats_per_measure)
{
	this->input = input;
	this->beats_per_measure = beats_per_measure;
	
	/* The variable clock operates on a period based on an input voltage */
	int normalized_mintempo = (mintempo < 1) ? 1 : (mintempo > 500) ? 500 : mintempo;
	int normalized_maxtempo = (maxtempo < 1) ? 1 : (maxtempo > 500) ? 500 : maxtempo;
	
	this->mintempo = normalized_mintempo;
	this->maxtempo = normalized_maxtempo;
	
	this->last_clock_t = 0;
}

void VariableClock::timer(unsigned long t)
{
	if (this->last_clock_t == 0)
	{
		this->update_tempo(t);
	}
	
	if (t >= this->next_clock_t)
	{
		this->update_tempo(t);
		//TODO: Update the clock display on Due based boards
	}
	
	for (int i = 0; i < this->devices.size(); i++)
	{
		if (t % (((unsigned long)this->devices[i]->getclockdivision() * (unsigned long)this->period) / 1000UL) == 0)
		{
			this->devices[i]->reset();
		}
		
		this->devices[i]->timer(t);
	}	
}

void VariableClock::update_tempo(unsigned long t)
{
	int tempo = ((((unsigned long)this->maxtempo - (unsigned long)this->mintempo) * ((unsigned long)analogRead(this->input)) / 1023UL)) + this->mintempo;
 	this->period = 60000UL / tempo;

	this->next_clock_t = (t + this->period);
	this->last_clock_t = t;
}

RandomClock::RandomClock(int mintempo, int maxtempo, unsigned char beats_per_measure)
{
	/* Make sure the pin is low, initialize variables */
	this->beats_per_measure = beats_per_measure;
	this->last_clock_t = 0;
	this->next_clock_t = 0;
	
	/* The random clock operates on a random period based on an input voltage */
	this->mintempo = (mintempo < 1) ? 1 : (mintempo > 500) ? 500 : mintempo;
	this->maxtempo = (maxtempo < 1) ? 1 : (maxtempo > 500) ? 500 : maxtempo;	
}

void RandomClock::timer(unsigned long t)
{	
	//TODO: calculate next_t
}

void RandomClock::next_t(unsigned long t)
{
	this->next_clock_t = (this->last_clock_t + (60000 / random(mintempo, maxtempo)));	
}



