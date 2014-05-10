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
	this->next_time = 0;
}

int BeatDevice::getclockdivision()
{
	return this->clock_division;
}

void BeatDevice::setNextTime(unsigned long t)
{
	this->next_time = t;
}

unsigned long BeatDevice::getNextTime()
{
	return this->next_time;
}

void BeatDevice::calculate()
{
	/* 
	   Some devices will need more complicated calculations that can take some time. In those cases, simply override 
	   this method and do the work in it, and it will happen once per reset(), but after everyone else's reset has 
	   happened, so you won't greedily delay anything. 
	*/
}

FixedClock* FixedClock::create(int tempo, unsigned char beats_per_measure)
{	
	return new FixedClock(tempo, beats_per_measure);
}

VariableClock* VariableClock::create(int mintempo, int maxtempo, PinAnalogIn input, unsigned char beats_per_measure)
{	
	return new VariableClock(mintempo, maxtempo, input, beats_per_measure);
}

RandomTempoClock* RandomTempoClock::create(int mintempo, int maxtempo, unsigned char beats_per_measure)
{	
	return new RandomTempoClock(mintempo, maxtempo, beats_per_measure);
}

// RandomDropoutClock* RandomDropoutClock::create(int tempo, unsigned char beats_per_measure, int chaos)
// {	
// 	return new RandomDropoutClock(tempo, beats_per_measure, chaos);
// }
//
// SlaveClock* SlaveClock::create(PinDigitalIn input, unsigned char beats_per_measure)
// {	
// 	return new SlaveClock(input, beats_per_measure);
// }

void Clock::registerDevice(BeatDevice* device)
{
	this->devices.push_back(device);	
}

Clock::Clock()
{
	IOUtils::displayBeat(1, this);

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
		
	Serial.println("Setup Clock " + String(tempo) + " " + String(normalized_tempo));	
		
	this->beat = 0;
	this->period = 60000UL / normalized_tempo;
	this->beats_per_measure = beats_per_measure;

	this->last_clock_t = 0;
	this->next_clock_t = 0;

	Serial.println("Setup Clock " + String(tempo) + " " + String(normalized_tempo) + " " + String(this->beat) + " " + String(this->next_clock_t) + " " + String(this->last_clock_t) + " " + String(this->period));	
}

void Clock::timer(unsigned long t)
{
	// if (t % 1000 == 0)
	// {
	// 	Serial.println("Clock T = " + String(t) + " " + String(this->beat) + " " + String(this->next_clock_t) + " " + String(this->last_clock_t) + " " + String(this->period));
	// }
	
	/* Call reset on devices first */
	for (int i = 0; i < this->devices.size(); i++)
	{
		if (this->devices[i]->getNextTime() == t)
		{
			this->devices[i]->reset();
		}
	}	
	
	/* Then update the timer on all devices */
	for (int i = 0; i < this->devices.size(); i++)
	{
		this->devices[i]->timer(t);
	}	

	/* Then update the clock display */
	if (t >= this->next_clock_t)
	{
		if (t > this->next_clock_t) Serial.println("Clock missed next clock T by (ms) " + String(t - this->next_clock_t));
		IOUtils::displayBeat(this->beat + 1, this);				
		this->beat = (this->beat + 1) % this->beats_per_measure;		

		this->updateTempo(t);
	}
	else if (this->last_clock_t == 0)
	{
		IOUtils::displayBeat(this->beat, this);				
		this->beat = (this->beat + 1) % this->beats_per_measure;		

		this->updateTempo(t);
	}	

	/* Then calculate new time on devices and let them do any work they wanted deferred */
	for (int i = 0; i < this->devices.size(); i++)
	{
		if ((this->devices[i]->getNextTime() == t) || (this->devices[i]->getNextTime() == 0))
		{
			this->devices[i]->setNextTime((((unsigned long)this->devices[i]->getclockdivision() * (unsigned long)this->period) / 1000UL) + t);
			
			this->devices[i]->calculate();
		}
	}
	
	// /* There is a swingpercentage, and the clock division is smaller than the swing division */			
	// 
	// /* Calculate all the times for swinging */
	// int swingfactor = (this->swingpercentage == 0) ? 1000 : 1000 - ((333 * this->swingpercentage) / 100);
	// int swingfactor2 = (this->swingpercentage == 0) ? 1000 : 1000 + ((333 * this->swingpercentage) / 100);
	// int t_measure = t % this->period;
	// 
	// if (t % (this->swingdivision * 2) < this->swingdivision)
	// {
	// 	/* We're in the front half of the swing */				
	// 	int t_swung = (t_measure * swingfactor) / 1000;
	// 
	// 	if (t_swung % (((unsigned long)this->devices[i]->getclockdivision() * (unsigned long)this->period) / 1000UL) == 0)
	// 	{
	// 		this->devices[i]->reset();
	// 	}				
	// }
	// else
	// {
	// 	/* We're in the back half of the swing */
	// 	int t_firsthalf = ((this->swingdivision * swingfactor) / 1000);
	// 	int t_swung2 = t_firsthalf + ((t_measure * swingfactor2) / 1000);
	// 
	// 	if (t_swung2 % (((unsigned long)this->devices[i]->getclockdivision() * (unsigned long)this->period) / 1000UL) == 0)
	// 	{
	// 		this->devices[i]->reset();
	// 	}				 
	// }	
}

void Clock::updateTempo(unsigned long t)
{
//	Serial.println("Before Update " + String(t) + " " + String(this->beat) + " " + String(this->next_clock_t) + " " + String(this->last_clock_t) + " " + String(this->period));	
	
	/* If a clock type needs to update the tempo from time to time, then this can be used to do so */
	this->next_clock_t = (t + this->period);
	this->last_clock_t = t;	

//	Serial.println("After Update " + String(this->beat) + " " + String(this->next_clock_t) + " " + String(this->last_clock_t) + " " + String(this->period));	
}

VariableClock::VariableClock(int mintempo, int maxtempo, PinAnalogIn input, unsigned char beats_per_measure)
{
	this->beat = 0;
	this->input = input;
	this->beats_per_measure = beats_per_measure;
	
	/* The variable clock operates on a period based on an input voltage */
	this->mintempo = (mintempo < 1) ? 1 : (mintempo > 500) ? 500 : mintempo;
	this->maxtempo = (maxtempo < 1) ? 1 : (maxtempo > 500) ? 500 : maxtempo;
	
	pastvalues[0] = analogReadmV(this->input);
	pastvalues[1] = analogReadmV(this->input);
	pastvalues[2] = analogReadmV(this->input);
	pastvalues[3] = analogReadmV(this->input);
	
	int movingaverage = (pastvalues[0] + pastvalues[1] + pastvalues[2] + pastvalues[3]) / 4;
	if (movingaverage < 0) movingaverage = 0;
	
	int tempo = ((((unsigned long)this->maxtempo - (unsigned long)this->mintempo) * ((unsigned long)movingaverage) / 5000UL)) + this->mintempo;
 	this->period = 60000UL / tempo;
		
	this->last_clock_t = 0;
}

void VariableClock::updateTempo(unsigned long t)
{
	pastvalues[0] = analogReadmV(this->input);
	pastvalues[1] = analogReadmV(this->input);
	pastvalues[2] = analogReadmV(this->input);
	pastvalues[3] = analogReadmV(this->input);

	int movingaverage = (pastvalues[0] + pastvalues[1] + pastvalues[2] + pastvalues[3]) / 4;
	if (movingaverage < 0) movingaverage = 0;

	int tempo = ((((unsigned long)this->maxtempo - (unsigned long)this->mintempo) * ((unsigned long)movingaverage) / 5000UL)) + this->mintempo;
 	this->period = 60000UL / tempo;

	this->next_clock_t = (t + this->period);
	this->last_clock_t = t;	
	
//	Serial.println("tempo: " + String(tempo));
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

void RandomTempoClock::updateTempo(unsigned long t)
{
	int tempo = random(mintempo, maxtempo);
 	this->period = 60000UL / tempo;

	this->next_clock_t = (t + this->period);
	this->last_clock_t = t;	
}

// volatile bool SlaveClock::trigger = false;
// volatile int SlaveClock::period = 0;
// volatile unsigned long SlaveClock::t = 0;
// volatile unsigned long SlaveClock::last_clock_t = 0;
// volatile unsigned long SlaveClock::next_clock_t = 0;
// PinDigitalIn SlaveClock::input = DIGITAL_IN_NONE;
// 
// 
// SlaveClock::SlaveClock(PinDigitalIn input, unsigned char beats_per_measure)
// {
// 	SlaveClock::input = input;
// 	SlaveClock::beats_per_measure = beats_per_measure;
// 	
// #ifdef __AVR__
// 	attachInterrupt(0, SlaveClock::isr, RISING);
// #else
// 	attachInterrupt(input, SlaveClock::isr, RISING);
// #endif	
// }
// 
// void SlaveClock::isr()
// {
// 	SlaveClock::trigger = true;
// }
// 
// void SlaveClock::timer(unsigned long t)
// {	
// 	SlaveClock::t = t;
// 	
// 	if (SlaveClock::trigger)
// 	{
// 		if (SlaveClock::period == 0)
// 		{
// 			if (SlaveClock::last_clock_t == 0)
// 			{
// 				SlaveClock::last_clock_t = t;
// 			}
// 			else
// 			{
// 				SlaveClock::period = t - last_clock_t;
// 				SlaveClock::last_clock_t = t;
// 			}
// 		}
// 		else
// 		{
// 			//TODO: Update the clock display on Due based boards
// 			SlaveClock::period = SlaveClock::t - SlaveClock::last_clock_t;
// 			SlaveClock::next_clock_t = (t + this->period);
// 			SlaveClock::last_clock_t = t;
// 		}
// 
// 		/* Reset */
// 		trigger = false;		
// 	}
// 
// 	for (int i = 0; i < this->devices.size(); i++)
// 	{
// 		if (t % (((unsigned long)this->devices[i]->getclockdivision() * (unsigned long)this->period) / 1000UL) == 0)
// 		{
// 			this->devices[i]->reset();
// 		}
// 
// 		this->devices[i]->timer(t);
// 	}		
// }
// 
// RandomDropoutClock::RandomDropoutClock(int tempo, unsigned char beats_per_measure, int chaos) : FixedClock(tempo, beats_per_measure)
// {
// 	this->chaos = chaos;
// }
// 
// void RandomDropoutClock::timer(unsigned long t)
// {
// 	if (t % this->period == 0)
// 	{
// 		IOUtils::displayBeat(this->beat, this);				
// 		this->beat = (this->beat + 1) % this->beats_per_measure;		
// 	}
// 
// 	for (int i = 0; i < this->devices.size(); i++)
// 	{		
// 		if (t % (((unsigned long)this->devices[i]->getclockdivision() * (unsigned long)this->period) / 1000UL) == 0)
// 		{
// 			/* This clock occasionally forgets to call reset() on its devices */
// 			if (this->chaos <= random(100))
// 			{
// 				this->devices[i]->reset();
// 			}			
// 		}
// 		
// 		this->devices[i]->timer(t);
// 	}	
// }



