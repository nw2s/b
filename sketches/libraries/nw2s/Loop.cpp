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
#include "Constants.h"
#include <Arduino.h>

#define CONTROL_CHANGE_THRESHOLD 25


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
	static const char reversemodeNodeName[] = "reversemode";
	static const char mixcontrolNodeName[] = "mixcontrol";
	static const char mixtriggerNodeName[] = "mixtrigger";
	static const char resettriggerNodeName[] = "resettrigger";
	static const char lengthcontrolNodeName[] = "lengthcontrol";
	static const char finelengthcontrolNodeName[] = "finelengthcontrol";
	static const char startcontrolNodeName[] = "startcontrol";
	static const char bitcontrolNodeName[] = "bitcontrol";
	static const char triggeroutputNodeName[] = "triggerout";
	
	char* subfolder = getStringFromJSON(data, subFolderNodeName);
	char* filename = getStringFromJSON(data, filenameNodeName);
	SampleRateInterrupt sri = getSampleRateFromJSON(data);
	PinAudioOut output = getAudioOutputFromJSON(data);
	PinDigitalIn glitch = getDigitalInputFromJSON(data, glitchNodeName);
	PinDigitalIn resettrigger = getDigitalInputFromJSON(data, resettriggerNodeName);
	PinDigitalIn reverse = getDigitalInputFromJSON(data, reverseNodeName);
	PinDigitalIn mixtrigger = getDigitalInputFromJSON(data, mixtriggerNodeName);
	PinDigitalOut triggerout = getDigitalOutputFromJSON(data, triggeroutputNodeName);
	PinAnalogIn density = getAnalogInputFromJSON(data, densityNodeName);
	PinAnalogIn bitcontrol = getAnalogInputFromJSON(data, bitcontrolNodeName);
	PinAnalogIn mixcontrol = getAnalogInputFromJSON(data, mixcontrolNodeName);
	PinAnalogIn lengthcontrol = getAnalogInputFromJSON(data, lengthcontrolNodeName);
	PinAnalogIn finelengthcontrol = getAnalogInputFromJSON(data, finelengthcontrolNodeName);
	PinAnalogIn startcontrol = getAnalogInputFromJSON(data, startcontrolNodeName);
	char* mixmodeVal = getStringFromJSON(data, mixmodeNodeName);
	char* reversemodeVal = getStringFromJSON(data, reversemodeNodeName);

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

	/* RESET TRIGGER */
	if (resettrigger != DIGITAL_IN_NONE)
	{
		looper->setResetTrigger(resettrigger);
	}

	/* DENSITY INPUT */
	if (density != ANALOG_IN_NONE)
	{
		looper->setDensityInput(density);
	}
	
	/* MIXCONTROL INPUT */
	if (mixcontrol != ANALOG_IN_NONE)
	{
		looper->setMixControl(mixcontrol);
	}
	
	/* TRIGGER OUTPUT */
	if (triggerout != ANALOG_OUT_NONE)
	{
		looper->setTriggerOut(triggerout);
	}
	
	/* LENGTHCONTROL INPUT */
	if (lengthcontrol != ANALOG_IN_NONE)
	{
		looper->setLengthControl(lengthcontrol);
	}

	/* FINELENGTHCONTROL INPUT */
	if (finelengthcontrol != ANALOG_IN_NONE)
	{
		looper->setFineLengthControl(finelengthcontrol);
	}

	/* STARTCONTROL INPUT */
	if (startcontrol != ANALOG_IN_NONE)
	{
		looper->setStartControl(startcontrol);
	}
	
	/* BITCONTROL INPUT */
	if (bitcontrol != ANALOG_IN_NONE)
	{
		looper->setBitControl(bitcontrol);
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
	else if (strcmp(mixmodeVal, "cv") == 0)
	{
		looper->setMixMode(MIXMODE_CV);
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

	/* REVERSEMODE */
	if (strcmp(reversemodeVal, "gate") == 0)
	{
		looper->setReverseMode(REVERSE_GATE);
	}
	else if (strcmp(reversemodeVal, "trigger") == 0)
	{
		looper->setReverseMode(REVERSE_TRIGGER);
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
		
		/* Figure out the loop length */
		
		/* Figure out the start and stop point */
			
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

	this->muted = false;	
	this->glitched = 0;
	this->glitched_bounce = false;
	this->mixtrigger_bounce = false;
	this->reversed = false;	
	
	this->loopcount = loopcount;
	this->looprange = 4095 / loopcount;
	this->loop1index = 0;
	this->loop2index = 1;
	this->glitchmode_stream = 0;
	this->mixmode = MIXMODE_NONE;
			
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
	if (!this->muted)
	{
		int32_t outputval = 0;
		
	 	dacc_set_channel_selection(DACC_INTERFACE, this->dac);
		if (this->loopcount == 1)
		{
			outputval = this->signalData[0]->getNextSample();
		}
		else
		{
			/* Mix the two according to the current mix mode */
			if (this->mixmode == MIXMODE_TOGGLE || this->mixmode == MIXMODE_CV)
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
				/* Our gain lookup table converts linear values to a kinda equal power curve */
				int16_t sample1 = this->signalData[loop1index]->getNextSample();
				int16_t sample2 = this->signalData[loop2index]->getNextSample();

				int16_t val1 = (sample1 * this->loop1gain) / 1024;
				int16_t val2 = (sample2 * this->loop2gain) / 1024;

				/* Sum and dither */
				outputval = (val1 + val2) ^ Entropy::getBit();				
			}
			else if (this->mixmode == MIXMODE_GLITCH)
			{
				int sample1 = this->signalData[loop1index]->getNextSample();
				int sample2 = this->signalData[loop2index]->getNextSample();
				
				/* Toggle from one stream to the other if the samples are equal */
				this->glitchmode_stream = this->glitchmode_stream ^ (sample1 == sample2);
				
				if (!this->nexttriggerstate) this->nexttriggerstate = (sample1 == sample2);
				
				outputval = (this->glitchmode_stream) ? sample2 : sample1;
			}
			else if (this->mixmode == MIXMODE_RING)
			{
				long val1 = this->signalData[loop1index]->getNextSample() * this->signalData[loop2index]->getNextSample();
				
				outputval = val1 >> 16;
			}
		}
		
		/* Convert to unsigned range */
		outputval = outputval + 0x7FFF;


		/* Saturate at 16 bits */
		outputval = (outputval > 0xFFFF) ? 0xFFFF : (outputval < 0) ? 0 : outputval;

		/* Bit crushing */
		if (this->bitcontrol != ANALOG_IN_NONE)
		{
			outputval = outputval & this->bitDepthMask;
		}
					
		/* For now, we're doing all audio as 12 bit unsigned, so we have to do the conversion before writing the register */
		uint32_t dacval = outputval >> 4;
		
		dacc_write_conversion_data(DACC_INTERFACE, dacval);
	}
}

void Looper::timer(unsigned long t)
{	
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
		

	if (this->reverseMode == REVERSE_TRIGGER)
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
	}
	else
	{
		/* If the reverse gate is high, then reverse direction */
		this->reversed = digitalRead(reverseTrigger);
	}

	/* Every 10ms, read the analog input of the mix control */
	if (this->mixcontrol != ANALOG_IN_NONE) 
	{		
		/* Get the mix factor between 0 and 4096 for 0-5V */
		if (t % 10 == 0)
		{
			this->controlvalImmediate = (analogRead(this->mixcontrol) - 2048) << 1;

			/* Clip to unsigned 12 bits */
			this->controlvalImmediate = (controlvalImmediate < 0) ? 0 : (controlvalImmediate > 4095) ? 4095 : controlvalImmediate;
		}

		int16_t controldifference = this->controlvalImmediate - this->controlval;
		int16_t changeval = 0;

		if (controldifference > 0)
		{
			changeval = (controldifference > 1024) ? 8 : (controldifference > 128) ? 2 : 1;
		}
		else if (controldifference < 0)
		{
			changeval = (controldifference < -1024) ? -8 : (controldifference < -128) ? -2 : -1;
		}

		this->controlval = this->controlval + changeval;

		/* Don't mess with the active loops if we're in a non-blended mode */
		if (this->mixmode != MIXMODE_TOGGLE && this->mixmode != MIXMODE_NONE)
		{
			uint16_t mixfactor = (this->controlval % (4095 / (this->loopcount - 1))) * (this->loopcount - 1);

			if (this->mixmode == MIXMODE_CV)
			{
				this->loop1index = (this->controlval * this->loopcount) >> 12;
			}
			else
			{
				/* Calculate the index of the waves to mix between */
				int z = (this->controlval * (this->loopcount - 1)) >> 12;
						
				if (z % 2 == 0)
				{
					this->loop1index = z;
					this->loop2index = z + 1;
					this->loop1gain = EQUAL_POWER_1024[1023 - (mixfactor >> 2)];
					this->loop2gain = EQUAL_POWER_1024[mixfactor >> 2];
				}
				else
				{
					this->loop1index = z + 1;
					this->loop2index = z;
					this->loop1gain = EQUAL_POWER_1024[mixfactor >> 2];
					this->loop2gain = EQUAL_POWER_1024[1023 - (mixfactor >> 2)];
				}				
			}
		}
	}
	
	if (this->triggerout != DIGITAL_OUT_NONE)
	{
		if (this->nexttriggerstate && !this->triggerstate)
		{
			/* If the trigger is supposed to be high and we are not, then raise the output */
			digitalWrite(this->triggerout, 1);
			this->triggert = t + 50;
			this->triggerstate = true;
			this->nexttriggerstate = false;
		}
		else if (this->triggerstate && (t > this->triggert))
		{
			this->triggerstate = false;
			digitalWrite(this->triggerout, 0);
		}
	}
	
	/* Every 10ms, read the analog input of the bit control */
	if ((this->bitcontrol != ANALOG_IN_NONE) && (t % 10 == 0))
	{
		/* Get the bit factor between 0 and 16 for 0-5V */
		int controlval = (analogRead(this->bitcontrol) - 2048) >> 7;
		
		/* Calculate the bitmask */
		this->bitDepthMask = BIT_CRUSH_MASK[controlval];
	}
	
	if ((this->startcontrol != ANALOG_IN_NONE) && (t % 10 == 0))
	{
		/* Get the mix factor between 0 and 4096 for 0-5V */
		int controlval = (analogRead(this->startcontrol) - 2048) * 2;

		controlval = (controlval < 0) ? 0 : (controlval > 4095) ? 4095 : controlval;

		/* Snap to zero if we're close to ensure starting at the beginning */
		if (controlval < 4)
		{
			controlval = 0;
		}

		/* Only update if the change is greater than some threshold */
		if (controlval < (CONTROL_CHANGE_THRESHOLD * 2) || controlval > (this->laststartval + CONTROL_CHANGE_THRESHOLD) || controlval < (this->laststartval - CONTROL_CHANGE_THRESHOLD))
		{
			/* Update all of our samples with that info */
			for (int i = 0; i < this->signalData.size(); i++)
			{
				this->signalData[i]->setStartFactor(controlval);
			}

			this->laststartval = controlval;
		}
	}

	if ((this->lengthcontrol != ANALOG_IN_NONE) && (t % 10 == 0))
	{
		/* Get the mix factor between 0 and 4096 for 0-5V */
		int controlval = (analogRead(this->lengthcontrol) - 2048) << 1;

		controlval = (controlval < 0) ? 0 : (controlval > 4095) ? 4095 : controlval;

		/* Only update if the change is greater than some threshold */
		if (controlval > (this->lastlenval + CONTROL_CHANGE_THRESHOLD) || controlval < (this->lastlenval - CONTROL_CHANGE_THRESHOLD))
		{
			/* Update all of our samples with that info */
			for (int i = 0; i < this->signalData.size(); i++)
			{
				this->signalData[i]->setEndFactor(controlval);
			}

			this->lastlenval = controlval;
		}
	}

	if ((this->finelengthcontrol != ANALOG_IN_NONE) && (t % 10 == 0))
	{
		/* Get the mix factor between 0 and 4096 for 0-5V */
		int controlval = (analogRead(this->finelengthcontrol) - 2048) << 1;

		controlval = (controlval < 0) ? 0 : (controlval > 4095) ? 4095 : controlval;

		/* Only update if the change is greater than some threshold */
		if (controlval > (this->lastfinelenval + CONTROL_CHANGE_THRESHOLD) || controlval < (this->lastfinelenval - CONTROL_CHANGE_THRESHOLD))
		{
			/* Update all of our samples with that info */
			for (int i = 0; i < this->signalData.size(); i++)
			{
				this->signalData[i]->setFineEndFactor(controlval);
			}

			this->lastfinelenval = controlval;
		}
	}

	/* If the reset trigger is high, seek to the beginning */
	if ((resetTrigger != DIGITAL_IN_NONE) && !reset_bounce && digitalRead(resetTrigger))
	{
		this->reset_bounce = true;
		this->resett = t;

		/* Reset any of the loops that we need to */
		this->signalData[loop1index]->reset();
		
		if (this->loopcount > 1)
		{
			this->signalData[loop2index]->reset();
		}		
	}
	else if (this->reset_bounce && t > (this->resett + 1000) && !digitalRead(resetTrigger))
	{
		/* Debounce the reset every 1000 milliseconds */
		this->reset_bounce = false;
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
	
	/* If we're in a non-blend mix mode and the trigger is high, then iterate over the list of loops */
	if (this->mixmode != MIXMODE_NONE && this->mixmode != MIXMODE_BLEND && !mixtrigger_bounce && digitalRead(mixtrigger))
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

void Looper::setBitControl(PinAnalogIn bitcontrol)
{
	this->bitcontrol = bitcontrol;
}

void Looper::setLengthControl(PinAnalogIn lengthcontrol)
{
	this->lengthcontrol = lengthcontrol;
}

void Looper::setFineLengthControl(PinAnalogIn finelengthcontrol)
{
	this->finelengthcontrol = finelengthcontrol;
}

void Looper::setStartControl(PinAnalogIn startcontrol)
{
	this->startcontrol = startcontrol;
}

void Looper::setGlitchTrigger(PinDigitalIn glitchTrigger)
{
	this->glitchTrigger = glitchTrigger;
}

void Looper::setReverseTrigger(PinDigitalIn reverseTrigger)
{
	this->reverseTrigger = reverseTrigger;
}

void Looper::setResetTrigger(PinDigitalIn resetTrigger)
{
	this->resetTrigger = resetTrigger;
}

void Looper::setMixTrigger(PinDigitalIn mixtrigger)
{
	this->mixtrigger = mixtrigger;
}

void Looper::setMixMode(MixMode mixmode)
{
	this->mixmode = mixmode;
}

void Looper::setReverseMode(ReverseMode reverseMode)
{
	this->reverseMode = reverseMode;
}

void Looper::setTriggerOut(PinDigitalOut triggerout)
{
	this->triggerout = triggerout;
}
