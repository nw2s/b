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
#include "aJSON/aJSON.h"
#include "JSONUtil.h"

using namespace std;
using namespace nw2s;

int nw2s::clockDivisionFromName(char* name)
{	
	if (strcmp("whole", name) == 0)
	{
		return DIV_WHOLE;
	}
	if (strcmp("half", name) == 0)
	{
		return DIV_HALF;
	}
	if (strcmp("dotted half", name) == 0)
	{
		return DIV_HALF_DOT;
	}
	if (strcmp("quarter", name) == 0)
	{
		return DIV_QUARTER;
	}
	if (strcmp("quarter triplet", name) == 0)
	{
		return DIV_QUARTER_TRIPLET;
	}
	if (strcmp("dotted quarter", name) == 0)
	{
		return DIV_QUARTER_DOT;
	}
	if (strcmp("eighth", name) == 0)
	{
		return DIV_EIGHTH;
	}
	if (strcmp("eighth triplet", name) == 0)
	{
		return DIV_EIGHTH_TRIPLET;
	}
	if (strcmp("dotted eighth", name) == 0)
	{
		return DIV_EIGHTH_DOT;
	}
	if (strcmp("sixteenth", name) == 0)
	{
		return DIV_SIXTEENTH;
	}
	if (strcmp("sixteenth triplet", name) == 0)
	{
		return DIV_SIXTEENTH_TRIPLET;
	}
	if (strcmp("dotted sixteenth", name) == 0)
	{
		return DIV_SIXTEENTH_DOT;
	}
	if (strcmp("thirtysecond", name) == 0)
	{
		return DIV_THIRTYSECOND;
	}
	if (strcmp("dotted thirtysecond", name) == 0)
	{
		return DIV_THIRTYSECOND_DOT;
	}
	if (strcmp("thirtysecond triplet", name) == 0)
	{
		return DIV_THIRTYSECOND_TRIPLET;
	}	
	
	Serial.println("Unknown division " + String(name) + " using quarter instead.");
	return DIV_QUARTER;
}

BeatDevice::BeatDevice()
{
	this->clock_division = DIV_QUARTER;
	this->next_time = 0;
}

int BeatDevice::getclockdivision()
{
	/* If there's an input for the divider, read it to see where we are */
	if (this->clockDivisionInput != ANALOG_IN_NONE)
	{
		int rawVal = (analogRead(this->clockDivisionInput) - 2048);
	
		/* Limit it to the positive range */
		rawVal = (rawVal < 0) ? 0 : (rawVal > 2047) ? 2047 : rawVal;
		
		this->clock_division = CLOCK_DIVISIONS[rawVal / ((2048 / CLOCK_DIVISION_COUNT) + 1)];		
	}
	
	return this->clock_division;
}

void BeatDevice::setClockDivisionInput(PinAnalogIn input)
{
	this->clockDivisionInput = input;
}

void BeatDevice::setNextTime(unsigned long t)
{
	this->next_time = t;
}

bool BeatDevice::isStopped()
{
	return (this->stopInput == DIGITAL_IN_NONE) ? false : digitalRead(this->stopInput);
}

unsigned long BeatDevice::getNextTime()
{
	return this->next_time;
}

void BeatDevice::setStopInput(PinDigitalIn input)
{
	this->stopInput = input;
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

FixedClock* FixedClock::create(aJsonObject* data)
{
	const char tempoNodeName[] = "tempo";
	const char beatsNodeName[] = "beats";
	
	int tempo = getIntFromJSON(data, tempoNodeName, 120, 10, 1000);
	int beats = getIntFromJSON(data, beatsNodeName, 16, 1, 16);
		
	return new FixedClock(tempo, beats);
}

VariableClock* VariableClock::create(int mintempo, int maxtempo, PinAnalogIn input, unsigned char beats_per_measure)
{	
	return new VariableClock(mintempo, maxtempo, input, beats_per_measure);
}

VariableClock* VariableClock::create(aJsonObject* data)
{
	const char mintempoNodeName[] = "minTempo";
	const char maxtempoNodeName[] = "maxTempo";
	const char inputtempoNodeName[] = "tempoInput";
	const char beatsNodeName[] = "beats";
	
	int mintempo = getIntFromJSON(data, mintempoNodeName, 120, 10, 1000);
	int maxtempo = getIntFromJSON(data, maxtempoNodeName, 120, 10, 1000);
	PinAnalogIn tempoinput = getAnalogInputFromJSON(data, inputtempoNodeName);
	int beats = getIntFromJSON(data, beatsNodeName, 16, 1, 16);
		
	return new VariableClock(mintempo, maxtempo, tempoinput, beats);
}

RandomTempoClock* RandomTempoClock::create(int mintempo, int maxtempo, unsigned char beats_per_measure)
{	
	return new RandomTempoClock(mintempo, maxtempo, beats_per_measure);
}

RandomTempoClock* RandomTempoClock::create(aJsonObject* data)
{
	const char mintempoNodeName[] = "minTempo";
	const char maxtempoNodeName[] = "maxTempo";
	const char beatsNodeName[] = "beats";
	
	int mintempo = getIntFromJSON(data, mintempoNodeName, 120, 10, 1000);
	int maxtempo = getIntFromJSON(data, maxtempoNodeName, 120, 10, 1000);
	int beats = getIntFromJSON(data, beatsNodeName, 16, 1, 16);
		
	return new RandomTempoClock(mintempo, maxtempo, beats);
}

TapTempoClock* TapTempoClock::create(PinDigitalIn input, PinDigitalIn resetInput, unsigned char beats_per_measure)
{
	tapTempoClock = new TapTempoClock(input, resetInput, beats_per_measure);
	
	return TapTempoClock::tapTempoClock;
}

TapTempoClock* TapTempoClock::create(aJsonObject* data)
{
	const char inputNodeName[] = "tapInput";
	const char resetInputNodeName[] = "resetInput";
	const char beatsNodeName[] = "beats";
	
	PinDigitalIn input = getDigitalInputFromJSON(data, inputNodeName);
	PinDigitalIn resetInput = getDigitalInputFromJSON(data, resetInputNodeName);
	int beats = getIntFromJSON(data, beatsNodeName, 16, 1, 16);
		
	return TapTempoClock::create(input, resetInput, beats);
}

PassthruClock* PassthruClock::create(PinDigitalIn input, unsigned char beats_per_measure)
{
	passthruClock = new PassthruClock(input, beats_per_measure);
	
	return PassthruClock::passthruClock;
}

PassthruClock* PassthruClock::create(aJsonObject* data)
{
	const char inputNodeName[] = "tapInput";
	const char beatsNodeName[] = "beats";
	
	PinDigitalIn input = getDigitalInputFromJSON(data, inputNodeName);
	int beats = getIntFromJSON(data, beatsNodeName, 16, 1, 16);
		
	return PassthruClock::create(input, beats);
}

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
		
	this->beat = 0;
	this->period = 60000UL / normalized_tempo;
	this->beats_per_measure = beats_per_measure;

	this->last_clock_t = 0;
	this->next_clock_t = 0;
}

void Clock::timer(unsigned long t)
{	
	/* Call reset on devices first */
	if (this->period > 0)
	{
		for (int i = 0; i < this->devices.size(); i++)
		{
			if (this->devices[i]->getNextTime() <= t)
			{
				if (!this->devices[i]->isStopped())
				{
					this->devices[i]->reset();
				}
			}
		}	
	}
	
	/* Then update the timer on all devices */
	for (int i = 0; i < this->devices.size(); i++)
	{
		this->devices[i]->timer(t);
	}	

	/* Then update the clock display */
	if ((t >= this->next_clock_t) && (this->period > 0))
	{
		IOUtils::displayBeat(this->beat, this);				
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
		if (this->devices[i]->getNextTime() <= t)
		{
			int clockDivision = this->devices[i]->getclockdivision();
			
			//TODO: !!!!! Need to make sure the time divisions on on the measure rather than based on last_t !!!!!!
			
			this->devices[i]->setNextTime((((unsigned long)(clockDivision) * (unsigned long)(this->period)) / 1000UL) + t);

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
	
}

void FixedClock::updateTempo(unsigned long t)
{
	this->next_clock_t = (t + this->period);
	this->last_clock_t = t;	
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

TapTempoClock* TapTempoClock::tapTempoClock;
volatile bool TapTempoClock::tapping = false;

TapTempoClock::TapTempoClock(PinDigitalIn input, PinDigitalIn resetInput, unsigned char beats_per_measure)
{
	this->beat = 0;
	this->input = input;
	this->resetInput = resetInput;
	this->beats_per_measure = beats_per_measure;
			
	int tempo = 0;
 	this->period = 0;
		
	this->last_clock_t = 0;
	
	attachInterrupt(input, onTempoTap, RISING);
}

void TapTempoClock::timer(uint32_t t)
{
	Clock::timer(t);

	/* Keep track of the last time we saw the input high */
	if (digitalRead(this->input))
	{
		this->lastTapStateT = t;
	}	
}

void TapTempoClock::updateTempo(unsigned long t)
{
	this->next_clock_t = (t + this->period);
	this->last_clock_t = t;	
}

void TapTempoClock::tap(uint32_t t)
{	
	/* If the following conditions are met, update the tempo: */ 
	/* - the clock hasn't wrapped around */
	/* - it's not the first tap,         */
	/* - it's been at least 20mS since the last tap */
	/* - it hasn't been more than 4 seconds since the last tap */
	
	if ((t > (this->lastT + 20)) && (this->lastT > 0) && (t < (this->lastT + 4000)) && (t > (this->lastTapStateT + 20)))
	{
		/* Update the period to be the difference in your taps */
		this->period = t - lastT;	

		IOUtils::displayBeat(this->beat, this);				
		this->beat = (this->beat + 1) % this->beats_per_measure;		

		this->updateTempo(t);
		
		/* Now that we've updated the tempo, recalculate the next time for devices */
		for (int i = 0; i < this->devices.size(); i++)
		{
			if (this->devices[i]->getclockdivision() != DIV_NEVER)
			{
				this->devices[i]->setNextTime((((unsigned long)(this->devices[i]->getclockdivision()) * (unsigned long)(this->period)) / 1000UL) + t);

				this->devices[i]->calculate();
			}
		}
		
		/* Call reset on any devices that need it */
		for (int i = 0; i < this->devices.size(); i++)
		{
			if (this->devices[i]->getNextTime() <= t)
			{
				if (!this->devices[i]->isStopped())
				{
					this->devices[i]->reset();
				}
			}
		}					
	}

	this->lastT = t;	
}

void TapTempoClock::onTempoTap()
{
	if (!tapping)
	{
		tapping = true;
		/* Interrupt handler is static, so we have to keep a static reference to the clock */
		TapTempoClock::tapTempoClock->tap(millis());
		
		tapping = false;
	}
}

PassthruClock* PassthruClock::passthruClock;
volatile bool PassthruClock::tapping = false;

PassthruClock::PassthruClock(PinDigitalIn input, unsigned char beats_per_measure)
{
	this->beat = 0;
	this->input = input;
	this->beats_per_measure = beats_per_measure;			
 	this->period = 0;		
	this->last_clock_t = 0;
	
	attachInterrupt(input, onTap, RISING);
}

void PassthruClock::timer(uint32_t t)
{
	Clock::timer(t);

	/* Keep track of the last time we saw the input high */
	if (digitalRead(this->input))
	{
		this->lastTapStateT = t;
	}	
}

void PassthruClock::updateTempo(unsigned long t)
{
	this->last_clock_t = t;
}

void PassthruClock::tap(uint32_t t)
{	
	if ((t > (this->lastT + 20)) && (t > (this->lastTapStateT + 20)))
	{
		IOUtils::displayBeat(this->beat, this);				
		this->beat = (this->beat + 1) % this->beats_per_measure;		

		for (int i = 0; i < this->devices.size(); i++)
		{
			if (!this->devices[i]->isStopped())
			{
				this->devices[i]->reset();
			}
		}	
	}
	
	this->lastT = t;	
}

void PassthruClock::onTap()
{
	if (!tapping)
	{
		tapping = true;

		/* Interrupt handler is static, so we have to keep a static reference to the clock */
		PassthruClock::passthruClock->tap(millis());
		
		tapping = false;
	}
}


