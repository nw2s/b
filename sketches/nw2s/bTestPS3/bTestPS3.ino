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

UsbPS3* ps3;

void setup()
{
	Serial.begin(9600);
	
	EventManager::initialize();

	ps3 = new UsbPS3();

	EventManager::registerUsbDevice(ps3);

	Serial.println("testing...");

	delay(200);
}

void loop()
{
	EventManager::loop();

	if (ps3->isReady())
	{
		ps3->setLedOn(LED1);
		delay(5);
		ps3->setLedOff(LED1);
		delay(5);
		ps3->setLedOn(LED2);
		delay(5);
		ps3->setLedOff(LED2);
		delay(5);
		ps3->setLedOn(LED3);
		delay(5);
		ps3->setLedOff(LED3);
		delay(5);
		ps3->setLedOn(LED4);
		delay(5);
		ps3->setLedOff(LED4);
		
		bool triangle = ps3->getButtonPress(TRIANGLE);
		
		digitalWrite(DUE_OUT_D00, triangle);
	}

	delay(1);

}
