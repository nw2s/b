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
