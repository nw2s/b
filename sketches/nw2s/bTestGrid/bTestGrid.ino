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


using namespace nw2s;

//USBGrid* grid;
GridTriggerController* grid;

void setup()
{
	Serial.begin(9600);
	
	//grid = new USBGrid(DEVICE_SERIES);
	grid = GridTriggerController::create(DEVICE_SERIES, 8, 8, DIV_SIXTEENTH, DIGITAL_OUT_NONE, DIGITAL_OUT_NONE, DIGITAL_OUT_NONE, DIGITAL_OUT_NONE, DIGITAL_OUT_NONE, DIGITAL_OUT_NONE, DIGITAL_OUT_NONE);

	Serial.println("testing...");

	delay(200);
}

#define RCVSIZE 64

void loop()
{
  // uint8_t buf[RCVSIZE];
  // uint32_t nbread = 0;
	//   uint8_t helloworld[] = { 0x80, 0x00,
	//   							0x81, 0x00,
	// 						0x82, 0x00,
	// 						0x83, 0x00,
	// 						0x84, 0x00,
	// 						0x85, 0x00,
	// 						0x86, 0x00,
	// 						0x87, 0x00,
	// 						0x88, 0x00,
	// 						0x89, 0x00,
	// 						0x8A, 0x00,
	// 						0x8B, 0x00,
	// 						0x8C, 0x00,
	// 						0x8D, 0x00,
	// 						0x8E, 0x00,
	// 						0x8F, 0x00 };
	//
	//
	// uint8_t columns128[] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

  grid->task();
  grid->timer(millis());
  delay(1);

	//   if (grid->isReady())
	//   {
	// //   for (int i = 0; i < 8; i++)
	// //   {
	// // 	  helloworld[(i * 2) + 1] = 1 << random(8);
	// //   }
	// //
	// //     /* Write hello string to ADK */
	// // grid->write(32, helloworld);
	//
	//     delay(100);
	//
	//     /* Read data from ADK and print to UART */
	//     int rcode = grid->read(&nbread, RCVSIZE, buf);
	//
	// if (rcode > 1)
	// {
	// 	Serial.print("Read error: ");
	// 	Serial.println(rcode, HEX);
	// }
	//
	//     if (nbread > 0)
	//     {
	//       Serial.print("RCV: ");
	//
	//   for (uint32_t i = 0; i < nbread; ++i)
	//       {
	//         Serial.print(buf[i] >> 4, HEX);
	// 	Serial.print(" ");
	// 	Serial.print(buf[i] & 0x0F, HEX);
	// 	Serial.print("      ");
	//       }
	//
	//       Serial.println();
	//     }
	//   }
}
