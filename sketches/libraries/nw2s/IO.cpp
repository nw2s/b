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

#include <Arduino.h>
#include <SPI.h>
#include <Wire.h>
#include "PCA9685.h"
#include "mcp4822.h"
#include "IO.h"

using namespace nw2s;

AnalogOut* AnalogOut::create(PinAnalogOut out)
{
	return new AnalogOut(out);
}

PCA9685 AnalogOut::ledDriver;

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
		
		/* Calculate the LED pin on the PCA9685 driver */
		switch(pin)
		{
			case DUE_SPI_4822_00:
				this->ledpin = 0;
				break;
			case DUE_SPI_4822_01:
				this->ledpin = 1;
				break;
			case DUE_SPI_4822_02:
				this->ledpin = 2;
				break;
			case DUE_SPI_4822_03:
				this->ledpin = 3;
				break;
			case DUE_SPI_4822_04:
				this->ledpin = 4;
				break;
			case DUE_SPI_4822_05:
				this->ledpin = 5;
				break;
			case DUE_SPI_4822_06:
				this->ledpin = 6;
				break;
			case DUE_SPI_4822_07:
				this->ledpin = 7;
				break;
			case DUE_SPI_4822_08:
				this->ledpin = 8;
				break;
			case DUE_SPI_4822_09:
				this->ledpin = 9;
				break;
			case DUE_SPI_4822_10:
				this->ledpin = 10;
				break;
			case DUE_SPI_4822_11:
				this->ledpin = 11;
				break;
			case DUE_SPI_4822_12:
				this->ledpin = 12;
				break;
			case DUE_SPI_4822_13:
				this->ledpin = 13;
				break;
			case DUE_SPI_4822_14:
				this->ledpin = 14;
				break;
			case DUE_SPI_4822_15:
				this->ledpin = 15;
				break;
		}
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
		if (IOUtils::enableLED) AnalogOut::ledDriver.setLEDDimmed(this->ledpin, note.cv);
						
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

	if (IOUtils::enableLED) AnalogOut::ledDriver.setLEDDimmed(this->ledpin, v);

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

	if (IOUtils::enableLED) AnalogOut::ledDriver.setLEDDimmed(this->ledpin, dacval);

	if ((pin >= DUE_SPI_4822_14) && (pin <= DUE_SPI_4822_01))
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

	if (IOUtils::enableLED) AnalogOut::ledDriver.setLEDDimmed(this->ledpin, dacval);

	if ((pin >= DUE_SPI_4822_14) && (pin <= DUE_SPI_4822_01))
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

	Serial.println("Initializing...");
	
	Serial.println("digital output pins");
	for (int pin = 22; pin <= 37; pin++)
	{
		pinMode(pin, OUTPUT);
		digitalWrite(pin, LOW);	
	}

	Serial.println("beatclock pins");
	for (int pin = 41; pin <= 45; pin++)
	{
		/* Set up the beatclock pins low */
		pinMode(pin, OUTPUT);
		digitalWrite(pin, LOW);
	}
	
	

	Serial.println("DAC CS pins");
	for (int pin = 2; pin <= 11; pin++)
	{
		/* Set up the DAC ~CS pins as high */
		pinMode(pin, OUTPUT);
		digitalWrite(pin, HIGH);	
	}

	/* Beat display LE */
	Serial.println("enable beat display");
	digitalWrite(47, LOW);	

	/* Analog Read Resolution */
	Serial.println("analog read resolution: 12");
	analogReadResolution(12);

	/* Setup the I2C bus and LED driver */
	Wire1.begin();
	AnalogOut::ledDriver.begin(B001000);
	IOUtils::enableLED = AnalogOut::ledDriver.init();
	Serial.println("LED driver status: " + String(IOUtils::enableLED));

	/* Turn then all off */
	for (int i = 0; i < 16; i++) AnalogOut::ledDriver.setLEDOff(i);

	/* And spin through the LEDs once just to see them */
	for (int i = 0; i < 16; i++) 
	{
		// AnalogOut::ledDriver.setLEDOn(i);
		digitalWrite(22 + i, HIGH);

		digitalWrite(45, LOW);
		digitalWrite(41, (1 & (16 - i)) ? HIGH : LOW);
		digitalWrite(42, (2 & (16 - i)) ? HIGH : LOW);
		digitalWrite(44, (4 & (16 - i)) ? HIGH : LOW);
		digitalWrite(43, (8 & (16 - i)) ? HIGH : LOW);
		digitalWrite(45, HIGH);

		delay(10);

		// AnalogOut::ledDriver.setLEDOff(i);
		digitalWrite(22 + i, LOW);
	}

	/* Then reset clock to 1 */	
	digitalWrite(45, LOW);
	digitalWrite(41, LOW);
	digitalWrite(42, LOW);
	digitalWrite(44, LOW);
	digitalWrite(43, LOW);
	digitalWrite(45, HIGH);
 
	Serial.println("done setting up.\n\n\n\n");

#endif
}

void* IOUtils::clockinstance = NULL;
bool IOUtils::enableLED = false;

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

	beat = 16 - beat;

	digitalWrite(45, LOW);
	digitalWrite(41, (1 & beat) ? HIGH : LOW);
	digitalWrite(42, (2 & beat) ? HIGH : LOW);
	digitalWrite(44, (4 & beat) ? HIGH : LOW);
	digitalWrite(43, (8 & beat) ? HIGH : LOW);
	digitalWrite(45, HIGH);
#endif

}




