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

//define MAX_ENDPOINTS 3

#define bmREQ_FTDI_OUT  0x40
#define bmREQ_FTDI_IN   0xc0
#define FTDI_SIO_SET_BAUD_RATE 3
#define FTDI_SIO_SET_FLOW_CTRL 2
#define FTDI_SIO_DISABLE_FLOW_CTRL 0x0


namespace nw2s
{
	enum GridDevice {
		
		DEVICE_40H_TRELLIS = 0,
		DEVICE_SERIES = 1,
		DEVICE_GRIDS = 2
	};
	
	typedef struct 
	{
        uint32_t dwDTERate; 
        uint8_t bCharFormat; 
        uint8_t bParityType; 
        uint8_t bDataBits;
	} 
	LineCoding;

	class USBGrid;
	class USBGridController;

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
		
		USBGrid(GridDevice deviceType);

		virtual void task();

		/* Support multiple types, but you have to tell me what it is */
		GridDevice deviceType = DEVICE_40H_TRELLIS;

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

class nw2s::USBGridController : public USBGrid
{
	protected:
		
		uint8_t beat = 0;
		uint8_t columnCount;
		uint8_t rowCount;
		uint8_t pages = 16;
		
		/* Just allocate the max value and work with it - [page][column][row] */
		uint8_t cells[16][16][16];
				
		bool gridInitialized = false;
		bool memoryInitialized = false;
		uint8_t lastpress[2] = {0, 0};
		uint8_t lastrelease[2] = {0, 0};
		uint8_t currentPage = 0;

		void setLED(uint8_t page, uint8_t column, uint8_t row, uint8_t value);
		void clearLED(uint8_t page, uint8_t column, uint8_t row);
		void switchPage(uint8_t page);
		void refreshGrid();
			
		virtual void buttonPressed(uint8_t column, uint8_t row) = 0;
		virtual void buttonReleased(uint8_t column, uint8_t row) = 0;

	public:

		USBGridController(GridDevice deviceType, uint8_t columns, uint8_t rows);

		virtual void task();

		uint8_t getColumnCount();
		uint8_t getRowCount();
		uint8_t getValue(uint8_t page, uint8_t column, uint8_t row);
		uint8_t getColumn(uint8_t column);
		
		
};

#endif


