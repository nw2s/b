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
#include "mcp4822.h"

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
#ifdef _SAM3XA_
		analogWriteResolution(12);		
#endif
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
