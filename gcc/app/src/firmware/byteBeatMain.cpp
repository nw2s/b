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

#include <EventManager.h>
#include <Oscillator.h>

using namespace nw2s;


/* 
	This is a demo of some bitcode oscillating kind of like the equation composer module. Many thanks for clone45 for most
	of the code that makes these work. You can see the original code here: https://github.com/clone45/EquationComposer
	and you can see more info about the module here: http://www.papernoise.net/microbe-modular-equation-composer/
*/


void setup() 
{
	Serial.begin(19200);
	Serial.print("\n\nStarting...\n");

	EventManager::initialize();

	VCO* bytebeat0 = ByteBeat::create(DUE_DAC0, DUE_IN_A00, 0, DUE_IN_A02, DUE_IN_A04, DUE_IN_A06, 0);
	VCO* bytebeat1 = ByteBeat::create(DUE_DAC1, DUE_IN_A00, 1, DUE_IN_A02, DUE_IN_A04, DUE_IN_A06, 0);
		
	EventManager::registerDevice(bytebeat0);
	EventManager::registerDevice(bytebeat1);
}

void loop() 
{
	EventManager::loop();	
}




