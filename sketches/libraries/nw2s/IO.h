
#ifndef IO_h
#define IO_h

#include "Key.h"
#include "Slew.h"
#include <Arduino.h>

// #ifndef PORTB
// #define PORTB 0
// #endif
// 
// #ifndef PORTD
// #define PORTD 0
// #endif

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
		
		ANALOG_OUT_NONE = -2,
	};
	
	const int TRIGGER_TIME = 25;        // 25 ms trigger timing


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
