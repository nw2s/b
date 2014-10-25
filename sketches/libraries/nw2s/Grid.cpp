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


USBGrid::USBGrid(GridDevice deviceType) : pUsb(&usbHost), bAddress(0), bNumEP(1), ready(false)
{
	this->deviceType = deviceType;
	
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

		/* monome FTDI only looks for 2. CDC ACM (leonardo) looks for 3 */ 
		if (deviceType == DEVICE_40H_TRELLIS && bNumEP > 2)
		{
			break;
		}
		else if (deviceType == DEVICE_SERIES && bNumEP > 1)
		{
			break;
		}
	}

	// if (bNumEP == 3)
	// {
		/* Assign epInfo to epinfo pointer - this time all 3 endpoins */
		rcode = pUsb->setEpInfoEntry(bAddress, 3, epInfo);
		if (rcode)
		{
			Serial.print("Failed setEpInfoEntry: ");
			Serial.println(rcode);
		}
	// }

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

	if (deviceType == DEVICE_40H_TRELLIS)
	{
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
	}
	else if ((deviceType == DEVICE_SERIES) || (deviceType == DEVICE_GRIDS))
	{
		/* Set the baud rate */
        uint16_t baud_value = 0;
		uint16_t baud_index = 0;
		
        uint32_t divisor3 = 48000000 / 2 / 115200; // divisor shifted 3 bits to the left

        static const unsigned char divfrac [8] = {0, 3, 2, 0, 1, 1, 2, 3};
        static const unsigned char divindex[8] = {0, 0, 0, 1, 0, 1, 1, 1};

        baud_value = divisor3 >> 3;
        baud_value |= divfrac [divisor3 & 0x7] << 14;
        baud_index = divindex[divisor3 & 0x7];

        /* Deal with special cases for highest baud rates. */
        if (baud_value == 1) 
		{
			baud_value = 0;
		}
        else if (baud_value == 0x4001) 
		{
            baud_value = 1;
		}

        rcode = pUsb->ctrlReq(bAddress, 0, bmREQ_FTDI_OUT, FTDI_SIO_SET_BAUD_RATE, baud_value & 0xff, baud_value >> 8, baud_index, 0, 0, NULL, NULL);
		
	    if (rcode)
	    {
	        Serial.println("Error setting baudrate");
	        return rcode;
	    }

		/* Set no flow control */
		rcode = pUsb->ctrlReq(bAddress, 0, bmREQ_FTDI_OUT, FTDI_SIO_SET_FLOW_CTRL, 0x11, 0x13, FTDI_SIO_DISABLE_FLOW_CTRL << 8, 0, 0, NULL, NULL);		
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
	if (datalen > 255) Serial.println("WARNING: Trying to send more than 255 bytes down the USB pipe!");

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

USBGridController::USBGridController(GridDevice deviceType, uint8_t columnCount, uint8_t rowCount) : USBGrid(deviceType)
{
	this->columnCount = columnCount;
	this->rowCount = rowCount;
}

void USBGridController::setLED(uint8_t page, uint8_t column, uint8_t row, uint8_t value)
{
	if (!value)
	{
		this->clearLED(page, column, row);
		return;
	}

	this->cells[page][column][row] = value;

	if (page == this->currentPage)
	{
		switch (this->deviceType)
		{
			case DEVICE_40H_TRELLIS:
			{
				uint8_t setCommand[] = { 0x21, (column << 4) | (row & 0x0F) };
				this->write(2, setCommand);
				
				break;
			}				
			case DEVICE_SERIES:
			case DEVICE_GRIDS:
			{
				this->refreshGrid();
				break;		
			}			
		}
	}
}

void USBGridController::clearLED(uint8_t page, uint8_t column, uint8_t row)
{
	this->cells[page][column][row] = 0;

	if (page == this->currentPage)
	{
		switch (this->deviceType)
		{
			case DEVICE_40H_TRELLIS:
			{
				uint8_t setCommand[] = { 0x20, (column << 4) | (row & 0x0F) };
				this->write(2, setCommand);
				
				break;
			}
						
			case DEVICE_SERIES:
			case DEVICE_GRIDS:
			{
				this->refreshGrid();
				break;			
			}						
		}
	}
}

uint8_t USBGridController::getColumnCount()
{
	return this->columnCount;
}

uint8_t USBGridController::getRowCount()
{
	return this->rowCount;
}

uint8_t USBGridController::getValue(uint8_t page, uint8_t column, uint8_t row)
{
	return this->cells[page][column][row];
}

void USBGridController::switchPage(uint8_t page)
{
	this->currentPage = page;

	this->refreshGrid();
}

void USBGridController::refreshGrid()
{	
	switch (this->deviceType)
	{
		case DEVICE_40H_TRELLIS:
		{
			uint8_t gridCommand[32];
		
			for (uint8_t column = 0; column < this->columnCount; column++)
			{
				gridCommand[column * 2] = 0x80 | column;
				gridCommand[(column * 2) + 1] = 0;
				
				for (uint8_t row = 1; row < this->rowCount; row++)
				{
					if (this->cells[this->currentPage][column][row])
					{
						gridCommand[(column * 2) + 1] = gridCommand[(column * 2) + 1] | (1 << (this->rowCount - 1) - row);
					}
				}
			}
		
			this->write(this->columnCount * 2, gridCommand);
			
			break;
		}
		
		case DEVICE_SERIES:
		{
			/* Quadrant-based commands are most efficient for full screen refresh */
			
			/* 64 - one quadrant */
			uint8_t quadrant1[] = { (0x08 << 4) | 0, 0, 0, 0, 0, 0, 0, 0, 0};
	
			/* 128 - two quadrants */
			uint8_t quadrant2[] = { (0x08 << 4) | 1, 0, 0, 0, 0, 0, 0, 0, 0};
			
			/* 256 - four quadrants */
			uint8_t quadrant3[] = { (0x08 << 4) | 2, 0, 0, 0, 0, 0, 0, 0, 0};
			uint8_t quadrant4[] = { (0x08 << 4) | 3, 0, 0, 0, 0, 0, 0, 0, 0};
			
			//TODO: Only working with 64 right now to get this running. 
			/* The LED_FRAME command for series is per row, not column */
			for (uint8_t row = 0; row < this->rowCount; row++)
			{
				for (uint8_t column = 0; column < this->columnCount; column++)
				{
					/* quadrant 1 */
					if (column < 8 && row < 8)
					{
						if (this->cells[this->currentPage][column][row])
						{
							quadrant1[(row % 8) + 1] = quadrant1[(row % 8) + 1] | (1 << (column % 8));
						}
					}

					/* quadrant 2 */
					if (column <= 8 && row < 8)
					{
						if (this->cells[this->currentPage][column][row])
						{
							quadrant2[(row % 8) + 1] = quadrant2[(row % 8) + 1] | (1 << (column % 8));
						}
					}

					/* quadrant 3 */
					if (column <= 8 && row < 8)
					{
						if (this->cells[this->currentPage][column][row])
						{
							quadrant3[(row % 8) + 1] = quadrant3[(row % 8) + 1] | (1 << (column % 8));
						}
					}

					/* quadrant 4 */
					if (column <= 8 && row < 8)
					{
						if (this->cells[this->currentPage][column][row])
						{
							quadrant4[(row % 8) + 1] = quadrant4[(row % 8) + 1] | (1 << (column % 8));
						}
					}
				}				
			}

			this->write(9, quadrant1);
			this->write(9, quadrant2);
			this->write(9, quadrant3);
			this->write(9, quadrant4);
			delay(5);
	
			break;
		}
			
		case DEVICE_GRIDS:
		{
			if (!varibright)
			{
				/* If it's not varibright, then it's just boolean off and on */

				/* Quadrant-based commands are most efficient for full screen refresh */
			
				/* 64 - one quadrant */
				uint8_t quadrant1[] = { 0x14, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
	
				/* 128 - two quadrants */
				uint8_t quadrant2[] = { 0x14, 8, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
			
				/* 256 - four quadrants */
				uint8_t quadrant3[] = { 0x14, 0, 8, 0, 0, 0, 0, 0, 0, 0, 0, 0};
				uint8_t quadrant4[] = { 0x14, 8, 8, 0, 0, 0, 0, 0, 0, 0, 0, 0};
			
				/* The LED_FRAME command for grids is per row, not column */
				for (uint8_t row = 0; row < this->rowCount; row++)
				{
					for (uint8_t column = 0; column < this->columnCount; column++)
					{
						/* quadrant 1 */
						if (column < 8 && row < 8)
						{
							if (this->cells[this->currentPage][column][row])
							{
								quadrant1[(row % 8) + 3] = quadrant1[(row % 8) + 3] | (1 << (column % 8));
							}
						}

						/* quadrant 2 */
						if (column >= 8 && row < 8)
						{
							if (this->cells[this->currentPage][column][row])
							{
								quadrant2[(row % 8) + 3] = quadrant2[(row % 8) + 3] | (1 << (column % 8));
							}
						}

						/* quadrant 3 */
						if (column < 8 && row >= 8)
						{
							if (this->cells[this->currentPage][column][row])
							{
								quadrant3[(row % 8) + 3] = quadrant3[(row % 8) + 3] | (1 << (column % 8));
							}
						}

						/* quadrant 4 */
						if (column >= 8 && row >= 8)
						{
							if (this->cells[this->currentPage][column][row])
							{
								quadrant4[(row % 8) + 3] = quadrant4[(row % 8) + 3] | (1 << (column % 8));
							}
						}
					}				
				}
				
				//TODO: We don't need to send them all if the size isn't that big
				this->write(11, quadrant1);
				this->write(11, quadrant2);
				this->write(11, quadrant3);
				this->write(11, quadrant4);

				delay(5);
			}
			else
			{
				/* If it is varibright, then it's 4 bits of brightness */

				/* Quadrant-based commands are most efficient for full screen refresh */
			
				/* 64 - one quadrant */
				uint8_t quadrant1[] = { 0x1A, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
	
				/* 128 - two quadrants */
				uint8_t quadrant2[] = { 0x1A, 8, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
			
				/* 256 - four quadrants */
				uint8_t quadrant3[] = { 0x1A, 0, 8, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
				uint8_t quadrant4[] = { 0x1A, 8, 8, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
			
				/* The LED_FRAME command for grids is per row, not column */
				for (uint8_t row = 0; row < this->rowCount; row++)
				{
					for (uint8_t column = 0; column < this->columnCount; column++)
					{
						/* quadrant 1 */
						if (column < 8 && row < 8)
						{
							if (this->cells[this->currentPage][column][row])
							{
								quadrant1[((row % 8) * 4) + ((column % 8) / 2) + 3] = quadrant1[((row % 8) * 4) + ((column % 8) / 2) + 3] | ((this->cells[this->currentPage][column][row] & 0x0F) << (((column + 1) % 2) * 4));
							}
						}

						/* quadrant 2 */
						if (column >= 8 && row < 8)
						{
							if (this->cells[this->currentPage][column][row])
							{
								quadrant2[((row % 8) * 4) + ((column % 8) / 2) + 3] = quadrant2[((row % 8) * 4) + ((column % 8) / 2) + 3] | ((this->cells[this->currentPage][column][row] & 0x0F) << (((column + 1) % 2) * 4));
							}
						}

						/* quadrant 3 */
						if (column < 8 && row >= 8)
						{
							if (this->cells[this->currentPage][column][row])
							{
								quadrant3[((row % 8) * 4) + ((column % 8) / 2) + 3] = quadrant3[((row % 8) * 4) + ((column % 8) / 2) + 3] | ((this->cells[this->currentPage][column][row] & 0x0F) << (((column + 1) % 2) * 4));
							}
						}

						/* quadrant 4 */
						if (column >= 8 && row >= 8)
						{
							if (this->cells[this->currentPage][column][row])
							{
								quadrant4[((row % 8) * 4) + ((column % 8) / 2) + 3] = quadrant4[((row % 8) * 4) + ((column % 8) / 2) + 3] | ((this->cells[this->currentPage][column][row] & 0x0F) << (((column + 1) % 2) * 4));
							}
						}
					}				
				}

				//TODO: Not sure why it screws up without the delay

				this->write(35, quadrant1);
				if (this->columnCount > 8) 
				{
					this->write(35, quadrant2);

					if (this->rowCount > 8) 
					{
						delay(5);
						this->write(35, quadrant3);
						this->write(35, quadrant4);						
					}	
				}	

				delay(5);				
			}
	
			break;
		}
	}
}

void USBGridController::task()
{
	USBGrid::task();

	uint32_t nbread = 0;
    uint8_t buf[64];

	if (isReady())
	{		
		if (!gridInitialized)
		{	
			this->currentPage = 0;
			this->refreshGrid();
			
			gridInitialized = true;
		}
		
		/* See if there is any data to read */
	    int rcode = read(&nbread, 64, buf);
	
		if (rcode > 1)
		{
			Serial.print("Read error: ");
			Serial.println(rcode, HEX);
		}
			
		/* Only problem, if we don't get the whole chunk at once, we'll drop half */
		for (uint8_t i = 0; i < nbread; i++)
		{
			if (deviceType == DEVICE_40H_TRELLIS)
			{
				uint8_t command = buf[i++];			
				uint8_t data = buf[i];
			
				uint8_t column = data >> 4;
				uint8_t row = data & 0x0F;

				if (command == 0x01)
				{
					//TODO: refactor to a struct
					this->lastpress[0] = column;
					this->lastpress[1] = row;
				
					this->buttonPressed(column, row);
				}
				else if (command == 0x00)
				{
					this->lastrelease[0] = column;
					this->lastrelease[1] = row;
				
					this->buttonReleased(column, row);
				}
				else
				{
					Serial.print("Unknown command: ");
					Serial.println(command, HEX);
				}
			}
			else if (deviceType == DEVICE_SERIES)
			{
				uint8_t command = buf[i++];			
				uint8_t data = buf[i];
			
				uint8_t column = data >> 4;
				uint8_t row = data & 0x0F;

				if (command == 0x00)
				{
					//TODO: refactor to a struct
					this->lastpress[0] = column;
					this->lastpress[1] = row;
				
					this->buttonPressed(column, row);
				}
				else if (command == 0x10)
				{
					this->lastrelease[0] = column;
					this->lastrelease[1] = row;
				
					this->buttonReleased(column, row);
				}
				else if (command == 0x31)
				{
					/* Not sure why the monome is spitting this out constantly */
				}
				else
				{
					Serial.print("Unknown command: ");
					Serial.println(command, HEX);
				}
			}
			else if (deviceType == DEVICE_GRIDS)
			{
				uint8_t command = buf[i++];			
				
				if (command == 0x21)
				{
					uint8_t column = buf[i++];
					uint8_t row = buf[i];

					this->lastpress[0] = column;
					this->lastpress[1] = row;
				
					this->buttonPressed(column, row);
					
					Serial.print(column, HEX);
					Serial.print(" ");
					Serial.println(row, HEX);
				}
				else if (command == 0x20)
				{
					uint8_t column = buf[i++];
					uint8_t row = buf[i];

					this->lastrelease[0] = column;
					this->lastrelease[1] = row;
				
					this->buttonReleased(column, row);
				}
				else if (command == 0x31)
				{
					/* Not sure why the monome is spitting this out constantly */
				}
				else
				{
					Serial.print("Unknown command: ");
					Serial.println(command, HEX);
				}
			}
		}
	}	
}





