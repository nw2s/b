/*
	support for monome arc
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

#ifndef ARC_H
#define ARC_H

#include "Usb.h"
#include "EventManager.h"
#include "confdescparser.h"
#include "../aJson/aJson.h"
#include "Grid.h"

#define bmREQ_FTDI_OUT  0x40
#define bmREQ_FTDI_IN   0xc0
#define FTDI_SIO_SET_BAUD_RATE 3
#define FTDI_SIO_SET_FLOW_CTRL 2
#define FTDI_SIO_DISABLE_FLOW_CTRL 0x0

#define ARC_MAX_PAGES 16
#define ARC_MAX_ENCODERS 4
#define ARC_MAX_LEDS 64


namespace nw2s
{
	class USBArc;
	class USBArcController;
}

class nw2s::USBArc : public USBDeviceConfig, public UsbConfigXtracter, public UsbBasedDevice
{
	protected:

		static const uint32_t epDataInIndex;			// DataIn endpoint index
		static const uint32_t epDataOutIndex;			// DataOUT endpoint index

		/* Mandatory members */
		// USBHost		*pUsb;
		uint32_t	bAddress;							// Device USB address
		uint32_t	bConfNum;							// configuration number
		uint8_t 	bControlIface; 						// Control interface value
		uint32_t	bNumEP;								// total number of EP in the configuration
		bool		ready;

		/* Endpoint data structure describing the device EP */
		EpInfo		epInfo[MAX_ENDPOINTS];

	public:
		
		USBArc();

		// virtual void task();

		/* Basic IO */
		uint32_t read(uint32_t *nreadbytes, uint32_t datalen, uint8_t *dataptr);
		uint32_t write(uint32_t datalen, uint8_t *dataptr);

		/* Line control setup */
        uint8_t setLineCoding(const LineCoding *dataptr);
        uint8_t setControlLineState(uint8_t state);

		/* USBDeviceConfig implementation */
		virtual uint32_t Init(uint32_t parent, uint32_t port, uint32_t lowspeed);
		virtual uint32_t Release();
		virtual uint32_t Poll() { return 0; };	// not implemented
		virtual uint32_t GetAddress() { return bAddress; };
		virtual bool isReady() { return ready; };

		/* UsbConfigXtracter implementation */
		virtual void EndpointXtract(uint32_t conf, uint32_t iface, uint32_t alt, uint32_t proto, const USB_ENDPOINT_DESCRIPTOR *ep);
};

class nw2s::USBArcController : public USBArc
{
	protected:
		
		uint8_t encoderCount;
		bool pushButton = false;
		
		uint8_t beat = 0;
		
		bool arcInitialized = false;
		uint8_t values[ARC_MAX_PAGES][ARC_MAX_ENCODERS][ARC_MAX_LEDS];
		int8_t lastDelta[ARC_MAX_ENCODERS];
		uint8_t lastPressed = 0;
		uint8_t lastReleased = 0;
		uint8_t currentPage = 0;
			
		virtual void encoderPositionChanged(uint8_t ring, int8_t delta) = 0;
		virtual void buttonPressed(uint8_t encoder) = 0;
		virtual void buttonReleased(uint8_t encoder) = 0;

	public:

		USBArcController(uint8_t encoderCount, bool pushButton);
		
		virtual void task();

		uint8_t getEncoderCount();
		bool pushButtonSupported();
		uint8_t getValue(uint8_t page, uint8_t ring, uint8_t led);

		void switchPage(uint8_t page);
		void refreshArc();

		void setLED(uint8_t page, uint8_t ring, uint8_t led, uint8_t value);
		void clearLED(uint8_t page, uint8_t ring, uint8_t led);
		void setRing(uint8_t page, uint8_t ring, uint8_t value);
		void clearRing(uint8_t page, uint8_t ring);
		void setRange(uint8_t page, uint8_t ring, uint8_t startLed, uint8_t endLed, uint8_t value);
		void clearRange(uint8_t page, uint8_t ring, uint8_t startLed, uint8_t endLed);
		
};

#endif


