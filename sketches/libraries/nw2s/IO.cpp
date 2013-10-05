
#include "IO.h"


using namespace nw2s;

void ArduinoIO::dacOutputInitialize(AnalogOut out)
{
	if (out == ARDCORE_DAC)
	{
  		for (int i=0; i<8; i++) 
		{
    		pinMode(ARDCORE_DAC_PIN_OFFSET + i, OUTPUT);
    		digitalWrite(ARDCORE_DAC_PIN_OFFSET + i, LOW);
		}
	}
}

void ArduinoIO::dacOutputNote(AnalogOut out, ScaleNote note)
{
	if (out == ARDCORE_DAC)
	{
		byte v = note.cv8;

		Serial.print("\n- " + String(v));
				
	  	PORTB = (PORTB & B11100000) | (v >> 3);
		PORTD = (PORTD & B00011111) | ((v & B00000111) << 5);
	}	
}

void ArduinoIO::dacOutputSlewedNote(AnalogOut out, ScaleNote note, Slew* slew, int t)
{
	if (out == ARDCORE_DAC)
	{
		byte v = slew->calculate_value(note.cv8, t);

		if (t % 10 == 0) Serial.print("\n- " + String(v));

	  	PORTB = (PORTB & B11100000) | (v >> 3);
		PORTD = (PORTD & B00011111) | ((v & B00000111) << 5);		
	}	
}
