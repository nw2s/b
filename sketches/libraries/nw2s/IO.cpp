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
	if ((pin >= DUE_SPI_4822_14) && (pin <= DUE_SPI_4822_01))
	{
		/* Calculate the CS and latch pins from the out pin */
		int cspin = ((pin - DUE_SPI_4822_PREFIX) / 2) + 2;
		int ldacpin = DUE_SPI_LATCH;

		this->spidac_index = pin % 2;
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

	if ((pin >= DUE_SPI_4822_14) && (pin <= DUE_SPI_4822_01))
	{		
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

	if ((pin >= DUE_SPI_4822_14) && (pin <= DUE_SPI_4822_01))
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

	int dacval = (cv * 240UL) / 5000;

	if (pin == ARDCORE_DAC)
	{
		byte v = dacval;
		
	  	PORTB = (PORTB & B11100000) | (v >> 3);
		PORTD = (PORTD & B00011111) | ((v & B00000111) << 5);
	}	

#endif

#ifdef _SAM3XA_

	int dacval = (cv * 4000UL) / 5000;

	if ((pin >= DUE_SPI_4822_00) && (pin <= DUE_SPI_4822_15))
	{
		if (this->spidac_index == 0)
		{
			this->spidac.setValue_A(dacval);
		}
		else
		{
			this->spidac.setValue_B(dacval);
		}
	}

#endif 


}

void AnalogOut::outputSlewedCV(int cv, Slew* slew)
{

#ifdef __AVR__

	int slewval = slew->calculate_value(cv);
	int dacval = (slewval * 240UL) / 5000;

	if (pin == ARDCORE_DAC)
	{
		byte v = dacval;
		
	  	PORTB = (PORTB & B11100000) | (v >> 3);
		PORTD = (PORTD & B00011111) | ((v & B00000111) << 5);
	}	

#endif

#ifdef _SAM3XA_

	int slewval = slew->calculate_value(cv);
	int dacval = (slewval * 4000UL) / 5000;

	if ((pin >= DUE_SPI_4822_00) && (pin <= DUE_SPI_4822_15))
	{
		if (this->spidac_index == 0)
		{
			this->spidac.setValue_A(dacval);
		}
		else
		{
			this->spidac.setValue_B(dacval);
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

	for (int pin = 41; pin <= 45; pin++)
	{
		/* Set up the beatclock pins low */
		pinMode(pin, OUTPUT);
		digitalWrite(pin, LOW);
	}

	for (int pin = 2; pin <= 11; pin++)
	{
		/* Set up the DAC ~CS pins as high */
		pinMode(pin, OUTPUT);
		digitalWrite(pin, HIGH);	
	}

	/* Beat display LE */
	digitalWrite(47, LOW);
#endif
}

void* IOUtils::clockinstance = NULL;

void IOUtils::displayBeat(int beat, void* clockinstance)
{
#ifdef _SAM3XA_

	/* There's only one clock display, so only allow one instance to update the display. */
	if (IOUtils::clockinstance == NULL)
	{
		IOUtils::clockinstance = clockinstance;
	}
	else
	{
		if (clockinstance != IOUtils::clockinstance) return;
	}

	Serial.println(String(beat));

	digitalWrite(45, LOW);
	digitalWrite(41, (1 & beat) ? HIGH : LOW);
	digitalWrite(42, (2 & beat) ? HIGH : LOW);
	digitalWrite(44, (4 & beat) ? HIGH : LOW);
	digitalWrite(43, (8 & beat) ? HIGH : LOW);
	digitalWrite(45, HIGH);
#endif

}




