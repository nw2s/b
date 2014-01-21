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

#include <Key.h>
#include <EventManager.h>
#include <Trigger.h>
#include <Clock.h>
#include <Slew.h>
#include <Sequence.h>
#include <IO.h>
#include <SPI.h>
#include <SD.h>
#include <Wire.h>
#include <ShiftRegister.h>
#include <Oscillator.h>

using namespace nw2s;


/* 
	This simple demo just sets up two independent saw oscillators on DAC1 and DAC2.
	This can be used as either a starting point for your own oscillators or can be used
	to set the output gain of the AC coupled DAC circuits.
	
*/


void setup() 
{
	Serial.begin(19200);
	Serial.print("\n\nStarting...\n");

	EventManager::initialize();

	VCO* noise1 = DiscreteNoise::create(DUE_DAC0, DUE_IN_A00);
	VCO* noise2 = DiscreteNoise::create(DUE_DAC1, DUE_IN_A01);
		
	EventManager::registerDevice(noise1);
	EventManager::registerDevice(noise2);
}

void loop() 
{
	EventManager::loop();	
}




