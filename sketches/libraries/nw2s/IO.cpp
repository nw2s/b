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



#include "mcp4822.h"
#include "IO.h"
#include <SPI.h>

using namespace nw2s;

AnalogOut* AnalogOut::create(PinAnalogOut out)
{
	return new AnalogOut(out);
}


AnalogOut::AnalogOut(PinAnalogOut pin)
{

	this->pin = pin;
	
#ifdef __AVR__

	/* Arcdcore runs on an AVR platform */
	if (pin == ARDCORE_DAC)
	{
  		for (int i=0; i<8; i++) 
		{
    		pinMode(ARDCORE_DAC_PIN_OFFSET + i, OUTPUT);
    		digitalWrite(ARDCORE_DAC_PIN_OFFSET + i, LOW);
		}
	}

#endif
	
#ifdef _SAM3XA_

	/* nw2s::b runs on a SAM platform */
	if ((pin >= DUE_SPI_4822_00) && (pin <= DUE_SPI_4822_15))
	{
		/* Calculate the CS and latch pins from the out pin */
		int cspin = ((pin - DUE_SPI_4822_PREFIX) / 2) + 2;
		int ldacpin = DUE_SPI_LATCH;
		this->spidac_index = pin % 2;
		
		Serial.print("\nUsing cspin = " + String(cspin));
		Serial.print("\nUsing ldacpin = " + String(ldacpin));
		Serial.print("\nUsing chip out = " + String(this->spidac_index));
		
		this->spidac = MCP4822(cspin,ldacpin);
		
	    this->spidac.begin();
	    SPI.setDataMode(SPI_MODE0);
	    SPI.setBitOrder(MSBFIRST);
	    SPI.setClockDivider(42);
	    SPI.begin();
		this->spidac.setGain1X_AB();			
		
	}

#endif
}

void AnalogOut::outputNoteCV(ScaleNote note)
{

#ifdef __AVR__

	if (pin == ARDCORE_DAC)
	{
		byte v = note.cv;
	
	  	PORTB = (PORTB & B11100000) | (v >> 3);
		PORTD = (PORTD & B00011111) | ((v & B00000111) << 5);
	}	

#endif

#ifdef _SAM3XA_

	if ((pin >= DUE_SPI_4822_00) && (pin <= DUE_SPI_4822_15))
	{
		Serial.print("\ncv: " + String(note.cv));
		Serial.print("\tcvin: " + String(note.cvin));
		Serial.print("\tindex: " + String(note.index));
		Serial.print("\to: " + String(note.octave));
		Serial.print("\td: " + String(note.degree));
		
		if (this->spidac_index == 0)
		{
			this->spidac.setValue_A(note.cv);
		}
		else
		{
			this->spidac.setValue_B(note.cv);
		}
	}

#endif 


}

void AnalogOut::outputSlewedNoteCV(ScaleNote note, Slew* slew)
{
	
#ifdef __AVR__
	
	if (pin == ARDCORE_DAC)
	{
		byte v = slew->calculate_value(note.cv);
	
	  	PORTB = (PORTB & B11100000) | (v >> 3);
		PORTD = (PORTD & B00011111) | ((v & B00000111) << 5);		
	}	
	
#endif

#ifdef _SAM3XA_

	int v = slew->calculate_value(note.cv);

	if ((pin >= DUE_SPI_4822_00) && (pin <= DUE_SPI_4822_15))
	{		
		if (this->spidac_index == 0)
		{
			this->spidac.setValue_A(v);
		}
		else
		{
			this->spidac.setValue_B(v);
		}
	}
		
#endif 

}


void AnalogOut::outputCV(int cv)
{

#ifdef __AVR__

	if (pin == ARDCORE_DAC)
	{
		byte v = cv;
	
		//Serial.print("\n" + String(v));
	
	  	PORTB = (PORTB & B11100000) | (v >> 3);
		PORTD = (PORTD & B00011111) | ((v & B00000111) << 5);
	}	

#endif

#ifdef _SAM3XA_

	if ((pin >= DUE_SPI_4822_00) && (pin <= DUE_SPI_4822_15))
	{
		if (this->spidac_index == 0)
		{
			this->spidac.setValue_A(cv);
		}
		else
		{
			this->spidac.setValue_B(cv);
		}
	}

#endif 


}

void IOUtils::setupPins()
{
#ifdef _SAM3XA_

	for (int pin = 22; pin <= 29; pin++)
	{
		pinMode(pin, OUTPUT);
		digitalWrite(pin, LOW);	
	}

	for (int pin = 42; pin <= 47; pin++)
	{
		pinMode(pin, OUTPUT);
	}

	for (int pin = 2; pin <= 8; pin++)
	{
		pinMode(pin, OUTPUT);
		digitalWrite(pin, LOW);	
	}

	//TODO: Can we design this pin out of the equation?
	/* Beat display LE */
	digitalWrite(47, LOW);
#endif
}


void IOUtils::displayBeat(int beat)
{
#ifdef _SAM3XA_
	//TODO: Optimize with register math
	digitalWrite(47, LOW);
	digitalWrite(43, (1 & beat) ? HIGH : LOW);
	digitalWrite(44, (2 & beat) ? HIGH : LOW);
	digitalWrite(45, (4 & beat) ? HIGH : LOW);
	digitalWrite(46, (8 & beat) ? HIGH : LOW);
	digitalWrite(47, HIGH);
#endif
}




