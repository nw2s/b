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
	uint8_t divisorIndex = 1;
	for (int i = 0; i < ARC_MAX_ENCODERS; i++)
	{	
		transposeCvIn[i] = INDEX_ANALOG_IN[i+1];
		transposeCv[i] = aRead(transposeCvIn[i]);
		delay(10); // give the ADC time to recover
		phaseCvIn[i] = INDEX_ANALOG_IN[i+1+ARC_MAX_ENCODERS];
		phaseCv[i] = aRead(phaseCvIn[i]);
		delay(10);
		divisorCvIn[i] = INDEX_ANALOG_IN[i+1+ARC_MAX_ENCODERS*2];
		divisorCv[i] = aRead(divisorCvIn[i]) * MAX_DIVISORS / 2048;
		divisors[i] = i % MAX_DIVISORS;

		phase[i] = constrain(phaseCv[i] * ARC_MAX_LEDS / 2048, 0, 63);
		pitchCvOut[i] = AnalogOut::create(INDEX_ANALOG_OUT[i*2+1]);
		cvOut[i] = AnalogOut::create(INDEX_ANALOG_OUT[i*2+2]);
		level[i] = 0;
		gateOutput[i] = INDEX_DIGITAL_OUT[i*2+1];
		triggerOutput[i] = INDEX_DIGITAL_OUT[i*2+2];
		triggerState[i] = 0;

		updateRing(i, false);
		delay(10); // give the ADC time to recover
	}
	
	/* Give it a moment... */
	delay(100);	
	IOUtils::displayBeat(scale, this);
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
		uint8_t cvIndex = readCvCounter % ARC_MAX_ENCODERS;
		if (readCvCounter < ARC_MAX_ENCODERS)
		{
			transposeCv[cvIndex] = aRead(transposeCvIn[cvIndex]);
			// Serial.println(analogRead(transposeCvIn[0]));
		}
		else if (readCvCounter < ARC_MAX_ENCODERS * 2)
		{
			int newPhaseCv = aRead(phaseCvIn[cvIndex]);
			if (abs(newPhaseCv - phaseCv[cvIndex]) > 35)
			{
				phaseCv[cvIndex] = newPhaseCv;
				phase[cvIndex] = constrain(phaseCv[cvIndex] * ARC_MAX_LEDS / 2048, 0, 63);
				updateRing(cvIndex, false);
				refresh = true;
			}
		}
		else
		{
			int newDivisor = aRead(divisorCvIn[cvIndex]) * MAX_DIVISORS / 2048;
			if (abs(newDivisor - divisorCv[cvIndex]) > 0)
			{
				divisorCv[cvIndex] = newDivisor;
				updateRing(cvIndex, false);
				refresh = true;
			}
		}
		
		readCvCounter = (readCvCounter + 1) % (ARC_MAX_ENCODERS * 3);
		readCvClockState = t + 10; // stagger reading CVs
	}

	for (int ring = 0; ring < ARC_MAX_ENCODERS; ring++)
	{
		if (triggerState[ring] && triggerState[ring] < t)
		{
			digitalWrite(triggerOutput[ring], LOW);
			triggerState[ring] = 0;
		}
	}

	if (!resetState && digitalRead(resetInput))
	{
		resetState = t + 20;
		counter = 0;
		for (int ring = 0; ring < ARC_MAX_ENCODERS; ring++)
			updateRing(ring, false);
		refresh = true;
	} 
	else if (resetState && (resetState < t) && !digitalRead(resetInput))
	{
		resetState = 0;
	}

	if (!scaleState && digitalRead(nextScaleInput))
	{
		scaleState = t + 20;
		scale = (scale + 1) % MAX_SCALES;
		IOUtils::displayBeat(scale, this);
	}
	else if (scaleState && (scaleState < t) && !digitalRead(nextScaleInput))
	{
		scaleState = 0;
	}

	if (this->clockInput != DIGITAL_IN_NONE && !this->clockState && digitalRead(this->clockInput))
	{
		this->clockState = t + 20;
		this->reset();
		refresh = true;
	}
	else if (this->clockInput != DIGITAL_IN_NONE && this->clockState && (this->clockState < t) && !digitalRead(this->clockInput))
	{
		this->clockState = 0;
	}
	
	if (refresh && isReady())
	{
		this->refreshArc();
		delay(2);
	}
}

void BinaryArc::reset()
{
	for (int ring = 0; ring < ARC_MAX_ENCODERS; ring++)
	{
		values[0][ring][counter] = prevValue[ring];
	}
	counter = (counter + 1) % ARC_MAX_LEDS;
	int totalTranspose, totalLevel;
	totalTranspose = totalLevel = 0;
	for (int ring = 0; ring < ARC_MAX_ENCODERS; ring++)
	{
		if ((counter + ARC_MAX_LEDS - phase[ring]) % getDivisor(ring) == 0)
		{
			flip[ring] = !flip[ring];
			int cv = getNoteCv(transposeCv[ring], flip[ring] ? level[ring] : 0);
			pitchCvOut[ring]->outputCV(cv);
			cvOut[ring]->outputCV(flip[ring] ? level[ring] * 273 : 0);
			digitalWrite(gateOutput[ring], flip[ring] ? HIGH : LOW);
			digitalWrite(triggerOutput[ring], HIGH);
			triggerState[ring] = currentTime + TRIGGER_LENGTH;
		}
		prevValue[ring] = values[0][ring][counter];
		values[0][ring][counter] = 15;
		if (digitalRead(sumInput[ring]))
		{
			totalTranspose += transposeCv[ring];
			totalLevel += flip[ring] ? level[ring] : 0;
		}
	}
	mainCvOut->outputCV(getNoteCv(totalTranspose, totalLevel));
}

void BinaryArc::updateRing(uint8_t ring, bool refresh)
{
	bool f = (phase[ring] / getDivisor(ring) % 2) == 0;
	for(int led = 0; led < ARC_MAX_LEDS; led++)
	{
		if ((led + ARC_MAX_LEDS - phase[ring]) % getDivisor(ring) == 0) f = !f;
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
	return constrain(analogRead(analogIn) - 2048 - 20, 0, 2047);
}

int BinaryArc::getNoteCv(int transpose, uint8_t level)
{
	int realLevel = transpose*16/2048 + level;
	return realLevel/SCALES[scale].length * 1000 + SEMITONE_MV[SCALES[scale].semis[realLevel%SCALES[scale].length]];
}

uint8_t BinaryArc::getDivisor(uint8_t ring)
{
	return divisor[(divisors[ring] + divisorCv[ring]) % MAX_DIVISORS];
}

void BinaryArc::encoderPositionChanged(uint8_t ring, int8_t delta)
{
	if (delta < 0)
	{
		deltaDivState += abs(delta);
		if (deltaDivState > divisionDelta)
		{
			deltaDivState = 0;
			divisors[ring] = (divisors[ring] + 1) % MAX_DIVISORS;
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

			int cv = getNoteCv(transposeCv[ring], flip[ring] ? level[ring] : 0);
			pitchCvOut[ring]->outputCV(cv);
			cvOut[ring]->outputCV(flip[ring] ? level[ring] * 273 : 0);
			
			int totalTranspose, totalLevel;
			totalTranspose = totalLevel = 0;
			for (int ring = 0; ring < ARC_MAX_ENCODERS; ring++)
				if (digitalRead(sumInput[ring]))
				{
					totalTranspose += transposeCv[ring];
					totalLevel += flip[ring] ? level[ring] : 0;
				}
			mainCvOut->outputCV(getNoteCv(totalTranspose, totalLevel));
		}
	}
	delay(1);
}

void BinaryArc::buttonPressed(uint8_t encoder) {}
void BinaryArc::buttonReleased(uint8_t encoder) {}
