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
#include "EventManager.h"
#include "IO.h"
#include <Arduino.h>
#include <Reset.h>
#include <Usb.h>

using namespace std;
using namespace nw2s;

volatile unsigned long EventManager::t = 0UL;
vector<TimeBasedDevice *> EventManager::timedevices;
UsbBasedDevice* EventManager::usbDevice = NULL;

/* SERIAL COMMAND PROCESSING */
String inputString = "";         
bool stringComplete = false;

void UsbBasedDevice::task()
{
	this->pUsb->Task();
}

void EventManager::initialize()
{
	IOUtils::setupPins();
}

void EventManager::loop()
{
	/* This gets run every loop() call, but we only want to */
	/* fire events if the clock has changed. */
	unsigned long current_time = millis();
	
	if (t != current_time)
	{ 		
		t = current_time;
				
		for (int i = 0; i < EventManager::timedevices.size(); i++)
		{
			EventManager::timedevices[i]->timer(EventManager::t);	
		}
	}
	
	if (usbDevice != NULL)
	{
		usbDevice->task();
	}

	if (stringComplete)
	{
		if (inputString == "ERASEANDRESET")
		{
			Serial.println("Received command: ERASEANDRESET");
			initiateReset(1);
			tickReset();
		}
		else if (inputString == "DEBUG ON")
		{
			Serial.println("Received command: " + inputString);
			b::debugMode = true;
		}
		else if (inputString == "DEBUG OFF")
		{
			Serial.println("Received command: " + inputString);
			b::debugMode = false;
		}
		else
		{
			Serial.println("Unknown command: " + inputString);
		}
		
		inputString = "";
		stringComplete = false;
	}
}

void EventManager::registerDevice(TimeBasedDevice* device)
{
	timedevices.push_back(device);
}

void EventManager::registerUsbDevice(UsbBasedDevice* device)
{
	EventManager::usbDevice = device;
}

unsigned long EventManager::getT()
{
	return t;
}

void serialEvent() 
{
	while (Serial.available()) 
	{
    	char c = (char)Serial.read(); 

	    if (c == '\n') 
		{
	    	stringComplete = true;
	    } 
		else
		{
			inputString += c;			
		}
  	}
}




