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

#include "Grid.h"

using namespace nw2s;

const uint32_t nw2s::USBGrid::epDataInIndex  = 1;
const uint32_t nw2s::USBGrid::epDataOutIndex = 2;

USBHost usbHost;


USBGrid::USBGrid() : pUsb(&usbHost), bAddress(0), bNumEP(1), ready(false)
{

	Serial.println("USBGrid ctor");

	/* Setup an empty set of endpoints */
	for (uint32_t i = 0; i < MAX_ENDPOINTS; ++i)
	{
		epInfo[i].deviceEpNum	= 0;
		epInfo[i].hostPipeNum	= 0;
		epInfo[i].maxPktSize	= (i) ? 0 : 8;
		epInfo[i].epAttribs		= 0;
		epInfo[i].bmNakPower  	= (i) ? USB_NAK_NOWAIT : USB_NAK_MAX_POWER;
	}

	/* Register ourselves in USB subsystem */
	if (pUsb)
	{
		pUsb->RegisterDeviceClass(this);
	}
}

void USBGrid::task()
{
	if (!isReady()) Serial.println("not ready");
	
	/* This must be run every loop() */
	pUsb->Task();	
}


uint32_t USBGrid::Init(uint32_t parent, uint32_t port, uint32_t lowspeed)
{
	uint8_t		buf[sizeof(USB_DEVICE_DESCRIPTOR)];
	uint32_t	rcode = 0;
	UsbDevice	*p = NULL;
	EpInfo		*oldep_ptr = NULL;
	uint32_t 	adkproto = -1;
	uint32_t	num_of_conf = 0;

	/* Get memory address of USB device address pool */
	AddressPool	&addrPool = pUsb->GetAddressPool();

    /* Check if address has already been assigned to an instance */
    if (bAddress)
	{
		Serial.println("USB_ERROR_CLASS_INSTANCE_ALREADY_IN_USE");
		return USB_ERROR_CLASS_INSTANCE_ALREADY_IN_USE;
	}

    /* Get pointer to pseudo device with address 0 assigned */
	p = addrPool.GetUsbDevicePtr(0);

	if (!p)
	{
		Serial.println("USB_ERROR_ADDRESS_NOT_FOUND_IN_POOL");
		return USB_ERROR_ADDRESS_NOT_FOUND_IN_POOL;
    }

	if (!p->epinfo)
	{
		Serial.println("USB_ERROR_EPINFO_IS_NULL");
		return USB_ERROR_EPINFO_IS_NULL;
	}

	/* Save old pointer to EP_RECORD of address 0 */
	oldep_ptr = p->epinfo;

	/* Temporary assign new pointer to epInfo to p->epinfo in order to avoid toggle inconsistence */
	p->epinfo = epInfo;

	p->lowspeed = lowspeed;

	/* Get device descriptor */
	rcode = pUsb->getDevDescr(0, 0, sizeof(USB_DEVICE_DESCRIPTOR), (uint8_t*)buf);

	/* Restore p->epinfo */
	p->epinfo = oldep_ptr;

	if (rcode)
	{
		Serial.print("Failed to get device descriptor : ");
		Serial.println(rcode);
		return rcode;
	}

	/* Allocate new address according to device class */
	bAddress = addrPool.AllocAddress(parent, false, port);

	/* Extract Max Packet Size from device descriptor */
	epInfo[0].maxPktSize = (uint8_t)((USB_DEVICE_DESCRIPTOR*)buf)->bMaxPacketSize0;

	/* Assign new address to the device */
	rcode = pUsb->setAddr(0, 0, bAddress);

	if (rcode)
	{
		p->lowspeed = false;
		addrPool.FreeAddress(bAddress);
		bAddress = 0;
		Serial.print("setAddr failed with rcode ");
		Serial.println(rcode);
		return rcode;
	}

	Serial.print("device address is now ");
	Serial.println(bAddress);

	p->lowspeed = false;

	/* get pointer to assigned address record */
	p = addrPool.GetUsbDevicePtr(bAddress);

	if (!p)
	{
		Serial.println("USB_ERROR_ADDRESS_NOT_FOUND_IN_POOL");
		return USB_ERROR_ADDRESS_NOT_FOUND_IN_POOL;
	}

	p->lowspeed = lowspeed;

	/* Assign epInfo to epinfo pointer - only EP0 is known */
	rcode = pUsb->setEpInfoEntry(bAddress, 1, epInfo);
	if (rcode)
	{
		Serial.print("Failed setEpInfoEntry: ");
		Serial.println(rcode);
		return rcode;
	}

	/* Go through configurations, find first bulk-IN, bulk-OUT EP, fill epInfo and quit */
	num_of_conf = ((USB_DEVICE_DESCRIPTOR*)buf)->bNumConfigurations;

	for (uint32_t i = 0; i < num_of_conf; ++i)
	{
		ConfigDescParser<0, 0, 0, 0> confDescrParser(this);

		delay(1);
		rcode = pUsb->getConfDescr(bAddress, 0, i, &confDescrParser);

		if (rcode)
		{
			Serial.print("Failed to get configurarion descriptors: ");
			Serial.println(rcode);
			return rcode;
		}

		if (bNumEP > 2)
		{
			break;
		}
	}

	if (bNumEP == 3)
	{
		/* Assign epInfo to epinfo pointer - this time all 3 endpoins */
		rcode = pUsb->setEpInfoEntry(bAddress, 3, epInfo);
		if (rcode)
		{
			Serial.print("Failed setEpInfoEntry: ");
			Serial.println(rcode);
		}
	}

	/* Set Configuration Value */
	rcode = pUsb->setConf(bAddress, 0, bConfNum);
	if (rcode)
	{
		Serial.print("setConf failed: ");
		Serial.println(rcode);
		return rcode;
	}

	/* Assume for now that the control interface is 0 */
	bControlIface = 0;

	/* Send the line control information */
    rcode = setControlLineState(3);

    if (rcode)
    {
		Serial.print("SetControlLineState failed: ");
		Serial.println(rcode);
        return rcode;
    }
	
    LineCoding	lc;
    lc.dwDTERate	= 115200;
    lc.bCharFormat	= 0;
    lc.bParityType	= 0;
    lc.bDataBits	= 8;

    rcode = setLineCoding(&lc);

    if (rcode)
	{
		Serial.print("SetLineCoding failed: ");
		Serial.println(rcode);
		return rcode;
	}

	ready = true;
	
	Serial.println("Grid configured.");

	return 0;
}



void USBGrid::EndpointXtract(uint32_t conf, uint32_t iface, uint32_t alt, uint32_t proto, const USB_ENDPOINT_DESCRIPTOR *pep)
{		
	if (pep->bmAttributes != 2) return;
	
	if (bNumEP == MAX_ENDPOINTS)
	{
		return;
	}

	bConfNum = conf;

	uint32_t index = 0;
	uint32_t pipe = 0;

	if ((pep->bmAttributes & 0x02) == 2)
	{
		index = ((pep->bEndpointAddress & 0x80) == 0x80) ? epDataInIndex : epDataOutIndex;
	}

	/* Fill in the endpoint info structure */
	epInfo[index].deviceEpNum = pep->bEndpointAddress & 0x0F;
	epInfo[index].maxPktSize = pep->wMaxPacketSize;

	if (index == epDataInIndex)
	{
		pipe = UHD_Pipe_Alloc(bAddress, epInfo[index].deviceEpNum, UOTGHS_HSTPIPCFG_PTYPE_BLK, UOTGHS_HSTPIPCFG_PTOKEN_IN, epInfo[index].maxPktSize, 0, UOTGHS_HSTPIPCFG_PBK_1_BANK);
	}
	else if (index == epDataOutIndex)
	{
		pipe = UHD_Pipe_Alloc(bAddress, epInfo[index].deviceEpNum, UOTGHS_HSTPIPCFG_PTYPE_BLK, UOTGHS_HSTPIPCFG_PTOKEN_OUT, epInfo[index].maxPktSize, 0, UOTGHS_HSTPIPCFG_PBK_1_BANK);
	}

	/* Ensure pipe allocation is okay */
	if (pipe == 0)
	{
		Serial.println("Pipe allocation failure");
		return;
	}

	epInfo[index].hostPipeNum = pipe;

	bNumEP++;
}


uint32_t USBGrid::Release()
{
	UHD_Pipe_Free(epInfo[epDataInIndex].hostPipeNum);
	UHD_Pipe_Free(epInfo[epDataOutIndex].hostPipeNum);

	/* Free allocated USB address */
	pUsb->GetAddressPool().FreeAddress(bAddress);

	/* Must have to be reset to 1 */
	bNumEP = 1;

	bAddress = 0;
	ready = false;

	return 0;
}

uint32_t USBGrid::read(uint32_t *nreadbytes, uint32_t datalen, uint8_t *dataptr)
{
	*nreadbytes = datalen;
	return pUsb->inTransfer(bAddress, epInfo[epDataInIndex].deviceEpNum, nreadbytes, dataptr);
}

uint32_t USBGrid::write(uint32_t datalen, uint8_t *dataptr)
{
	return pUsb->outTransfer(bAddress, epInfo[epDataOutIndex].deviceEpNum, datalen, dataptr);
}

uint8_t USBGrid::setControlLineState(uint8_t state) 
{
	return ( pUsb->ctrlReq(bAddress, 0, USB_SETUP_HOST_TO_DEVICE|USB_SETUP_TYPE_CLASS|USB_SETUP_RECIPIENT_INTERFACE, CDC_SET_CONTROL_LINE_STATE, state, 0, bControlIface, 0, 0, NULL, NULL));
}

uint8_t USBGrid::setLineCoding(const LineCoding *dataptr) 
{
	return ( pUsb->ctrlReq(bAddress, 0, USB_SETUP_HOST_TO_DEVICE|USB_SETUP_TYPE_CLASS|USB_SETUP_RECIPIENT_INTERFACE, CDC_SET_LINE_CODING, 0x00, 0x00, bControlIface, sizeof (LineCoding), sizeof (LineCoding), (uint8_t*)dataptr, NULL));
}

USBGridController::USBGridController(uint8_t columnCount)
{
	this->columnCount = columnCount;
	this->columns = new uint8_t[columnCount];	
}

void USBGridController::setGrid(uint8_t *columns)
{		
	memcpy(this->columns, columns, this->columnCount);

	/* Hardcoding the max size into the stack so I don't pollute the heap */
	uint8_t gridCommand[16];

	for (uint8_t i = 0; i < this->columnCount; i++)
	{
		gridCommand[i * 2] = 0x80 | i;
		gridCommand[(i * 2) + 1] = this->columns[i];
	}

	this->write(this->columnCount * 2, gridCommand);
}

void USBGridController::setColumn(uint8_t column, uint8_t value)
{		
	this->columns[column] = value;

	uint8_t columnCommand[] = { 0x80 | (column & 0x0F), value };

	this->write(2, columnCommand);
}

void USBGridController::setLED(uint8_t column, uint8_t row)
{
	this->columns[column] = this->columns[column] | (1 << row);

	uint8_t setCommand[] = { 0x21, (row << 4) | (column & 0x0F) };

	this->write(2, setCommand);
}

void USBGridController::clearLED(uint8_t column, uint8_t row)
{
	this->columns[column] = this->columns[column] | (1 << row);

	uint8_t setCommand[] = { 0x20, (row << 4) | (column & 0x0F) };

	this->write(2, setCommand);
}

uint8_t USBGridController::getColumnCount()
{
	return this->columnCount;
}

uint8_t getRowCount()
{
	return 8;
}

bool USBGridController::isSet(uint8_t column, uint8_t row)
{
	return this->columns[column] & (1 << row);
}

uint8_t USBGridController::getColumn(uint8_t column)
{
	return this->columns[column];
}

void USBGridController::task()
{
	USBGrid::task();

	uint32_t nbread = 0;
    uint8_t buf[64];

	if (isReady())
	{
		/* See if there is any data to read */
	    int rcode = read(&nbread, 64, buf);
	
		if (rcode > 1)
		{
			Serial.print("Read error: ");
			Serial.println(rcode, HEX);
		}

			// 	    if (nbread > 0)
			// 	    {
			// Serial.print("RCV: ");
			//
			// for (uint32_t i = 0; i < nbread; ++i)
			// {
			// Serial.print(buf[i], HEX);
			// }
			//
			// Serial.println();
			// 	    }
		
		for (uint8_t i = 0; i < nbread / 2; i++)
		{
			uint8_t command = buf[i * 2];
			uint8_t data = buf[(i * 2) + 1];
			
			if (command == 0x00)
			{
				Serial.println("release");
			}
			else if (command = 0x01)
			{
				Serial.println("press");
			}
			else
			{
				Serial.print("Unknown command: ");
				Serial.println(command, HEX);
			}
		}
	}	
}









