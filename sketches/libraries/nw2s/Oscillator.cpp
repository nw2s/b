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
#include "SignalData.h"
#include "../aJSON/aJSON.h"



using namespace nw2s;

Saw* Saw::create(PinAudioOut pinout, PinAnalogIn pinin)
{
	return new Saw(pinout, pinin);
}

VCSamplingFrequencyOscillator* VCSamplingFrequencyOscillator::create(PinAudioOut pinout, PinAnalogIn pinin)
{
	return new VCSamplingFrequencyOscillator(pinout, pinin);
}

VCSamplingFrequencyOscillator* VCSamplingFrequencyOscillator::create(aJsonObject* data)
{
	aJsonObject* inputNode = aJson.getObjectItem(data, "analogInput");
	aJsonObject* outputNode = aJson.getObjectItem(data, "dacOutput");
	
	if (inputNode == NULL)
	{
		Serial.println("The VCSamplingFrequencyOscillator node is missing an analogInput definition.");
		return NULL;
	}
	
	if (outputNode == NULL)
	{
		Serial.println("The VCSamplingFrequencyOscillator node is missing a dacOutput definition.");
		return NULL;
	}
	
	Serial.println("Frequency Input: Analog In " + String(inputNode->valueint));
	Serial.println("Audio Output: DAC" + String(inputNode->valueint));
	
	return new VCSamplingFrequencyOscillator(INDEX_AUDIO_OUT[outputNode->valueint - 1], INDEX_ANALOG_IN[inputNode->valueint - 1]);
}


BitCode* BitCode::create(PinAudioOut pinout, PinAnalogIn pinin)
{
	return new BitCode(pinout, pinin);
}

BitCode* BitCode::create(aJsonObject* data)
{
	//TODO: Use the encapsulated version of code
	aJsonObject* inputNode = aJson.getObjectItem(data, "analogInput");
	aJsonObject* outputNode = aJson.getObjectItem(data, "dacOutput");
	
	if (inputNode == NULL)
	{
		Serial.println("The DiscreteNoise node is missing an analogInput definition.");
		return NULL;
	}
	
	if (outputNode == NULL)
	{
		Serial.println("The DiscreteNoise node is missing a dacOutput definition.");
		return NULL;
	}
	
	Serial.println("Frequency Input: Analog In " + String(inputNode->valueint));
	Serial.println("Audio Output: DAC" + String(inputNode->valueint));
	
	return new BitCode(INDEX_AUDIO_OUT[outputNode->valueint - 1], INDEX_ANALOG_IN[inputNode->valueint - 1]);
}

DiscreteNoise* DiscreteNoise::create(PinAudioOut pinout, PinAnalogIn pinin)
{
	return new DiscreteNoise(pinout, pinin);
}

DiscreteNoise* DiscreteNoise::create(aJsonObject* data)
{
	//TODO: Use the encapsulated version of code
	aJsonObject* inputNode = aJson.getObjectItem(data, "analogInput");
	aJsonObject* outputNode = aJson.getObjectItem(data, "dacOutput");
	
	if (inputNode == NULL)
	{
		Serial.println("The DiscreteNoise node is missing an analogInput definition.");
		return NULL;
	}
	
	if (outputNode == NULL)
	{
		Serial.println("The DiscreteNoise node is missing a dacOutput definition.");
		return NULL;
	}
	
	Serial.println("Frequency Input: Analog In " + String(inputNode->valueint));
	Serial.println("Audio Output: DAC" + String(inputNode->valueint));
	
	return new DiscreteNoise(INDEX_AUDIO_OUT[outputNode->valueint - 1], INDEX_ANALOG_IN[inputNode->valueint - 1]);
}


Oscillator::Oscillator(PinAudioOut pinout)
{
	this->pinout = pinout;
}

void Oscillator::timer_start()
{
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

VCSamplingFrequencyOscillator::VCSamplingFrequencyOscillator(PinAudioOut pinout, PinAnalogIn pinin) : Oscillator(pinout)
{
	this->pinin = pinin;
	this->sample = 2000;
	this->phaseindex = 0;
	this->decimationlevel = 0;
	this->interruptrate = 1050;
	this->nextinterruptrate = 1050;
		
	short unsigned int w[600];	
	unsigned int source[600];
	
	/* Initialize 1:1 array */
	for (int i = 0; i < 600; i++)
	{
		w[i] = SIGNAL_SAW[i] / 100;
		source[i] = SIGNAL_SAW[i];
	}
	
	this->wave = SignalData::fromArray(w, 600);
	this->wave2 = decimate(source, 600, 100, 300); 
	this->wave3 = decimate(source, 600, 100, 150);
	this->wave4 = decimate(source, 600, 100, 75);
	this->wave5 = decimate(source, 600, 100, 25);
	this->wave6 = decimate(source, 600, 100, 15);	
	this->wave7 = decimate(source, 600, 100, 5);
		
	this->timer_start();
}

SignalData* VCSamplingFrequencyOscillator::decimate(unsigned int* source, int size, int sourcescale, int targetsize)
{
	/* NOTE: targetsize must be a factor of size!!!! */
	int factor = size / targetsize;
	unsigned short int destination[targetsize];
	
	for (int i = 0; i < targetsize; i++)
	{
		long accumulator = 0;
		int j = i * factor;
		
		for (int offset = 0; offset < factor; offset++)
		{
			accumulator += source[j + offset];
		}
		
		destination[i] = accumulator / (factor * sourcescale);
	}
	
	return SignalData::fromArray(destination, targetsize);
}


int VCSamplingFrequencyOscillator::getSample()
{
	return this->sample;
}

void VCSamplingFrequencyOscillator::nextSample()
{
	if ((this->phaseindex == 0) && (this->nextinterruptrate != this->interruptrate))
	{
		/* our interrupt rate changed and we're at a zero point, so update */
		this->interruptrate = this->nextinterruptrate;
		this->decimationlevel = this->nextdecimationlevel;
		
	  	TC_SetRC(TC1, this->channel, this->nextinterruptrate); // sets 10Khz interrupt rate	
	}
	
	if (this->decimationlevel == 0)
	{
		this->phaseindex = (this->phaseindex + 1) % 600;
		this->sample = this->wave->getSample(this->phaseindex);
	}
	else if (this->decimationlevel == 1)
	{
		this->phaseindex = (this->phaseindex + 1) % 300;
		this->sample = this->wave2->getSample(this->phaseindex);
	}
	else if (this->decimationlevel == 2)
	{
		this->phaseindex = (this->phaseindex + 1) % 150;
		this->sample = this->wave3->getSample(this->phaseindex);
	}
	else if (this->decimationlevel == 3)
	{
		this->phaseindex = (this->phaseindex + 1) % 75;
		this->sample = this->wave4->getSample(this->phaseindex);
	}	
	else if (this->decimationlevel == 4)
	{
		this->phaseindex = (this->phaseindex + 1) % 25;
		this->sample = this->wave5->getSample(this->phaseindex);
	}	
	else if (this->decimationlevel == 5)
	{
		this->phaseindex = (this->phaseindex + 1) % 15;
		this->sample = this->wave6->getSample(this->phaseindex);
	}	
	else if (this->decimationlevel == 6)
	{
		this->phaseindex = (this->phaseindex + 1) % 5;
		this->sample = this->wave7->getSample(this->phaseindex);
	}	
}

void VCSamplingFrequencyOscillator::timer(unsigned long t)
{
	if (t % 5 == 0)
	{
		/* Read the analog in and get a frequency */
		//int value = analogRead(pinin);
		//value = (value < 0) ? 0 : (value > 4000) ? 4000 : value;
			
		// value = value >> 2;
		// value = value << 2;
	
		/* Testing a sweep */
		// int value = (t / 10) % 4000; 
	
		/* Hardcode to 1kHz for testing */
		int frequency100 = 100000;
	
		/* Convert the input value to a frequency (x100) via lookup */
		// int frequency100 = CVFREQUENCY[value];
	
		/* We want to use progressively decimated waves as the frequency goes up */
		if (frequency100 < 3300)
		{
			this->nextdecimationlevel = 0;
			this->nextinterruptrate = 10500000 / (frequency100 * 6);
		}
		else if (frequency100 < 6600)
		{
			this->nextdecimationlevel = 1;
			this->nextinterruptrate = 10500000 / (frequency100 * 3);
		}
		else if (frequency100 < 20000)
		{
			this->nextdecimationlevel = 2;
			this->nextinterruptrate = 10500000 / ((frequency100 / 10) * 15);
		}
		else if (frequency100 < 53300)
		{
			this->nextdecimationlevel = 3;
			this->nextinterruptrate = 10500000 / ((frequency100 / 100) * 75);
		}
		else if (frequency100 < 160000)
		{
			this->nextdecimationlevel = 4;
			this->nextinterruptrate = 10500000 / (frequency100 / 4);
		}
		else if (frequency100 < 266600)
		{
			this->nextdecimationlevel = 5;
			this->nextinterruptrate = 10500000 / ((frequency100 / 100) * 15);
		}
		else
		{
			this->nextdecimationlevel = 6;
			this->nextinterruptrate = 10500000 / (frequency100 / 20);
		}
	}
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

	this->timer_start();
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


Saw::Saw(PinAudioOut pinout, PinAnalogIn pinin) : VCSamplingFrequencyOscillator(pinout, pinin) 
{
}

// Sin::Sin(PinAudioOut pinout, PinAnalogIn pinin) : VCO(pinout, pinin)
// {
// }
// 
// int Sin::nextVCOSample()
// {
// 	if (this->phaseindex == 0)
// 	{
// 		/* Get a new random value */
// 		this->currentvalue = Entropy::getValue(0, 4000);
// 	}
// 		
// 	return this->currentvalue;
// }

BitCode::BitCode(PinAudioOut pinout, PinAnalogIn pinin) : VCO(pinout, pinin)
{
	//TODO: Add an integer parameter to indicate the iterator initial value 
	//TODO: Add a parameter to allow an analog input to define the iterator offset
	//TODO: Add a parameter that switches between algos
	
	this->currentvalue = 0;
	this->iterator = 0;
}

int BitCode::nextVCOSample()
{
	if (this->phaseindex == 0)
	{
		this->currentvalue = this->iterator * ((this->iterator >> 12 | this->iterator >> 8) & 63 & this->iterator >> 4);
		this->iterator++;
	}

	return this->currentvalue;
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





