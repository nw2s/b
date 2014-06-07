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
#include <aJSON.h>

using namespace nw2s;


/* 
	This is a demo of some bitcode oscillating kind of like the equation composer module. Maby thanks for clone45 for most
	of the code that makes these work. You can see the original code here: https://github.com/clone45/EquationComposer
	and you can see more info about the module here: http://www.papernoise.net/microbe-modular-equation-composer/
*/


void setup() 
{
	Serial.begin(19200);
	Serial.print("\n\nStarting...\n");

	EventManager::initialize();

	VCO* bytebeat = ByteBeat::create(DUE_DAC0, DUE_IN_A00);
		
	EventManager::registerDevice(bytebeat);
}

void loop() 
{
	EventManager::loop();	
}




