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

USBMonophonicMidiController* usbDevice;


void setup()
{
	Serial.begin(19200);

	EventManager::initialize();
	
	usbDevice = USBMonophonicMidiController::create(DUE_OUT_D00, DUE_OUT_D01, DUE_OUT_D02, DUE_SPI_4822_00, DUE_SPI_4822_03, DUE_SPI_4822_02, DUE_SPI_4822_03);

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
