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

#include "Oscillator.h"
#include "Key.h"
#include "Entropy.h"



using namespace nw2s;

Saw* Saw::create(PinAudioOut pinout, PinAnalogIn pinin)
{
	return new Saw(pinout, pinin);
}

DiscreteNoise* DiscreteNoise::create(PinAudioOut pinout, PinAnalogIn pinin)
{
	return new DiscreteNoise(pinout, pinin);
}


Oscillator::Oscillator(PinAudioOut pinout)
{
	this->pinout = pinout;

	/* The event handler needs static references to these devices */
	if (pinout == DUE_DAC0) AudioDevice::device0 = this;
	if (pinout == DUE_DAC1) AudioDevice::device1 = this;

	/* Make sure the dac is zeroed and on */
	analogWriteResolution(12);
  	analogWrite(pinout, 0);

	this->channel = (pinout == DUE_DAC0) ? 1 : 2;
	this->dac = (pinout == DUE_DAC0) ? 0 : 1;

	int tc_id = (pinout == DUE_DAC0) ? ID_TC4 : ID_TC5;
	IRQn_Type tc_irq = (pinout == DUE_DAC0) ? TC4_IRQn : TC5_IRQn;

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

void Oscillator::timer_handler()
{
	/* Get the sample and dither it */
	int sample = this->getSample() ^ random(0, 1);

 	dacc_set_channel_selection(DACC_INTERFACE, this->dac);
	dacc_write_conversion_data(DACC_INTERFACE, sample);

	this->nextSample();	
}

VCO::VCO(PinAudioOut pinout, PinAnalogIn pinin) : Oscillator(pinout)
{
	this->pinin = pinin;

	/* Read the analog in and get a frequency */
	int value = analogRead(pinin);
	value = (value < 0) ? 0 : (value > 4000) ? 4000 : value;
	this->frequency = CVFREQUENCY[value];
	this->phaseindex = 0;
	this->sample = 0;
	this->samplespercycle = 1000000UL / this->frequency; // 10kHz sample rate
}

void VCO::timer(unsigned long t)
{
}

void VCO::nextSample()
{
	/* Next sample calulates the next value */
	this->phaseindex = (phaseindex + 1) % this->samplespercycle;

	if (this->phaseindex == 0)
	{
		/* Read the analog in and get a frequency */
		int value = analogRead(pinin);
		value = (value < 0) ? 0 : (value > 4000) ? 4000 : value;
		this->frequency = CVFREQUENCY[value];
		this->samplespercycle = 1000000UL / this->frequency; // 10kHz sample rate
	}

	this->sample = this->nextVCOSample();
}

int VCO::getSample()
{
	/* Sample just returns the sample value */
	return this->sample;
}


Saw::Saw(PinAudioOut pinout, PinAnalogIn pinin) : VCO(pinout, pinin)
{
}

int Saw::nextVCOSample()
{
	/* Saw is a simple osc. Current value is the same as the phase index normalized to output scale. */

	return (400000 / (this->samplespercycle * 100)) * this->phaseindex;
}

DiscreteNoise::DiscreteNoise(PinAudioOut pinout, PinAnalogIn pinin) : VCO(pinout, pinin)
{
	this->currentvalue = 0;
}

int DiscreteNoise::nextVCOSample()
{
	if (this->phaseindex == 0)
	{
		/* Get a new random value */
		this->currentvalue = Entropy::getValue(0, 4000);
	}
		
	return this->currentvalue;
}





