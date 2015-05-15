#include <GridTrigger.h>
#include <Usb.h>

#include <EventManager.h>
#include <Key.h>
#include <Trigger.h>
#include <Clock.h>
#include <Slew.h>
#include <Sequence.h>
#include <IO.h>
#include <SPI.h>
#include <SD.h>
#include <Wire.h>
#include <ShiftRegister.h>
#include <aJSON.h>
#include <USBMidi.h>

using namespace nw2s;

USBMidiDevice* usbDevice;


void setup()
{
	Serial.begin(9600);

	EventManager::initialize();
	
	usbDevice = new USBMidiDevice();

	EventManager::registerUsbDevice(usbDevice);

	Serial.println("testing...");

	delay(200);
}

void loop()
{
	EventManager::loop();

    if (usbDevice->isReady())
    {
		MIDI_poll();
    }

	delay(1);
}

void MIDI_poll()
{
    byte outBuf[3];
	uint32_t size = 0;
	
    do 
	{
		if ((size = usbDevice->RecvData(outBuf)) > 0)
		{
			Serial.print(outBuf[0], HEX);
			Serial.print(" ");
			Serial.print(outBuf[1], HEX);
			Serial.print(" ");
			Serial.println(outBuf[2], HEX);
		}
    }
	while(size > 0);
}