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
