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

BeatDevice::BeatDevice()
{
	this->clock_division = DIV_QUARTER;
}

int BeatDevice::getclockdivision()
{
	return this->clock_division;
}

FixedClock* FixedClock::create(int tempo, unsigned char beats_per_measure)
{	
	return new FixedClock(tempo, beats_per_measure);
}

RandomDropoutClock* RandomDropoutClock::create(int tempo, unsigned char beats_per_measure, int chaos)
{	
	return new RandomDropoutClock(tempo, beats_per_measure, chaos);
}

VariableClock* VariableClock::create(int mintempo, int maxtempo, PinAnalogIn input, unsigned char beats_per_measure)
{	
	return new VariableClock(mintempo, maxtempo, input, beats_per_measure);
}

RandomTempoClock* RandomTempoClock::create(int mintempo, int maxtempo, unsigned char beats_per_measure)
{	
	return new RandomTempoClock(mintempo, maxtempo, beats_per_measure);
}

SlaveClock* SlaveClock::create(PinDigitalIn input, unsigned char beats_per_measure)
{	
	return new SlaveClock(input, beats_per_measure);
}

void Clock::registerdevice(BeatDevice* device)
{
	this->devices.push_back(device);	
}

Clock::Clock()
{
	IOUtils::displayBeat(0, this);

	this->swingpercentage = 0;
	this->swingdivision = 1000;
}

void Clock::setSwing(int swingdivision, int swingpercentage)
{
	this->swingpercentage = (swingpercentage < -100) ? -100 : (swingpercentage > 100) ? 100 : swingpercentage;
	this->swingdivision = swingdivision;
}

FixedClock::FixedClock(int tempo, unsigned char beats_per_measure)
{
	/* The fixed clock operates on a regular period based on the tempo */
	int normalized_tempo = (tempo < 1) ? 1 : (tempo > 500) ? 500 : tempo;
		
	this->beat = 0;
	this->period = 60000 / normalized_tempo;
	this->beats_per_measure = beats_per_measure;
}

void FixedClock::timer(unsigned long t)
{
	if (t % this->period == 0)
	{
		IOUtils::displayBeat(this->beat, this);				
		this->beat = (this->beat + 1) % this->beats_per_measure;		
	}
		
	for (int i = 0; i < this->devices.size(); i++)
	{	
		if ((this->swingpercentage == 0) || (this->devices[i]->getclockdivision() > this->swingdivision))
		{	
			/* Either the swing percentage is 0 or the clock division is greater than the swing division */
			if (t % (((unsigned long)this->devices[i]->getclockdivision() * (unsigned long)this->period) / 1000UL) == 0)
			{
				Serial.println("straight beat " + String(t));
				Serial.println("period " + String(this->period));
				Serial.println("clock div " + String(this->devices[i]->getclockdivision()));
				this->devices[i]->reset();
			}
		}
		else
		{
			//TODO: Optimize a little
			
			/* There is a swingpercentage, and the clock division is smaller than the swing division */			

			/* Calculate all the times for swinging */
			int swingfactor = (this->swingpercentage == 0) ? 1000 : 1000 - ((333 * this->swingpercentage) / 100);
			int swingfactor2 = (this->swingpercentage == 0) ? 1000 : 1000 + ((333 * this->swingpercentage) / 100);
			int t_measure = t % this->period;

			if (t % (this->swingdivision * 2) < this->swingdivision)
			{
				/* We're in the front half of the swing */				
				int t_swung = (t_measure * swingfactor) / 1000;

				if (t_swung % (((unsigned long)this->devices[i]->getclockdivision() * (unsigned long)this->period) / 1000UL) == 0)
				{
					Serial.println("swung beat " + String(t_swung));					
					Serial.println("woulda been " + String(t_measure));					
					this->devices[i]->reset();
				}				
			}
			else
			{
				/* We're in the back half of the swing */
				int t_firsthalf = ((this->swingdivision * swingfactor) / 1000);
				int t_swung2 = t_firsthalf + ((t_measure * swingfactor2) / 1000);

				if (t_swung2 % (((unsigned long)this->devices[i]->getclockdivision() * (unsigned long)this->period) / 1000UL) == 0)
				{
					Serial.println("swung back beat " + String(t_swung2));					
					Serial.println("woulda been " + String(t_measure));					
					this->devices[i]->reset();
				}				 
			}
		}
		
		this->devices[i]->timer(t);
	}	
}

VariableClock::VariableClock(int mintempo, int maxtempo, PinAnalogIn input, unsigned char beats_per_measure)
{
	this->beat = 0;
	this->input = input;
	this->beats_per_measure = beats_per_measure;
	
	/* The variable clock operates on a period based on an input voltage */
	this->mintempo = (mintempo < 1) ? 1 : (mintempo > 500) ? 500 : mintempo;
	this->maxtempo = (maxtempo < 1) ? 1 : (maxtempo > 500) ? 500 : maxtempo;
	
	this->last_clock_t = 0;
}

void VariableClock::timer(unsigned long t)
{
	if (this->last_clock_t == 0)
	{
		IOUtils::displayBeat(this->beat, this);				
		this->beat = (this->beat + 1) % this->beats_per_measure;		

		this->update_tempo(t);
	}
	
	if (t >= this->next_clock_t)
	{
		IOUtils::displayBeat(this->beat, this);				
		this->beat = (this->beat + 1) % this->beats_per_measure;		

		this->update_tempo(t);
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

RandomTempoClock::RandomTempoClock(int mintempo, int maxtempo, unsigned char beats_per_measure)
{
	this->beat = 0;
	this->beats_per_measure = beats_per_measure;
	
	/* The random clock operates on a period based on randomly changing value */
	this->mintempo = (mintempo < 1) ? 1 : (mintempo > 500) ? 500 : mintempo;
	this->maxtempo = (maxtempo < 1) ? 1 : (maxtempo > 500) ? 500 : maxtempo;
	
	this->last_clock_t = 0;
}


void RandomTempoClock::timer(unsigned long t)
{
	if (this->last_clock_t == 0)
	{
		IOUtils::displayBeat(this->beat, this);				
		this->beat = (this->beat + 1) % this->beats_per_measure;		

		this->update_tempo(t);
	}
	
	if (t >= this->next_clock_t)
	{
		IOUtils::displayBeat(this->beat, this);				
		this->beat = (this->beat + 1) % this->beats_per_measure;		

		this->update_tempo(t);
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

void RandomTempoClock::update_tempo(unsigned long t)
{
	int tempo = random(mintempo, maxtempo);
 	this->period = 60000UL / tempo;

	this->next_clock_t = (t + this->period);
	this->last_clock_t = t;
	
	Serial.print("\ntempo: " + String(tempo));
}

volatile bool SlaveClock::trigger = false;
volatile int SlaveClock::period = 0;
volatile unsigned long SlaveClock::t = 0;
volatile unsigned long SlaveClock::last_clock_t = 0;
volatile unsigned long SlaveClock::next_clock_t = 0;
PinDigitalIn SlaveClock::input = DIGITAL_IN_NONE;


SlaveClock::SlaveClock(PinDigitalIn input, unsigned char beats_per_measure)
{
	SlaveClock::input = input;
	SlaveClock::beats_per_measure = beats_per_measure;
	
#ifdef __AVR__
	attachInterrupt(0, SlaveClock::isr, RISING);
#else
	attachInterrupt(input, SlaveClock::isr, RISING);
#endif	
}

void SlaveClock::isr()
{
	SlaveClock::trigger = true;
}

void SlaveClock::timer(unsigned long t)
{	
	SlaveClock::t = t;
	
	if (SlaveClock::trigger)
	{
		if (SlaveClock::period == 0)
		{
			if (SlaveClock::last_clock_t == 0)
			{
				SlaveClock::last_clock_t = t;
			}
			else
			{
				SlaveClock::period = t - last_clock_t;
				SlaveClock::last_clock_t = t;
			}
		}
		else
		{
			//TODO: Update the clock display on Due based boards
			SlaveClock::period = SlaveClock::t - SlaveClock::last_clock_t;
			SlaveClock::next_clock_t = (t + this->period);
			SlaveClock::last_clock_t = t;
		}

		/* Reset */
		trigger = false;		
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

RandomDropoutClock::RandomDropoutClock(int tempo, unsigned char beats_per_measure, int chaos) : FixedClock(tempo, beats_per_measure)
{
	this->chaos = chaos;
}

void RandomDropoutClock::timer(unsigned long t)
{
	if (t % this->period == 0)
	{
		IOUtils::displayBeat(this->beat, this);				
		this->beat = (this->beat + 1) % this->beats_per_measure;		
	}

	for (int i = 0; i < this->devices.size(); i++)
	{		
		if (t % (((unsigned long)this->devices[i]->getclockdivision() * (unsigned long)this->period) / 1000UL) == 0)
		{
			/* This clock occasionally forgets to call reset() on its devices */
			if (this->chaos <= random(100))
			{
				this->devices[i]->reset();
			}			
		}
		
		this->devices[i]->timer(t);
	}	
}



