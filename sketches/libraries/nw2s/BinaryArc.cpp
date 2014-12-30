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
		phaseCvIn[i] = INDEX_ANALOG_IN[i+1];
		phaseCv[i] = aRead(phaseCvIn[i]);
		phase[i] = phaseCv[i] * ARC_MAX_LEDS / 4096;
		cvOut[i] = AnalogOut::create(INDEX_ANALOG_OUT[i+1]);
		dividers[i] = dividerIndex++ % MAX_DIVIDERS;
		level[i] = 0;
		gateOutput[i] = INDEX_DIGITAL_OUT[i*2+1];
		triggerOutput[i] = INDEX_DIGITAL_OUT[i*2+2];
		triggerState[i] = 0;

		updateRing(i, false);
	}
	
	/* Give it a moment... */
	delay(200);	
	IOUtils::displayBeat(this->beat, this);
	refreshArc();
}

void BinaryArc::setClockInput(PinDigitalIn input)
{
	this->clockInput = input;
}

void BinaryArc::timer(unsigned long t)
{
	currentTime = t;
	bool refresh = false;
	if (readCvClockState < t)
	{
		for (int ring = 0; ring < ARC_MAX_ENCODERS; ring++)
		{
			int readPhase = aRead(phaseCvIn[ring]);
			
			if (ring == 0) Serial.println(readPhase);
			
			if (abs(readPhase - phaseCv[ring]) >= (8192 / ARC_MAX_LEDS))
			{
				phaseCv[ring] = readPhase;
				phase[ring] = readPhase * ARC_MAX_LEDS / 4096;
				updateRing(ring, false);
				refresh = true;
			}
		}
		readCvClockState = t + 100; // only read CVs every 100ms
	}

	for (int ring = 0; ring < ARC_MAX_ENCODERS; ring++)
	{
		if (triggerState[ring] && triggerState[ring] < t)
		{
			digitalWrite(triggerOutput[ring], LOW);
			triggerState[ring] = 0;
		}
	}
	
	if (this->clockInput != DIGITAL_IN_NONE && !this->clockState && digitalRead(this->clockInput))
	{
		this->clockState = t;
		this->reset();
	}
	else if (refresh && isReady())
	{
		this->refreshArc();
		delay(2);
	}

	if (this->clockInput != DIGITAL_IN_NONE && this->clockState && ((this->clockState + 20) < t) && !digitalRead(this->clockInput))
	{
		this->clockState = 0;
	}
}

void BinaryArc::reset()
{
	beat = (beat + 1) % 16;
	IOUtils::displayBeat(beat, this);
	
	for (int ring = 0; ring < ARC_MAX_ENCODERS; ring++)
	{
		values[0][ring][counter] = prevValue[ring];
	}
	counter = (counter + 1) % ARC_MAX_LEDS;
	int mainCv = 0;
	for (int ring = 0; ring < ARC_MAX_ENCODERS; ring++)
	{
		if ((counter + ARC_MAX_LEDS - phase[ring]) % divider[dividers[ring]] == 0)
		{
			flip[ring] = !flip[ring];
			int cv = flip[ring] ? ((level[ring] > 11 ? 1000 : 0) + SEMITONE_MV[level[ring] % 12]) : 0;
			cvOut[ring]->outputCV(cv);
			digitalWrite(gateOutput[ring], flip[ring] ? HIGH : LOW);
			digitalWrite(triggerOutput[ring], HIGH);
			triggerState[ring] = currentTime + TRIGGER_LENGTH;
		}
		prevValue[ring] = values[0][ring][counter];
		values[0][ring][counter] = 15;
		mainCv += flip[ring] ? level[ring] : 0;
	}
	mainCvOut->outputCV((mainCv / 12) * 1000 + SEMITONE_MV[mainCv % 12]);
	
	if (isReady()) this->refreshArc();
	delay(2);
}

void BinaryArc::updateRing(uint8_t ring, bool refresh)
{
	bool f = (phase[ring] / divider[dividers[ring]] % 2) == 0;
	for(int led = 0; led < ARC_MAX_LEDS; led++)
	{
		if ((led + ARC_MAX_LEDS - phase[ring]) % divider[dividers[ring]] == 0) f = !f;
		values[0][ring][led] = f ? level[ring] : 0;
		if (led == counter)
		{
			prevValue[ring] = values[0][ring][led];
			flip[ring] = f;
			digitalWrite(gateOutput[ring], flip[ring] ? HIGH : LOW);
			values[0][ring][led] = 15;
		}
		
	}
	if (refresh && isReady()) this->refreshArc();
}

int BinaryArc::aRead(PinAnalogIn analogIn)
{
	return constrain((analogRead(analogIn) - 2048) * 2, 0, 4095);
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
			updateRing(ring, true);
		}
	}
	else
	{
		deltaVolState += delta;
		if (deltaVolState > voltageDelta)
		{
			deltaVolState = 0;
			level[ring] = (level[ring] + 1) % 15;
			updateRing(ring, true);

			int cv = flip[ring] ? ((level[ring] > 11 ? 1000 : 0) + SEMITONE_MV[level[ring] % 12]) : 0;
			cvOut[ring]->outputCV(cv);
			
			int mainCv = 0;
			for (int ring = 0; ring < ARC_MAX_ENCODERS; ring++)
				mainCv += flip[ring] ? level[ring] : 0;
			mainCvOut->outputCV((mainCv / 12) * 1000 + SEMITONE_MV[mainCv % 12]);
		}
	}
	delay(1);
}

void BinaryArc::buttonPressed(uint8_t encoder) {}
void BinaryArc::buttonReleased(uint8_t encoder) {}
