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
#include <Arduino.h>

namespace nw2s
{
	enum AnalogIn
	{
		ARDCORE_IN_A0 = 0,
		ARDCORE_IN_A1 = 1,
		ARDCORE_IN_A2 = 2,
		ARDCORE_IN_A3 = 3,
		ARDCORE_IN_A4 = 4,
		ARDCORE_IN_A5 = 5,
	};
	
	enum DigitalOut
	{
		ARDCORE_CLOCK_IN = 2,
		ARDCORE_OUT_D0 = 3,
		ARDCORE_OUT_D1 = 4,
		ARDCORE_OUT_PIN13 = 13,
		ARDCORE_DAC_PIN_OFFSET = 5,
		
		DIGITAL_OUT_NONE = -1,
	};

	enum AnalogOut
	{
		ARDCORE_DAC = -1,
		DUE_DAC0 = -3,
		DUE_DAC1 = -4,
		B_DAC0 = -5,
		ANALOG_OUT_NONE = -2,
		
		DUE_SPI_4822_0 = 1000,
		DUE_SPI_4822_1 = 1001,
		DUE_SPI_4822_2 = 1002,
		DUE_SPI_4822_3 = 1003,
		DUE_SPI_4822_4 = 1004,
		DUE_SPI_4822_5 = 1005,
		DUE_SPI_4822_6 = 1006,
		DUE_SPI_4822_7 = 1007,
		DUE_SPI_4822_8 = 1008,
		DUE_SPI_4822_9 = 1009,
		DUE_SPI_4822_10 = 1010,
		DUE_SPI_4822_11 = 1011,
		DUE_SPI_4822_12 = 1012,
		DUE_SPI_4822_13 = 1013,
		DUE_SPI_4822_14 = 1014,
		DUE_SPI_4822_15 = 1015,
	};
	
	const int TRIGGER_TIME = 25;

	class ArduinoIO;
}

class nw2s::ArduinoIO
{
	public:
		static void dacOutputInitialize(AnalogOut out);
		static void dacOutputNote(AnalogOut out, ScaleNote note);
		static void dacOutputSlewedNote(AnalogOut out, ScaleNote note, Slew* slew, int t);
};

#endif
