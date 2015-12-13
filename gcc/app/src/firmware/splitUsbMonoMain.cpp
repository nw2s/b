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

#include <Usb.h>
#include <EventManager.h>
#include <USBMidi.h>

using namespace nw2s;

USBSplitMonoMidiController* usbDevice;


void setup()
{
	Serial.begin(19200);

	EventManager::initialize();
	
	usbDevice = USBSplitMonoMidiController::create(DUE_OUT_D00, DUE_OUT_D01, DUE_OUT_D02, DUE_SPI_4822_00, DUE_SPI_4822_02, DUE_SPI_4822_03, DUE_OUT_D05, DUE_OUT_D06, DUE_OUT_D07, DUE_SPI_4822_05, DUE_SPI_4822_06, DUE_SPI_4822_07, DUE_SPI_4822_04, 48);

	EventManager::registerUsbDevice(usbDevice);
	EventManager::registerDevice(usbDevice);

	Serial.println("testing...");

	delay(200);
}

void loop()
{
	EventManager::loop();
	
	delay(1);
	
	if (millis() % 1000 == 0) Serial.print("x");
}
