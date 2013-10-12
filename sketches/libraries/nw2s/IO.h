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

#ifndef IO_h
#define IO_h

#include "Key.h"
#include "Slew.h"
#include "mcp4822.h"
#include <Arduino.h>

namespace nw2s
{
	
#ifdef __AVR__

	enum PinAnalogIn
	{
		ARDCORE_IN_A0 = A0,
		ARDCORE_IN_A1 = A1,
		ARDCORE_IN_A2 = A2,
		ARDCORE_IN_A3 = A3,
		ARDCORE_IN_A4 = A4,
		ARDCORE_IN_A5 = A5,
	};

	enum PinDigitalOut
	{
		ARDCORE_CLOCK_IN = 2,
		ARDCORE_OUT_D0 = 3,
		ARDCORE_OUT_D1 = 4,
		ARDCORE_OUT_PIN13 = 13,
		ARDCORE_DAC_PIN_OFFSET = 5,
		
		DIGITAL_OUT_NONE = -1,
	};

	

	enum PinAnalogOut
	{
		ARDCORE_DAC = -1,
		ANALOG_OUT_NONE = -2,		
	};

#endif

#ifdef _SAM3XA_

	enum PinDigitalOut
	{
		DUE_OUT_D0 = 22,
		DUE_OUT_D1 = 23,
		DUE_OUT_D2 = 24,
		DUE_OUT_D3 = 25,
		DUE_OUT_D4 = 26,
		DUE_OUT_D5 = 27,
		DUE_OUT_D6 = 28,
		DUE_OUT_D7 = 29,
		
		DIGITAL_OUT_NONE = -1,
	};

	enum PinAnalogIn
	{
		DUE_IN_A00 = A0,
		DUE_IN_A01 = A1,
		DUE_IN_A02 = A2,
		DUE_IN_A03 = A3,
		DUE_IN_A04 = A4,
		DUE_IN_A05 = A5,
		DUE_IN_A06 = A6,
		DUE_IN_A07 = A7,
		DUE_IN_A08 = A8,
		DUE_IN_A09 = A9,
		DUE_IN_A10 = A10,
		DUE_IN_A11 = A11,
	};

	enum PinAnalogOut
	{
		ANALOG_OUT_NONE = -2,
		
		DUE_SPI_4822_PREFIX = 1000,
		DUE_SPI_4822_0 = 1030,
		DUE_SPI_4822_1 = 1031,
		DUE_SPI_4822_2 = 1032,
		DUE_SPI_4822_3 = 1033,
		DUE_SPI_4822_4 = 1034,
		DUE_SPI_4822_5 = 1035,
		DUE_SPI_4822_6 = 1036,
		DUE_SPI_4822_7 = 1037,
		DUE_SPI_4822_8 = 1038,
		DUE_SPI_4822_9 = 1039,
		DUE_SPI_4822_10 = 1040,
		DUE_SPI_4822_11 = 1041,
		DUE_SPI_4822_12 = 1042,
		DUE_SPI_4822_13 = 1043,
		DUE_SPI_4822_14 = 1044,
		DUE_SPI_4822_15 = 1045,
	};


#endif

	class AnalogOut;
	
}

class nw2s::AnalogOut
{
	public:
		static nw2s::AnalogOut* create(PinAnalogOut pin);
		void outputNoteCV(ScaleNote note);
		void outputSlewedNoteCV(ScaleNote note, Slew* slew, int t);
		
	private:
		PinAnalogOut pin;
		AnalogOut(PinAnalogOut out);

#ifdef _SAM3XA_
		MCP4822 spidac;
		int spidac_index;
#endif

};

#endif
