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
#include <UsbPS3.h>

using namespace nw2s;

void setup()
{
	Serial.begin(9600);
	
	EventManager::initialize();

	UsbPS3CV* ps3 = UsbPS3CV::create(true);

	EventManager::registerUsbDevice(ps3);
	EventManager::registerDevice(ps3);

	Serial.println("Running...");

	delay(200);
}

void loop()
{
	EventManager::loop();

	delay(1);
}
