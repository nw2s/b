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

#include "b.h"
#include <Arduino.h>
#include <SPI.h>
#include <Wire.h>
#include "PCA9685.h"
#include "mcp4822.h"
#include "IO.h"

using namespace nw2s;


int nw2s::analogRead(int input)
{
	return 4096 - ::analogRead(input);
}

int nw2s::analogReadmV(int input)
{
	/* Get the value from ADC, inverted */
	int val = ::analogRead(input);

	/* Make sure we're 0 - 4095 */
	int normalval = (val > 4095) ? 4095 : ((val < 0) ? 0 : val);
	
	/* Convert to voltage */
	return ANALOG_INPUT_TRANSLATION[normalval];
}

int nw2s::analogReadmV(int input, int min, int max)
{
	int mv = analogReadmV(input);
	
	if (mv > max) return max;
	if (mv < min) return min;
	
	return mv;
}

AnalogOut* AnalogOut::create(PinAnalogOut out)
{
	return new AnalogOut(out);
}

PCA9685 AnalogOut::ledDriver;

AnalogOut::AnalogOut(PinAnalogOut pin)
{
	this->pin = pin;

	int cspin = INDEX_SPI_DAC_PIN[pin];
	int ldacpin = DUE_SPI_LATCH;

	this->spidac_index = INDEX_SPI_DAC_CHANNEL[pin];
	this->spidac = MCP4822(cspin,ldacpin);

	/* LED pin on PCA9685 is just the index */
	this->ledpin = pin;
	
    this->spidac.begin();
    SPI.setDataMode(SPI_MODE0);
    SPI.setBitOrder(MSBFIRST);
    SPI.setClockDivider(42);
    SPI.begin();
	
	this->spidac.setGain1X(this->spidac_index);						
}

void AnalogOut::outputCV(int cv)
{
	/* 

	   The dacval is calculated as the desired input voltage in millivolts, scaled to the
	   nearest even value (12 bit = 4000) 
	
	   Because the outputs are going through an inverting opamp, we then invert the signal
	   by subtracting the scaled number from 4096.

	*/

	int cv_old = cv;

	/* Scale the values if we're using software tuning */
	if (b::softTune)
	{
		cv = ((cv * b::scale[this->pin]) / TUNE_SCALE_FACTOR) + b::offset[this->pin];
	}

	int dacval = b::cvGainMode ? 
		(4095 - ((cv + 10000) * 4000UL) / 20000) : 
		(4095 - ((cv +  5000) * 4000UL) / 10000);


	// Grr... where is this - could be a single cycle operation!
	//dacval = __usat(dacval, 12);

	/* Make sure the values are in a 12bit unsigned range */
	dacval = (dacval < 0) ? 0 : (dacval > 4095) ? 4095 : dacval;
	
	this->spidac.setValue(this->spidac_index, dacval);

	if (b::debugMode) Serial.println("outputCV: " + String(dacval) + " " + String(cv) + " " + String((dacval < 2000) ? 4000 - (dacval * 2) : (dacval - 2000) * 2));

	if (IOUtils::enableLED)
	{
		int ledval = (cv_old == 0) ? 0 : (dacval < 2000) ? 4000 - (dacval * 2) : (dacval - 2000) * 2;
		
		AnalogOut::ledDriver.setLEDDimmed(this->ledpin, ledval);
	}
}

void AnalogOut::outputRaw(int x)
{
	/* Make sure the values are in a 12bit unsigned range */
	int dacval = (x < 0) ? 0 : (x > 4095) ? 4095 : x;
	
	this->spidac.setValue(this->spidac_index, dacval);

	if (IOUtils::enableLED)
	{
		int ledval = (dacval < 2048) ? 4096 - (dacval * 2) : (dacval - 2048) * 2;
		
		AnalogOut::ledDriver.setLEDDimmed(this->ledpin, ledval);
	}
	
}

void IOUtils::setupPins()
{
	Serial.println("Initializing...");
	
	Serial.println("digital output pins");
	for (int pin = 22; pin <= 37; pin++)
	{
		pinMode(pin, OUTPUT);
		digitalWrite(pin, LOW);	
	}
	
	Serial.println("digital input pins");
	for (int pin = 46; pin <= 53; pin++)
	{
		/* Setup the inputs */
		pinMode(pin, INPUT);
	}

	Serial.println("noise pin");
	pinMode(DUE_IN_DIGITAL_NOISE, INPUT);


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
	AnalogOut::ledDriver.begin(B000000, b::dimming);
	IOUtils::enableLED = AnalogOut::ledDriver.init();
	Serial.println("LED driver status: " + String(IOUtils::enableLED));

	/* Turn then all off */
	for (int i = 0; i < 16; i++) AnalogOut::ledDriver.setLEDOff(i);

	/* And spin through the LEDs once just to see them */
	for (int i = 0; i < 16; i++) 
	{
		AnalogOut::ledDriver.setLEDOn(i);
		digitalWrite(22 + (15 - i), HIGH);

		digitalWrite(45, LOW);
		digitalWrite(41, (1 & (15 - i)) ? HIGH : LOW);
		digitalWrite(42, (2 & (15 - i)) ? HIGH : LOW);
		digitalWrite(44, (4 & (15 - i)) ? HIGH : LOW);
		digitalWrite(43, (8 & (15 - i)) ? HIGH : LOW);
		digitalWrite(45, HIGH);
		
		/* Delay just a tad so we see an animation when it reboots */
		delay(10);

		AnalogOut::ledDriver.setLEDOff(i);
		digitalWrite(22 + (15 - i), LOW);
	}

	/* Then reset clock to 1 */	
	digitalWrite(45, LOW);
	digitalWrite(41, HIGH);
	digitalWrite(42, HIGH);
	digitalWrite(44, HIGH);
	digitalWrite(43, HIGH);
	digitalWrite(45, HIGH);
 
	Serial.println("done setting up.\n\n\n\n");

}

void* IOUtils::clockinstance = NULL;
bool IOUtils::enableLED = false;

void IOUtils::displayBeat(int beat, void* clockinstance)
{
	beat = (beat < 0) ? 0 : (beat > 15) ? 15 : beat;
	
	/* There's only one clock display, so only allow one instance to update the display. */
	if (IOUtils::clockinstance == NULL)
	{
		IOUtils::clockinstance = clockinstance;
	}
	else
	{
		if (clockinstance != IOUtils::clockinstance) return;
	}

	beat = 15 - beat;

	digitalWrite(45, LOW);
	digitalWrite(41, (1 & beat) ? HIGH : LOW);
	digitalWrite(42, (2 & beat) ? HIGH : LOW);
	digitalWrite(44, (4 & beat) ? HIGH : LOW);
	digitalWrite(43, (8 & beat) ? HIGH : LOW);
	digitalWrite(45, HIGH);
}




