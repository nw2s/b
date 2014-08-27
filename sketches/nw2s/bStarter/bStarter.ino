/* 

nw2s::b - A microcontroller-based modular synth control framework 
Copyright (C) 2013 Scott Wilson (thomas.scott.wilson@gmail.com) 

This program is free software: you can redistribute it and/or modify 
it under the terms of the GNU General Public License as published by 
the Free Software Foundation, either version 3 of the License, or 
(at your option) any later version. 

This program is distributed in the hope that it will be useful, 
but WITHOUT ANY WARRANTY; without even the implied warranty of 
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the 
GNU General Public License for more details. 

You should have received a copy of the GNU General Public License 
along with this program. If not, see <http://www.gnu.org/licenses/>. 

*/ 

#include <EventManager.h> 
#include <Key.h> 
#include <Trigger.h> 
#include <Clock.h> 
#include <Slew.h> 
#include <Oscillator.h> 
#include <Sequence.h> 
#include <IO.h> 
#include <SPI.h> 
#include <Wire.h> 
#include <SD.h> 
#include <ShiftRegister.h> 
#include <aJSON.h> 
#include <SDFirmware.h> 
#include <b.h>


using namespace nw2s; 

AnalogOut *out1; 

unsigned long t; 
int phase1; 


void setup() 
{ 
	Serial.begin(19200); 

	EventManager::initialize(); 
	
	b::cvGainMode = CV_GAIN_HIGH;
 
	/* Put your setup() code here */ 
	out1 = AnalogOut::create(DUE_SPI_4822_01); 

	phase1 = 0; 
	t = 0; 
} 

void loop() 
{ 
	/* Put your loop() code here */ 
	/* Do this once per millisecond */ 
	if (millis() > t) 
	{ 
		t = millis(); 
		int outputval = (phase1 * 100) - 5000; 

		out1->outputCV(outputval); 

		phase1 = (phase1 + 1) % 10000; 
	} 

	EventManager::loop(); 
} 