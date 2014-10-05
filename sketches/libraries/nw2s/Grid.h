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

/* 

	Note: This code is based on and would not be possible without the USB Host code
	which was partially ported to Arduino. I've taken their further development and 
	adapted it to enable Arduino Due to USB CDC class devices such as the Ardiono
	Leonardo. https://github.com/felis/USB_Host_Shield_2.0

	The other half of this code is based on and would not be possible without
	the development of the monome. http://monome.org/

*/

#ifndef GRID_H
#define GRID_H

#include "usb_ch9.h"
#include "Usb.h"
#include "confdescparser.h"

#define MAX_ENDPOINTS 3

namespace nw2s
{
	
	typedef struct 
	{
        uint32_t dwDTERate; 
        uint8_t bCharFormat; 
        uint8_t bParityType; 
        uint8_t bDataBits;
	} 
	LineCoding;

	class USBGrid;
	class USBGrid128;

}

class nw2s::USBGrid : public USBDeviceConfig, public UsbConfigXtracter
{
	protected:

		static const uint32_t epDataInIndex;			// DataIn endpoint index
		static const uint32_t epDataOutIndex;			// DataOUT endpoint index

		/* Mandatory members */
		USBHost		*pUsb;
		uint32_t	bAddress;							// Device USB address
		uint32_t	bConfNum;							// configuration number
		uint8_t 	bControlIface; 						// Control interface value
		uint32_t	bNumEP;								// total number of EP in the configuration
		bool		ready;

		/* Endpoint data structure describing the device EP */
		EpInfo		epInfo[MAX_ENDPOINTS];

	public:
		
		USBGrid();

		void task();

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

class nw2s::USBGrid128  : public USBGrid
{
	
};

#endif


