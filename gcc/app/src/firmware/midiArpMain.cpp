/*

	nw2s::b - A microcontroller-based modular synth control framework
	Copyright (C) 2015 Scott Wilson (thomas.scott.wilson@gmail.com)

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
#include <Clock.h>
#include <USBMidi.h>

using namespace nw2s;

USBMidiApeggiator* usbDevice;


void setup()
{
	Serial.begin(19200);

	EventManager::initialize();
	
	/* Setup a variable clock */
	Clock* vclock = VariableClock::create(10, 240, DUE_IN_A00, 16);

	//std::vector<uint32_t> pattern;

	usbDevice = USBMidiApeggiator::create(DUE_OUT_D00, DUE_OUT_D01, DUE_SPI_4822_00, DUE_SPI_4822_02, DUE_SPI_4822_03, DUE_SPI_4822_04, DUE_IN_A01, NOTE_SORT_UPDOWN, DUE_IN_A02, DUE_IN_D0);
	
	/* Mod Wheel */
	usbDevice->addControlPin(1, DUE_SPI_4822_10, CC_RANGE_UNIPOLAR);
	
	usbDevice->setClockDivisionInput(DUE_IN_A03);

	EventManager::registerUsbDevice(usbDevice);
	EventManager::registerDevice(vclock);
	
	vclock->registerDevice(usbDevice);

	Serial.println("testing...");

	delay(200);
}

void loop()
{
	EventManager::loop();
	
	delay(1);	
}
