/*
	BinaryArc - a binary sequencer for nw2s::b and monome arc
	copyright (c) 2014 scannerdarkly (fuzzybeacon@gmail.com)

	This code is developed for the the nw2s::b framework 
	Copyright (C) 2013 Scott Wilson (thomas.scott.wilson@gmail.com)

	Parts of it are also based on USB Host library 
	https://github.com/felis/USB_Host_Shield_2.0
	and the monome
	https://github.com/monome

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

#include "BinaryArc.h"

#define TRIGGER_LENGTH 40 // must be over 20 for the triggers out to be used reliably as triggers in with 20ms jitter protection threshold

using namespace nw2s;

BinaryArc* BinaryArc::create(uint8_t encoderCount, bool pushButton)
{
	return new BinaryArc(encoderCount, pushButton);
}

BinaryArc* BinaryArc::create(aJsonObject* data)
{
	static const char encoderCountNodeName[] = "encoderCount";
	static const char pushButtonNodeName[] = "pushButton";
	static const char clockNodeName[] = "externalClock";

	uint8_t encoderCount = getIntFromJSON(data, encoderCountNodeName, 4, 1, 4);
	bool pushButton = getBoolFromJSON(data, pushButtonNodeName, false);
	PinDigitalIn clockPin = getDigitalInputFromJSON(data, clockNodeName);
	
	BinaryArc* binaryArc = new BinaryArc(encoderCount, pushButton);
	
	if (clockPin != DIGITAL_IN_NONE)
	{
		binaryArc->setClockInput(clockPin);
	}
			
	return binaryArc;
}

BinaryArc::BinaryArc(uint8_t encoderCount, bool pushButton) : USBArcController(encoderCount, pushButton)
{
	this->encoderCount = encoderCount;
	this->pushButton = pushButton;

	this->beat = 0;
	this->clock_division = DIV_SIXTEENTH;

	mainCvOut = AnalogOut::create(DUE_SPI_4822_15);
	uint8_t dividerIndex = 1;
	for (int i = 0; i < ARC_MAX_ENCODERS; i++)
	{	
		cvOut[i] = AnalogOut::create(INDEX_ANALOG_OUT[i+1]);
		dividers[i] = dividerIndex++ % MAX_DIVIDERS;
		updateRingForNewDivider(i, false);
		level[i] = 0;
		voltage[i] = 0;
		flip[i] = true;
		prevValue[i] = 0;
	}
	
	/* Give it a moment... */
	delay(200);	
	refreshArc();
}

void BinaryArc::setClockInput(PinDigitalIn input)
{
	this->clockInput = input;
}

void BinaryArc::timer(unsigned long t)
{
	if (this->clockInput != DIGITAL_IN_NONE && !this->clockState && digitalRead(this->clockInput))
	{
		this->clockState = t;
		this->reset();
	}

	if (this->clockInput != DIGITAL_IN_NONE && this->clockState && ((this->clockState + 20) < t) && !digitalRead(this->clockInput))
	{
		this->clockState = 0;
	}
}

void BinaryArc::reset()
{
	beat = (beat + 1) % 16; // thought this was for the clock LED (red) but doesn't seem to be the case...
	
	for (int ring = 0; ring < ARC_MAX_ENCODERS; ring++)
	{
		values[0][ring][counter] = prevValue[ring];
	}
	counter = (counter + 1) % ARC_MAX_LEDS;
	int mainCv = 0;
	for (int ring = 0; ring < ARC_MAX_ENCODERS; ring++)
	{
		if (counter % divider[dividers[ring]] == 0)
		{
			flip[ring] = !flip[ring];
			// voltage[ring] = 4096 * (flip[ring] ? level[ring] : 0) / 5 / 12 / 15;
			/* Converts to either -10/+10 or -5/+5 depending on which model you have */
			//int cv = (level[ring] > 11 ? 2000 : 1000) + SEMITONE_MV[flip[ring] ? level[ring] % 12 : 0];
			//voltage[ring] = constrain(4095 - (((cv + (b::cvGainMode ? 10000 : 5000)) * 4000UL) / 10000), 0, 4095);
			voltage[ring] = flip[ring] ? level[ring] * 100 : 0;
			cvOut[ring]->outputCV(2000 + voltage[ring]);
		}
		prevValue[ring] = values[0][ring][counter];
		values[0][ring][counter] = 15;
		mainCv += voltage[ring];
	}
	mainCvOut->outputCV(constrain(mainCv, 0, 4095));
	if (isReady()) this->refreshArc();
	delay(2);
}

void BinaryArc::updateRingForNewDivider(uint8_t ring, bool refresh)
{
	bool f = true;
	for(int led = 0; led < ARC_MAX_LEDS; led++)
	{
		if (led % divider[dividers[ring]] == 0) f = !f;
		values[0][ring][led] = f ? level[ring] : 0;
		if (led == counter)
		{
			prevValue[ring] = values[0][ring][led];
			values[0][ring][led] = 15;
		}
		
	}
	if (refresh && isReady()) this->refreshArc();
}

void BinaryArc::encoderPositionChanged(uint8_t ring, int8_t delta)
{
	if (delta < 0)
	{
		deltaDivState += abs(delta);
		if (deltaDivState > divisionDelta)
		{
			deltaDivState = 0;
			dividers[ring] = (dividers[ring] + 1) % MAX_DIVIDERS;
			updateRingForNewDivider(ring, true);
		}
	}
	else
	{
		deltaVolState += delta;
		if (deltaVolState > voltageDelta)
		{
			deltaVolState = 0;
			level[ring] = (level[ring] + 1) % 15;
			updateRingForNewDivider(ring, true);
			// voltage[ring] = 4096 * (flip[ring] ? level[ring] : 0) / 5 / 12 / 15;
			voltage[ring] = flip[ring] ? level[ring] * 100 : 0;
			cvOut[ring]->outputCV(2000 + voltage[ring]);
			// int cv = level[ring] > 11 ? 2000 : 1000 + SEMITONE_MV[level[ring] % 12];
			// voltage[ring] = constrain(4095 - (((cv + (b::cvGainMode ? 10000 : 5000)) * 4000UL) / 10000), 0, 4095);
			// cvOut[ring]->outputCV(voltage[ring]);
			int mainCv = 0;
			for (int ring = 0; ring < ARC_MAX_ENCODERS; ring++)
				mainCv += voltage[ring];
			mainCvOut->outputCV(constrain(mainCv, 0, 4095));
		}
	}
	delay(1);
}

void BinaryArc::buttonPressed(uint8_t encoder) {}
void BinaryArc::buttonReleased(uint8_t encoder) {}
