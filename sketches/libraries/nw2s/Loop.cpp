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


#include "IO.h"
#include "Loop.h"
#include "JSONUtil.h"
#include <Arduino.h>


using namespace nw2s;


SampleRateInterrupt sampleRateFromName(char* name)
{
	if (strcmp(name, "10000") == 0)
	{
		return SR_10000;
	}
	if (strcmp(name, "12000") == 0)
	{
		return SR_12000;
	}
	if (strcmp(name, "24000") == 0)
	{
		return SR_24000;
	}
	if (strcmp(name, "48000") == 0)
	{
		return SR_48000;
	}
	if (strcmp(name, "44100") == 0)
	{
		return SR_44100;
	}
	
	return SR_24000;
}

Looper* Looper::create(PinAudioOut pin, char* subfoldername, char* filename, SampleRateInterrupt sri)
{
	Looper* looper = new Looper(pin, subfoldername, filename, sri);

	return looper;
}

Looper* Looper::create(aJsonObject* data)
{
	static const char subFolderNodeName[] = "subfolder";
	static const char filenameNodeName[] = "filename";
	static const char glitchNodeName[] = "glitch";
	static const char reverseNodeName[] = "reverse";
	
	char* subfolder = getStringFromJSON(data, subFolderNodeName);
	char* filename = getStringFromJSON(data, filenameNodeName);
	SampleRateInterrupt sri = getSampleRateFromJSON(data);
	PinAudioOut output = getAudioOutputFromJSON(data);
	PinDigitalIn glitch = getDigitalInputFromJSON(data, glitchNodeName);
	PinDigitalIn reverse = getDigitalInputFromJSON(data, reverseNodeName);
		
	Looper* looper = new Looper(output, subfolder, filename, sri);

	if (glitch != DIGITAL_IN_NONE)
	{
		looper->setGlitchTrigger(glitch);
	}

	if (reverse != DIGITAL_IN_NONE)
	{
		looper->setReverseTrigger(reverse);
	}

	return looper;
}

EFLooper* EFLooper::create(PinAnalogOut pin, PinAnalogIn windowsize, PinAnalogIn scale, PinAnalogIn threshold, char* subfoldername, char* filename)
{
	EFLooper* looper = new EFLooper(pin, windowsize, scale, threshold, subfoldername, filename);

	return looper;
}

EFLooper* EFLooper::create(aJsonObject* data)
{
	static const char subFolderNodeName[] = "subfolder";
	static const char filenameNodeName[] = "filename";
	static const char windowsizeNodeName[] = "windowsize";
	static const char scaleNodeName[] = "scale";
	static const char fthresholdNodeName[] = "threshold";
	
	char* subfolder = getStringFromJSON(data, subFolderNodeName);
	char* filename = getStringFromJSON(data, filenameNodeName);
	PinAnalogOut output = getAnalogOutputFromJSON(data);
	PinAnalogIn windowsize = getAnalogInputFromJSON(data, windowsizeNodeName);
	PinAnalogIn scale = getAnalogInputFromJSON(data, scaleNodeName);
	PinAnalogIn threshold = getAnalogInputFromJSON(data, fthresholdNodeName);
		
	EFLooper* looper = new EFLooper(output, windowsize, scale, threshold, subfolder, filename);

	return looper;
}

EFLooper::EFLooper(PinAnalogOut pin, PinAnalogIn windowsize, PinAnalogIn scale, PinAnalogIn threshold, char* subfoldername, char* filename)
{
	this->output = AnalogOut::create(pin);
	this->thresholdin = threshold;
	this->windowsizein = windowsize;
	this->scalein = scale;
	this->signalData = StreamingSignalData::fromSDFile("loops", subfoldername, filename, true);	

	this->windowsize = analogReadmV(this->windowsizein, 0, 5000) / 10;
	this->threshold = analogReadmV(this->thresholdin, 0, 5000) / 2;
	this->scale = analogReadmV(this->scalein, 0, 5000) / 2;
}

void EFLooper::timer(unsigned long t)
{
	if (t % 100 == 0)
	{
		/* Only read the knobs every 100ms */
		this->windowsize = analogReadmV(this->windowsizein, 0, 5000) / 10;
		this->threshold = analogReadmV(this->thresholdin, 0, 5000) / 2;
		this->scale = analogReadmV(this->scalein, 0, 5000) / 2;		
	}
	
	/* Accumulate as many samples as are in the window, abs and average them and that's our output */	
	unsigned long a = 0;
	
	for (int i = 0; i < this->windowsize; i++)
	{
		/* Convert the unsigned data coming out back into signed */
		int b = this->signalData->getNextSample() - 2048;
		a += (b > 0) ? b : b * -1;
	}
	
	/* Scale, convert to a 12-bit value and clip at 5000 */
	a = (a < this->threshold) ? 0 : a;

	short int c = (a * this->scale) / (windowsize * 100);

	c = (c > 5000) ? 5000 : c;
		
	this->output->outputCV(c);
	
	if (this->signalData->isReadyForRefresh())
	{
		this->signalData->refresh();
	}
}

Looper::Looper(PinAudioOut pin, char* subfoldername, char* filename, SampleRateInterrupt sri)
{
	/* Load the file */
	this->signalData = StreamingSignalData::fromSDFile("loops", subfoldername, filename, true);
	
	/* The event handler needs static references to these devices */
	if (pin == DUE_DAC0) AudioDevice::device0 = this;
	if (pin == DUE_DAC1) AudioDevice::device1 = this;

	/* Make sure the dac is zeroed and on */
	analogWriteResolution(12);
  	analogWrite(pin, 0);

	this->pin = pin;
	this->signalData = signalData;
	this->channel = (pin == DUE_DAC0) ? 1 : 2;
	this->dac = (pin == DUE_DAC0) ? 0 : 1;

	int tc_id = (pin == DUE_DAC0) ? ID_TC4 : ID_TC5;
	IRQn_Type tc_irq = (pin == DUE_DAC0) ? TC4_IRQn : TC5_IRQn;

  	pmc_set_writeprotect(false);
  	pmc_enable_periph_clk(tc_id);

  	TC_Configure(TC1, this->channel, TC_CMR_WAVE | TC_CMR_WAVSEL_UP_RC | TC_CMR_TCCLKS_TIMER_CLOCK2);
  	TC_SetRC(TC1, this->channel, sri);
  	TC_Start(TC1, this->channel);

  	/* enable timer interrupts */
  	TC1->TC_CHANNEL[this->channel].TC_IER = TC_IER_CPCS;
  	TC1->TC_CHANNEL[this->channel].TC_IDR = ~TC_IER_CPCS;  

  	NVIC_EnableIRQ(tc_irq);
	
	this->glitchTrigger = DIGITAL_IN_NONE;
	this->reverseTrigger = DIGITAL_IN_NONE;
	this->glitched = false;
	this->reversed = false;
}

void Looper::timer_handler()
{
 	dacc_set_channel_selection(DACC_INTERFACE, this->dac);
	dacc_write_conversion_data(DACC_INTERFACE, this->signalData->getNextSample());
}

ClockedLooper* ClockedLooper::create(PinAudioOut pin, char* subfoldername, char* filename, SampleRateInterrupt sri, int beats, int clockdivision)
{
	return new ClockedLooper(pin, subfoldername, filename, sri, beats, clockdivision);
}

void Looper::timer(unsigned long t)
{
	if ((reverseTrigger != DIGITAL_IN_NONE) && !reversed && digitalRead(reverseTrigger))
	{
		this->reversed = true;
		this->signalData->reverse();
	}
	
	if ((reverseTrigger != DIGITAL_IN_NONE) && reversed && !digitalRead(reverseTrigger))
	{
		this->reversed = false;
	}
	
	if ((glitchTrigger != DIGITAL_IN_NONE) && !glitched && digitalRead(glitchTrigger))
	{
		this->glitched = true;
		this->signalData->seekRandom();
	}
	else if (this->signalData->isReadyForRefresh())
	{
		/* Every millisecond, check if it's ready to get more data loaded */
		this->signalData->refresh();
		this->glitched = false;
	}
}

void Looper::setGlitchTrigger(PinDigitalIn glitchTrigger)
{
	this->glitchTrigger = glitchTrigger;
}

void Looper::setReverseTrigger(PinDigitalIn reverseTrigger)
{
	this->reverseTrigger = reverseTrigger;
}

ClockedLooper::ClockedLooper(PinAudioOut pin, char* subfoldername, char* filename, SampleRateInterrupt sri, int beats, int clockdivision) : Looper(pin, subfoldername, filename, sri)
{
	this->clock_division = clockdivision;
	this->beats = beats;
	this->currentbeat = 0;
}

void ClockedLooper::timer(unsigned long t)
{
	Looper::timer(t);
}

void ClockedLooper::reset()
{
	this->currentbeat++;
	
	if (this->currentbeat >= this->beats)
	{
		Serial.println("resetting...");
		this->signalData->reset();
		this->currentbeat = 0;
	}
}

