
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
	else if ((out == DUE_DAC0) || (out == DUE_DAC1))
	{
		analogWriteResolution(12);		
	}
}

void ArduinoIO::dacOutputNote(AnalogOut out, ScaleNote note)
{

#ifdef __AVR__

	if (out == ARDCORE_DAC)
	{
		byte v = note.cv8;
	
		Serial.print("\n- " + String(v));
				
	  	PORTB = (PORTB & B11100000) | (v >> 3);
		PORTD = (PORTD & B00011111) | ((v & B00000111) << 5);
	}	

#endif

#ifdef _SAM3XA_

	if (out == DUE_DAC0)
	{
		analogWrite(DAC0, note.cv12);
	}
	else if (out == DUE_DAC1)
	{
		analogWrite(DAC1, note.cv12);
	}

#endif 


}

void ArduinoIO::dacOutputSlewedNote(AnalogOut out, ScaleNote note, Slew* slew, int t)
{
	
#ifdef __AVR__
	
	if (out == ARDCORE_DAC)
	{
		byte v = slew->calculate_value(note.cv8, t);
	
		if (t % 10 == 0) Serial.print("\n- " + String(v));
	
	  	PORTB = (PORTB & B11100000) | (v >> 3);
		PORTD = (PORTD & B00011111) | ((v & B00000111) << 5);		
	}	
	
#endif

#ifdef _SAM3XA_

	int v = slew->calculate_value(note.cv12, t);

	if (t % 10 == 0) Serial.print("\n- " + String(v));

	if (out == DUE_DAC0)
	{
		analogWrite(DAC0, v);
	}
	else if (out == DUE_DAC1)
	{
		analogWrite(DAC1, v);
	}
		
#endif 

}
