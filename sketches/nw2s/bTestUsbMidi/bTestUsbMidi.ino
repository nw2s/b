
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
#include <Clock.h>
#include <IO.h>
#include <SPI.h>
#include <SignalData.h>
#include <SD.h>
#include <Loop.h>
#include <Wire.h>
#include <aJSON.h>
#include <Usb.h>
#include <UsbMidi.h>

using namespace nw2s;

USBMidiDevice* usbmidi;

// void MIDI_poll();
// void doDelay(unsigned long t1, unsigned long t2, unsigned long delayTime);

void setup()
{
    Serial.begin(9600);
	
	Serial.println("Starting...");
	
	usbmidi = new USBMidiDevice();
		
	delay(200);
}

void loop()
{
	unsigned long t1;

	//usbmidi->task();
	// t1 = micros();
	// if( Usb.getUsbTaskState() == USB_STATE_RUNNING )
	// {
	// 	MIDI_poll();
	// }
	//
	// doDelay(t1, micros(), 1000);
	
	EventManager::loop();
	
	delay(1);
	
}

// Poll USB MIDI Controler and send to serial MIDI
// void MIDI_poll()
// {
// 	byte outBuf[3];
// 	uint8_t size;
//
// 	do
// 	{
// 		if ((size = Midi.RecvData(outBuf)) > 0 )
// 		{
// 			if (size > 0) Serial.print(outBuf[0], HEX);
// 			if (size > 1) Serial.print(outBuf[1], HEX);
// 			if (size > 2) Serial.print(outBuf[2], HEX);
//
// 			Serial.println();
// 		}
// 	}
// 	while(size > 0);
// }

// Delay time (max 16383 us)
// void doDelay(unsigned long t1, unsigned long t2, unsigned long delayTime)
// {
//     unsigned long t3;
//
//     if( t1 > t2 ){
//       t3 = (4294967295 - t1 + t2);
//     }else{
//       t3 = t2 - t1;
//     }
//
//     if( t3 < delayTime ){
//       delayMicroseconds(delayTime - t3);
//     }
// }
