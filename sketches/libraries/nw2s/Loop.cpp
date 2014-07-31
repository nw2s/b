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
#include "Entropy.h"
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

Looper* Looper::create(PinAudioOut pin, LoopPath loops[], unsigned int loopcount, SampleRateInterrupt sri)
{
	Looper* looper = new Looper(pin, loops, loopcount, sri);

	return looper;
}

Looper* Looper::create(aJsonObject* data)
{
	static const char subFolderNodeName[] = "subfolder";
	static const char filenameNodeName[] = "filename";
	static const char glitchNodeName[] = "glitch";
	static const char reverseNodeName[] = "reverse";
	static const char densityNodeName[] = "density";
	static const char loopsNodeName[] = "loops";
	static const char mixmodeNodeName[] = "mixmode";
	static const char mixcontrolNodeName[] = "mixcontrol";
	static const char mixtriggerNodeName[] = "mixtrigger";
	
	char* subfolder = getStringFromJSON(data, subFolderNodeName);
	char* filename = getStringFromJSON(data, filenameNodeName);
	SampleRateInterrupt sri = getSampleRateFromJSON(data);
	PinAudioOut output = getAudioOutputFromJSON(data);
	PinDigitalIn glitch = getDigitalInputFromJSON(data, glitchNodeName);
	PinDigitalIn reverse = getDigitalInputFromJSON(data, reverseNodeName);
	PinDigitalIn mixtrigger = getDigitalInputFromJSON(data, mixtriggerNodeName);
	PinAnalogIn density = getAnalogInputFromJSON(data, densityNodeName);
	PinAnalogIn mixcontrol = getAnalogInputFromJSON(data, mixcontrolNodeName);
	char* mixmodeVal = getStringFromJSON(data, mixmodeNodeName);

	aJsonObject* loops = aJson.getObjectItem(data, loopsNodeName);
		
	Looper* looper;	
		
	if (loops != NULL)
	{
		int loopcount = aJson.getArraySize(loops);
		LoopPath loopPaths[loopcount];
		
		for (int i = 0; i < loopcount; i++)
		{
			aJsonObject* loopPathNode = aJson.getArrayItem(loops, i);

			char* subfolder = getStringFromJSON(loopPathNode, subFolderNodeName);
			char* filename = getStringFromJSON(loopPathNode, filenameNodeName);
			
			loopPaths[i] = { subfolder, filename };
		}

		looper = new Looper(output, loopPaths, loopcount, sri);
	}
	else
	{
		//TODO: error if we can't find the nodes/filenames
		LoopPath lp[] = { { subfolder, filename } };
			
		looper = new Looper(output, lp, 1, sri);
	}
	
	/* GLITCH TRIGGER */
	if (glitch != DIGITAL_IN_NONE)
	{
		looper->setGlitchTrigger(glitch);
	}

	/* MIX TRIGGER */
	if (mixtrigger != DIGITAL_IN_NONE)
	{
		looper->setMixTrigger(mixtrigger);
	}

	/* REVERSE TRIGGER */
	if (reverse != DIGITAL_IN_NONE)
	{
		looper->setReverseTrigger(reverse);
	}

	/* DENSITY INPUT */
	if (density != ANALOG_IN_NONE)
	{
		looper->setDensityInput(density);
	}
	
	/* DENSITY INPUT */
	if (mixcontrol != ANALOG_IN_NONE)
	{
		looper->setMixControl(mixcontrol);
	}
	
	/* MIXMODE */
	if (strcmp(mixmodeVal, "toggle") == 0)
	{
		looper->setMixMode(MIXMODE_TOGGLE);
	}
	else if (strcmp(mixmodeVal, "blend") == 0)
	{
		looper->setMixMode(MIXMODE_BLEND);
	}
	else if (strcmp(mixmodeVal, "glitch") == 0)
	{
		looper->setMixMode(MIXMODE_GLITCH);
	}
	else if (strcmp(mixmodeVal, "xor") == 0)
	{
		looper->setMixMode(MIXMODE_XOR);
	}
	else if (strcmp(mixmodeVal, "and") == 0)
	{
		looper->setMixMode(MIXMODE_AND);
	}
	else if (strcmp(mixmodeVal, "ring") == 0)
	{
		looper->setMixMode(MIXMODE_RING);
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
		int b = this->signalData->getNextSample();
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

Looper::Looper(PinAudioOut pin, LoopPath loops[], unsigned int loopcount, SampleRateInterrupt sri)
{
	/* Load the file(s) */
	for (int i = 0; i < loopcount; i++)
	{
		this->signalData.push_back(StreamingSignalData::fromSDFile("loops", loops[i].subfoldername, loops[i].filename, true));
	}

	this->density = ANALOG_IN_NONE;
	this->muted = false;
	
	this->glitchTrigger = DIGITAL_IN_NONE;
	this->reverseTrigger = DIGITAL_IN_NONE;
	this->glitched = 0;
	this->glitched_bounce = false;
	this->mixtrigger_bounce = false;
	this->reversed = false;	
	
	this->loopcount = loopcount;
	this->loop1index = 0;
	this->loop2index = 1;
	this->mixfactor = 2048;
	this->glitchmode_stream = 0;
	this->mixmode = MIXMODE_NONE;
	
	this->mixcontrol = ANALOG_IN_NONE;
	this->mixtrigger = DIGITAL_IN_NONE;
		
	this->initializeTimer(pin, sri);
}

void Looper::initializeTimer(PinAudioOut pin, SampleRateInterrupt sri)
{
	/* The event handler needs static references to these devices */
	if (pin == DUE_DAC0) AudioDevice::device0 = this;
	if (pin == DUE_DAC1) AudioDevice::device1 = this;

	/* Make sure the dac is zeroed and on */
	analogWriteResolution(12);
  	analogWrite(pin, 0);

	this->pin = pin;
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
}

void Looper::timer_handler()
{
	/* Would it be better to just turn off the timer while muted? */
	if (!this->muted)
	{
		int outputval = 0;
		
	 	dacc_set_channel_selection(DACC_INTERFACE, this->dac);
		if (this->loopcount == 1)
		{
			outputval = this->signalData[0]->getNextSample();
		}
		else
		{
			/* Mix the two according to the current mix mode */
			if (this->mixmode == MIXMODE_TOGGLE)
			{
				outputval = this->signalData[loop1index]->getNextSample();			
			}
			else if (this->mixmode == MIXMODE_AND)
			{
				outputval = this->signalData[loop1index]->getNextSample() & this->signalData[loop2index]->getNextSample();
			}
			else if (this->mixmode == MIXMODE_XOR)
			{
				outputval = this->signalData[loop1index]->getNextSample() ^ this->signalData[loop2index]->getNextSample();
			}
			else if (this->mixmode == MIXMODE_BLEND)
			{
				//TODO: Really, we should be mixing across the range of waves 
				int val1 = (this->signalData[loop1index]->getNextSample() * (4096 - this->mixfactor)) >> 12;
				int val2 = (this->signalData[loop2index]->getNextSample() * this->mixfactor) >> 12;

				/* Sum and dither */
				outputval = (val1 + val2) ^ Entropy::getBit();
			}
			else if (this->mixmode == MIXMODE_GLITCH)
			{
				int sample1 = this->signalData[loop1index]->getNextSample();
				int sample2 = this->signalData[loop2index]->getNextSample();
				
				/* Toggle from one stream to the other if the samples are equal */
				this->glitchmode_stream = this->glitchmode_stream ^ (sample1 == sample2);
				
				outputval = (this->glitchmode_stream) ? sample2 : sample1;
			}
			else if (this->mixmode == MIXMODE_RING)
			{
				long val1 = this->signalData[loop1index]->getNextSample() * this->signalData[loop2index]->getNextSample();
				
				outputval = val1 >> 16;
			}
		}
		
		/* For now, we're doing all audio as 12 bit unsigned, so we have to do the conversion before writing the register */
		dacc_write_conversion_data(DACC_INTERFACE, (outputval + 0x7FFF) >> 4);
	}
}

void Looper::timer(unsigned long t)
{
	/* If the reverse trigger is high, reverse direction of the loop playback */
	if ((reverseTrigger != DIGITAL_IN_NONE) && !reversed && digitalRead(reverseTrigger))
	{
		this->reversed = true;

		for(int i = 0; i < this->loopcount; i++)
		{
			this->signalData[i]->reverse();
		}
	}
	
	/* Unset the reversal flag if the signal is no longer high */
	if ((reverseTrigger != DIGITAL_IN_NONE) && reversed && !digitalRead(reverseTrigger))
	{
		this->reversed = false;
	}

	/* Every 10ms, read the analog input of the mix control */
	if ((mixcontrol != ANALOG_IN_NONE) && (t % 10 == 0))
	{
		/* Get the mix factor between 0 and 4096 for 0-5V */
		this->mixfactor = (analogRead(this->mixcontrol) - 2048) << 1;
		if (this->mixfactor < 0)
		{
			this->mixfactor = 0;
		}	
	}

	/* If the glitch trigger is high, seek to a random location */
	if ((glitchTrigger != DIGITAL_IN_NONE) && !glitched_bounce && digitalRead(glitchTrigger))
	{
		this->glitched_bounce = true;
		this->glitched = t;

		/* Randomize either one or the two that we're currently playing */
		this->signalData[loop1index]->seekRandom();

		if (this->loopcount > 1)
		{
			this->signalData[loop2index]->seekRandom();
		}

		/* If density is set, see if we should even play anything this glitch */
		if (this->density != ANALOG_IN_NONE)
		{
			this->muted = Entropy::getValue(0, 4000) > analogRead(this->density);
		}
	}
	
	/* If we're in a mix mode and the trigger is high, then iterate over the list of loops */
	if (this->mixmode != MIXMODE_NONE && !mixtrigger_bounce && digitalRead(mixtrigger))
	{
		mixtrigger_bounce = true;
		loop1index = (loop1index + 1) % loopcount;
		loop2index = (loop2index + 1) % loopcount;
	}
	else if (mixtrigger_bounce && !digitalRead(mixtrigger))
	{
		/* Don't do it again until the signal goes low */
		mixtrigger_bounce = false;
	}
	
	/* Every millisecond, check if it's ready to get more data loaded */
	if (this->signalData[loop1index]->isReadyForRefresh())
	{
		this->signalData[loop1index]->refresh();
	}
		
	/* Check the other one too */
	if (loopcount > 1 && this->signalData[loop2index]->isReadyForRefresh())
	{
		this->signalData[loop2index]->refresh();
	}
		
	/* Debounce the glitch every 1000 milliseconds */
	if (this->glitched_bounce && t > (this->glitched + 1000))
	{
		this->glitched_bounce = false;
	}
}

void Looper::setDensityInput(PinAnalogIn density)
{
	this->density = density;
}

void Looper::setMixControl(PinAnalogIn mixcontrol)
{
	this->mixcontrol = mixcontrol;
}

void Looper::setGlitchTrigger(PinDigitalIn glitchTrigger)
{
	this->glitchTrigger = glitchTrigger;
}

void Looper::setReverseTrigger(PinDigitalIn reverseTrigger)
{
	this->reverseTrigger = reverseTrigger;
}

void Looper::setMixTrigger(PinDigitalIn mixtrigger)
{
	this->mixtrigger = mixtrigger;
}

void Looper::setMixMode(MixMode mixmode)
{
	this->mixmode = mixmode;
}

// ClockedLooper* ClockedLooper::create(PinAudioOut pin, char* subfoldername, char* filename, SampleRateInterrupt sri, int beats, int clockdivision)
// {
// 	return new ClockedLooper(pin, subfoldername, filename, sri, beats, clockdivision);
// }
//
// ClockedLooper::ClockedLooper(PinAudioOut pin, char* subfoldername, char* filename, SampleRateInterrupt sri, int beats, int clockdivision) : Looper(pin, subfoldername, filename, sri)
// {
// 	this->clock_division = clockdivision;
// 	this->beats = beats;
// 	this->currentbeat = 0;
// }
//
// void ClockedLooper::timer(unsigned long t)
// {
// 	Looper::timer(t);
// }
//
// void ClockedLooper::reset()
// {
// 	this->currentbeat++;
//
// 	if (this->currentbeat >= this->beats)
// 	{
// 		Serial.println("resetting...");
// 		this->signalData->reset();
// 		this->currentbeat = 0;
// 	}
// }

