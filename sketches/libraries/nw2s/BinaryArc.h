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

#ifndef BINARYARC_H
#define BINARYARC_H

#include "b.h"
#include "Arc.h"
#include "EventManager.h"
#include "../aJson/aJson.h"
#include "IO.h"
#include "Clock.h"
#include "Gate.h"
#include "JSONUtil.h"

#define MAX_DIVISORS 6
#define MAX_SCALES 16

namespace nw2s
{
	class BinaryArc;
}

class nw2s::BinaryArc : public BeatDevice, public USBArcController
{
	public:

		static BinaryArc* create(uint8_t encoderCount, bool pushButton);
		static BinaryArc* create(aJsonObject* data);

		virtual void timer(unsigned long t);
		virtual void reset();

		void setClockInput(PinDigitalIn input);	

	protected:
		
		virtual void encoderPositionChanged(uint8_t ring, int8_t delta);
		virtual void buttonPressed(uint8_t encoder);
		virtual void buttonReleased(uint8_t encoder);

	private:
	
		// Phrygian dominant scale
		int SCALE[16] = {0, 83, 333, 417, 583, 667, 833, 1000, 1083, 1333, 1417, 1583, 1667, 1833, 2000, 2083};
	
		BinaryArc(uint8_t encoderCount, bool pushButton);
		void updateRing(uint8_t ring, bool refresh);
		int aRead(PinAnalogIn analogIn);
		
		PinDigitalIn clockInput	= DUE_IN_D0;
		PinDigitalIn resetInput = DUE_IN_D1;
		PinDigitalIn nextScaleInput = DUE_IN_D2;
		PinDigitalIn reservedInput = DUE_IN_D3;
		PinDigitalIn sumInput[4] = {DUE_IN_D4, DUE_IN_D5, DUE_IN_D6, DUE_IN_D7};
		
		PinDigitalOut gateOutput[ARC_MAX_ENCODERS];
		PinDigitalOut triggerOutput[ARC_MAX_ENCODERS];
		
		PinAnalogIn phaseCvIn[ARC_MAX_ENCODERS];
		PinAnalogIn transposeCvIn[ARC_MAX_ENCODERS];
		PinAnalogIn divisorCvIn[ARC_MAX_ENCODERS];
		
		AnalogOut* pitchCvOut[ARC_MAX_ENCODERS];
		AnalogOut* cvOut[ARC_MAX_ENCODERS];
		AnalogOut* mainCvOut;
		
		uint8_t divisor[MAX_DIVISORS] = {1, 2, 4, 8, 16, 32};
		unsigned long currentTime = 0;
		unsigned long clockState = 0;
		unsigned long resetState = 0;
		unsigned long scaleState = 0;
		unsigned long readCvClockState = 0;
		uint8_t readCvCounter = 0;
		unsigned long triggerState[ARC_MAX_ENCODERS];
		uint8_t counter = 0;
		uint8_t scale = 0;
		uint8_t divisors[ARC_MAX_ENCODERS];
		uint8_t level[ARC_MAX_ENCODERS];
		bool flip[ARC_MAX_ENCODERS];
		uint8_t prevValue[ARC_MAX_ENCODERS];
		uint8_t phase[ARC_MAX_ENCODERS];
		int phaseCv[ARC_MAX_ENCODERS];
		int transposeCv[ARC_MAX_ENCODERS];
		
		uint8_t divisionDelta = 40;
		uint8_t voltageDelta = 25;
		uint8_t deltaDivState = 0;
		uint8_t deltaVolState = 0;
};

#endif
