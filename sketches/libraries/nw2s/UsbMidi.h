/*

	nw2s::b - A microcontroller-based modular synth control framework
	Copyright (C) 2015 Scott Wilson (thomas.scott.wilson@gmail.com)

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

	Based on code originally from
	USB-MIDI class driver for USB Host Shield 2.0 Library
	which is opyright 2012-2014 Yuuichi Akagawa

	Which in turn was based on an idea from 
	LPK25 USB-MIDI to Serial MIDI converter
	by Collin Cunningham - makezine.com, narbotic.com

*/

#ifndef USBMIDI_H
#define USBMIDI_H

#include "Usb.h"
#include "EventManager.h"
#include "confdescparser.h"
#include "../aJson/aJson.h"

#define MIDI_MAX_ENDPOINTS 			5 //endpoint 0, bulk_IN(MIDI), bulk_OUT(MIDI), bulk_IN(VSP), bulk_OUT(VSP)
#define USB_SUBCLASS_MIDISTREAMING	3
#define DESC_BUFF_SIZE        		256
#define MIDI_EVENT_PACKET_SIZE 		64


namespace nw2s
{	
	// typedef struct
	// {
	//         uint32_t dwDTERate;
	//         uint8_t bCharFormat;
	//         uint8_t bParityType;
	//         uint8_t bDataBits;
	// }
	// LineCoding;

	class USBMidiDevice;
}

class nw2s::USBMidiDevice : public USBDeviceConfig, public UsbBasedDevice //public UsbConfigXtracter, 
{
	protected:

		static const uint32_t epDataInIndex;			// DataIn endpoint index
		static const uint32_t epDataOutIndex;			// DataOUT endpoint index
	    static const uint32_t epDataInIndexVSP;			// DataIn endpoint index(Vendor Specific Protocl)
	    static const uint32_t epDataOutIndexVSP;		// DataOUT endpoint index(Vendor Specific Protocl)

	    bool isMidiFound;

		/* Mandatory members */
		// USBHost		*pUsb;
		uint32_t	bAddress;							// Device USB address
		uint32_t	bConfNum;							// configuration number
		uint32_t	bNumEP;								// total number of EP in the configuration
		
	    bool     	bPollEnable;
		//WAS:bool		ready;
		//NOT in midi uint8_t 	bControlIface; 						// Control interface value

		/* Endpoint data structure describing the device EP */
		EpInfo		epInfo[MIDI_MAX_ENDPOINTS];

	    /* MIDI Event packet buffer */
	    uint8_t recvBuf[MIDI_EVENT_PACKET_SIZE];
	    uint8_t readPtr;

	    void parseConfigDescr(uint32_t addr, uint32_t conf);
	    unsigned int countSysExDataSize(uint32_t *dataptr);
	    uint32_t lookupMsgSize(uint32_t midiMsg);

	public:
		
		//TODO: Need these?
	    //uint32_t pid, vid;
		
		USBMidiDevice();

		/* Basic IO */
		//This is what it was before: uint32_t RecvData(uint32_t *nreadbytes, uint32_t datalen, uint8_t *dataptr);
	    uint32_t RecvData(uint32_t *bytes_rcvd, uint8_t *dataptr);
	    uint32_t RecvData(uint8_t *outBuf);
	    //uint32_t SendData(uint8_t *dataptr, uint8_t nCable = 0);
		uint32_t SendData(uint32_t datalen, uint8_t *dataptr);
	    uint32_t SendSysEx(uint8_t *dataptr, unsigned int datasize, uint8_t nCable=0);

		// virtual void task();

		/* Line control setup */
        // uint8_t setLineCoding(const LineCoding *dataptr);
        // uint8_t setControlLineState(uint8_t state);

		/* USBDeviceConfig implementation */
		virtual uint32_t Init(uint32_t parent, uint32_t port, uint32_t lowspeed);
		virtual uint32_t Release();
		virtual uint32_t Poll() { return 0; };	// not implemented
		virtual uint32_t GetAddress() { return bAddress; };
		virtual bool isReady() { return bPollEnable; };

		/* UsbConfigXtracter implementation */
		//virtual void EndpointXtract(uint32_t conf, uint32_t iface, uint32_t alt, uint32_t proto, const USB_ENDPOINT_DESCRIPTOR *ep);
};

// class nw2s::USBGridController : public USBGrid
// {
// 	protected:
//
// 		bool varibright = false;
//
// 		uint8_t beat = 0;
// 		uint8_t columnCount;
// 		uint8_t rowCount;
// 		uint8_t pages = 16;
//
// 		/* Just allocate the max value and work with it - [page][column][row] */
// 		uint8_t cells[16][16][16];
//
// 		bool gridInitialized = false;
// 		bool memoryInitialized = false;
// 		uint8_t lastpress[2] = {0, 0};
// 		uint8_t lastrelease[2] = {0, 0};
// 		uint8_t currentPage = 0;
//
// 		void setLED(uint8_t page, uint8_t column, uint8_t row, uint8_t value);
// 		void clearLED(uint8_t page, uint8_t column, uint8_t row);
// 		void switchPage(uint8_t page);
// 		void refreshGrid();
//
// 		virtual void buttonPressed(uint8_t column, uint8_t row) = 0;
// 		virtual void buttonReleased(uint8_t column, uint8_t row) = 0;
//
// 	public:
//
// 		USBGridController(GridDevice deviceType, uint8_t columns, uint8_t rows);
//
// 		virtual void task();
//
// 		uint8_t getColumnCount();
// 		uint8_t getRowCount();
// 		uint8_t getValue(uint8_t page, uint8_t column, uint8_t row);
// 		uint8_t getColumn(uint8_t column);
//
//
// };

#endif


