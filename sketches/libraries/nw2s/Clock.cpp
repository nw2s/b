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

SlaveClock* SlaveClock::create(PinDigitalIn input, unsigned char beats_per_measure)
{	
	return new SlaveClock(input, beats_per_measure);
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


int SlaveClock::period_samples[5] = { 0, 0, 0, 0, 0};
volatile int SlaveClock::period = 0;
volatile unsigned long SlaveClock::t = 0;
volatile unsigned long SlaveClock::last_clock_t = 0;
volatile unsigned long SlaveClock::next_clock_t = 0;
unsigned char SlaveClock::period_sample_state = 0;
PinDigitalIn SlaveClock::input = DIGITAL_IN_NONE;


SlaveClock::SlaveClock(PinDigitalIn input, unsigned char beats_per_measure)
{
	SlaveClock::input = input;
	SlaveClock::beats_per_measure = beats_per_measure;
	SlaveClock::last_clock_t = 0;
	SlaveClock::next_clock_t = 0;
	SlaveClock::period_sample_state = 0;
	
#ifdef __AVR__
	attachInterrupt(0, SlaveClock::isr, RISING);
#else
	attachInterrupt(input, SlaveClock::isr, RISING);
#endif	
}

void SlaveClock::isr()
{
	if ((SlaveClock::period_samples[0] == 0) && (SlaveClock::last_clock_t == 0))
	{
		Serial.print("\nfirst...");
		
		/* Nothing's initialized */
		SlaveClock::last_clock_t = SlaveClock::t;
	}
	else if (SlaveClock::period_samples[0] == 0)
	{
		Serial.print("\nsecond...");

		/* This is the second rising state, so set up the period samples */
		SlaveClock::period = SlaveClock::t - SlaveClock::last_clock_t; 
		
		SlaveClock::period_samples[0] = SlaveClock::period;
		SlaveClock::period_samples[1] = SlaveClock::period;
		SlaveClock::period_samples[2] = SlaveClock::period;
		SlaveClock::period_samples[3] = SlaveClock::period;
		SlaveClock::period_samples[4] = SlaveClock::period;

		/* Force events to fire on the rising edge */
		SlaveClock::next_clock_t = 0;
	}
	else
	{
		Serial.print("\n" + SlaveClock::t);

		/* This is steady state, just update the clock periods */
		SlaveClock::period_samples[SlaveClock::period_sample_state] = SlaveClock::t - SlaveClock::last_clock_t;
		
		unsigned int periodsum = SlaveClock::period_samples[0] + SlaveClock::period_samples[1] + SlaveClock::period_samples[2] + SlaveClock::period_samples[3] + SlaveClock::period_samples[4];
		SlaveClock::period = periodsum / 5;

		SlaveClock::last_clock_t = 0;
	}
}

void SlaveClock::timer(unsigned long t)
{	
	SlaveClock::t = t;
	
	if (SlaveClock::period != 0)
	{
		if (SlaveClock::last_clock_t == 0)
		{
			this->update_tempo(t);
		}
	
		if (t >= SlaveClock::next_clock_t)
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
}

void SlaveClock::update_tempo(unsigned long t)
{
	SlaveClock::next_clock_t = (t + this->period);
	SlaveClock::last_clock_t = t;
}



