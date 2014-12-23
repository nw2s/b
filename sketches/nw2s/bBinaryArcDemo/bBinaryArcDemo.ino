/*
	BinaryArc - a binary sequencer for nw2s::b and monome arc
	copyright (c) 2014 scannerdarkly (fuzzybeacon@gmail.com)

	This code is developed for the the nw2s::b framework 
	Copyright (C) 2013 Scott Wilson (thomas.scott.wilson@gmail.com)

	Parts of it are also based on USB Host library 
	https://github.com/felis/USB_Host_Shield_2.0
	and the monome
	https://github.com/monome

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

#include <BinaryArc.h>
#include <Key.h>
#include <EventManager.h>
#include <IO.h>
#include <Trigger.h>
#include <Oscillator.h>
#include <Clock.h>
#include <Slew.h>
#include <Sequence.h>
#include <SPI.h>
#include <SD.h>
#include <Wire.h>
#include <aJSON.h>
#include <Gate.h>
#include <Grid.h>
#include <Usb.h>

using namespace nw2s;

BinaryArc* binaryArc;

void setup() 
{
    Serial.begin(19200);
  
    EventManager::initialize();
  
    binaryArc = BinaryArc::create(4, false);
    binaryArc->setClockInput(DUE_IN_D0);
    EventManager::registerDevice(binaryArc);
    EventManager::registerUsbDevice(binaryArc);
	
	/*
		Each ring represents a binary division. 
		Turning encoders CCW changes the division (1/2/4/8/16/32)
		Turning encoders CW changes the note (15 note range)

                Initial and still buggy implementation! But most things seem to work...
	*/
}

void loop()
{
    EventManager::loop();
    delay(5);
}





