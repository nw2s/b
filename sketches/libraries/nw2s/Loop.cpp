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



#include "Loop.h"
#include <Arduino.h>


using namespace nw2s;


Looper* Looper::create(PinAudioOut pin, SignalData* signalData)
{
	Looper* looper = new Looper(pin, signalData);

	return looper;
}

Looper::Looper(PinAudioOut pin, SignalData* signalData)
{
	/* The event handler needs static references to these devices */
	if (pin == DUE_DAC0) AudioDevice::device0 = this;
	if (pin == DUE_DAC1) AudioDevice::device1 = this;

	/* Make sure the dac is zeroed and on */
	analogWriteResolution(12);
  	analogWrite(pin, 0);

	this->pin = pin;
	this->signalData = signalData;
	this->size = signalData->getSize();
	this->currentSample = 0;
	this->channel = (pin == DUE_DAC0) ? 1 : 2;
	this->dac = (pin == DUE_DAC0) ? 0 : 1;

	int tc_id = (pin == DUE_DAC0) ? ID_TC4 : ID_TC5;
	IRQn_Type tc_irq = (pin == DUE_DAC0) ? TC4_IRQn : TC5_IRQn;

  	pmc_set_writeprotect(false);
  	pmc_enable_periph_clk(tc_id);

  	TC_Configure(TC1, this->channel, TC_CMR_WAVE | TC_CMR_WAVSEL_UP_RC | TC_CMR_TCCLKS_TIMER_CLOCK2);
  	TC_SetRC(TC1, this->channel, 1050); // sets 10Khz interrupt rate
  	TC_Start(TC1, this->channel);

  	/* enable timer interrupts */
  	TC1->TC_CHANNEL[this->channel].TC_IER = TC_IER_CPCS;
  	TC1->TC_CHANNEL[this->channel].TC_IDR = ~TC_IER_CPCS;  

  	NVIC_EnableIRQ(tc_irq);
}

void Looper::timer_handler()
{
	if (this->currentSample > this->size)
	{
		this->currentSample = 0;
	}
	
 	dacc_set_channel_selection(DACC_INTERFACE, this->dac);
	dacc_write_conversion_data(DACC_INTERFACE, signalData->getSample(currentSample));
	this->currentSample++;
}

ClockedLooper* ClockedLooper::create(PinAudioOut pin, SignalData* signalData, int beats, int clockdivision)
{
	return new ClockedLooper(pin, signalData, beats, clockdivision);
}

ClockedLooper::ClockedLooper(PinAudioOut pin, SignalData* signalData, int beats, int clockdivision) : Looper(pin, signalData)
{
	this->clock_division = clockdivision;
	this->beats = beats;
	this->currentbeat = 0;
}

void ClockedLooper::timer(unsigned long t)
{
	
}

void ClockedLooper::reset()
{
	this->currentbeat++;
	
	if (this->currentbeat >= this->beats)
	{
		this->currentSample = 0;
		this->currentbeat = 0;
	}
}

