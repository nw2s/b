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
#include <SignalData.h>
#include <Loop.h>

using namespace nw2s;


void setup() 
{
	Serial.begin(19200);
	Serial.print("\n\nStarting...\n");

	EventManager::initialize();

	EFLooper* lfo1 = EFLooper::create(DUE_SPI_4822_15, DUE_IN_A00, DUE_IN_A02, DUE_IN_A04, "noise", "rain.raw");

	EFLooper* lfo2 = EFLooper::create(DUE_SPI_4822_13, DUE_IN_A01, DUE_IN_A03, DUE_IN_A05, "melodic", "ebgtr120.raw");

	EFLooper* lfo3 = EFLooper::create(DUE_SPI_4822_14, DUE_IN_A06, DUE_IN_A08, DUE_IN_A10, "melodic", "musicbox.raw");

	EFLooper* lfo4 = EFLooper::create(DUE_SPI_4822_12, DUE_IN_A07, DUE_IN_A09, DUE_IN_A11, "drum", "bongo120.raw");

	EventManager::registerDevice(lfo1);
	EventManager::registerDevice(lfo2);
	EventManager::registerDevice(lfo3);
	EventManager::registerDevice(lfo4);
}

void loop() 
{
	EventManager::loop();	
}




