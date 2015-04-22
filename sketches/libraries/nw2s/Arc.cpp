/*
	support for monome arc
	copyright (c) 2014 scannerdarkly (scannerdarkly.git@gmail.com)

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

#include "Arc.h"
#include "JSONUtil.h"
#include "EventManager.h"

using namespace nw2s;

const uint32_t nw2s::USBArc::epDataInIndex  = 1;
const uint32_t nw2s::USBArc::epDataOutIndex = 2;


USBArc::USBArc() : bAddress(0), bNumEP(1), ready(false)
{
	this->pUsb = &EventManager::usbHost;
	
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

uint32_t USBArc::Init(uint32_t parent, uint32_t port, uint32_t lowspeed)
{
	// copied verbatim from USBGrid but with the parts for 40h and series grids removed
	// this and other USBArc stuff could be probably extracted into USBMonome class
	// that both USBGrid and USBArc would inherit from?

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

	ready = true;
	
	Serial.println("Arc configured.");

	return 0;
}


void USBArc::EndpointXtract(uint32_t conf, uint32_t iface, uint32_t alt, uint32_t proto, const USB_ENDPOINT_DESCRIPTOR *pep)
{	
	// copied verbatim from USBGrid
	
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


uint32_t USBArc::Release()
{
	// copied verbatim from USBGrid
	
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

uint32_t USBArc::read(uint32_t *nreadbytes, uint32_t datalen, uint8_t *dataptr)
{
	// copied verbatim from USBGrid
	
	*nreadbytes = datalen;
	return pUsb->inTransfer(bAddress, epInfo[epDataInIndex].deviceEpNum, nreadbytes, dataptr);
}

uint32_t USBArc::write(uint32_t datalen, uint8_t *dataptr)
{
	// copied verbatim from USBGrid
	
	if (datalen > 255) Serial.println("WARNING: Trying to send more than 255 bytes down the USB pipe!");

	return pUsb->outTransfer(bAddress, epInfo[epDataOutIndex].deviceEpNum, datalen, dataptr);
}

uint8_t USBArc::setControlLineState(uint8_t state) 
{
	// copied verbatim from USBGrid
	
	return ( pUsb->ctrlReq(bAddress, 0, USB_SETUP_HOST_TO_DEVICE|USB_SETUP_TYPE_CLASS|USB_SETUP_RECIPIENT_INTERFACE, CDC_SET_CONTROL_LINE_STATE, state, 0, bControlIface, 0, 0, NULL, NULL));
}

uint8_t USBArc::setLineCoding(const LineCoding *dataptr) 
{
	// copied verbatim from USBGrid
	
	return ( pUsb->ctrlReq(bAddress, 0, USB_SETUP_HOST_TO_DEVICE|USB_SETUP_TYPE_CLASS|USB_SETUP_RECIPIENT_INTERFACE, CDC_SET_LINE_CODING, 0x00, 0x00, bControlIface, sizeof (LineCoding), sizeof (LineCoding), (uint8_t*)dataptr, NULL));
}

USBArcController::USBArcController(uint8_t encoderCount, bool pushButton) : USBArc()
{
	this->encoderCount = encoderCount;
	this->pushButton = pushButton;
	
	for(int page = 0; page < ARC_MAX_PAGES; page++)
		for(int ring = 0; ring < ARC_MAX_ENCODERS; ring++)
			for(int led = 0; led < ARC_MAX_LEDS; led++)
				values[page][ring][led] = 0;
	
	for(int ring = 0; ring < ARC_MAX_ENCODERS; ring++)
		lastDelta[ring] = 0;
}

uint8_t USBArcController::getEncoderCount()
{
	return this->encoderCount;
}

bool USBArcController::pushButtonSupported()
{
	return this->pushButton;
}

uint8_t USBArcController::getValue(uint8_t page, uint8_t ring, uint8_t led)
{
	return this->values[page][ring][led];
}

void USBArcController::switchPage(uint8_t page)
{
	this->currentPage = page;
	this->refreshArc();
}

void USBArcController::refreshArc()
{	
	uint8_t ring0[34], ring1[34], ring2[34], ring3[34];
	ring0[0] = ring1[0] = ring2[0] = ring3[0] = 0x92;
	ring0[1] = 0; ring1[1] = 1; ring2[1] = 2; ring3[1] = 3;
	for (int i = 0; i < 32; i++)
	{
		ring0[i+2] = (this->values[this->currentPage][0][i*2+1] & 0xF) | (this->values[this->currentPage][0][i*2] << 4);
		ring1[i+2] = (this->values[this->currentPage][1][i*2+1] & 0xF) | (this->values[this->currentPage][1][i*2] << 4);
		ring2[i+2] = (this->values[this->currentPage][2][i*2+1] & 0xF) | (this->values[this->currentPage][2][i*2] << 4);
		ring3[i+2] = (this->values[this->currentPage][3][i*2+1] & 0xF) | (this->values[this->currentPage][3][i*2] << 4);
	}
	this->write(34, ring0);
	delay(2);
	this->write(34, ring1);
	delay(2);
	this->write(34, ring2);
	delay(2);
	this->write(34, ring3);
	delay(2);
}

void USBArcController::setLED(uint8_t page, uint8_t ring, uint8_t led, uint8_t value)
{
	this->values[page][ring][led] = value;
	if (page == this->currentPage)
	{
		uint8_t serial[4] = {0x90, ring, led, value};
		this->write(4, serial);
	}
}

void USBArcController::clearLED(uint8_t page, uint8_t ring, uint8_t led)
{
	this->setLED(page, ring, led, 0);
}

void USBArcController::setRing(uint8_t page, uint8_t ring, uint8_t value)
{
	for (uint8_t led = 0; led < ARC_MAX_LEDS; led++)
		this->values[page][ring][led] = value;
	if (page == this->currentPage)
	{
		uint8_t serial[3] = {0x91, ring, value};
		this->write(3, serial);
	}
}

void USBArcController::clearRing(uint8_t page, uint8_t ring)
{
	this->setRing(page, ring, 0);
}

void USBArcController::setRange(uint8_t page, uint8_t ring, uint8_t startLed, uint8_t endLed, uint8_t value)
{
	uint8_t wrappedEndLed = endLed;
	if (endLed < startLed) wrappedEndLed += ARC_MAX_LEDS;
	for (uint8_t led = startLed; led <= wrappedEndLed; led++)
		this->values[page][ring][led % ARC_MAX_LEDS] = value;
		
	if (page == this->currentPage)
	{
		uint8_t serial[5] = {0x93, ring, startLed, endLed, value};
		this->write(5, serial);
	}
}

void USBArcController::clearRange(uint8_t page, uint8_t ring, uint8_t startLed, uint8_t endLed)
{
	this->setRange(page, ring, startLed, endLed, 0);
}

void USBArcController::task()
{
	USBArc::task();

	uint32_t nbread = 0;
    uint8_t buf[64];

	if (isReady())
	{		
		if (!arcInitialized)
		{	
			this->currentPage = 0;
			this->refreshArc();
			
			arcInitialized = true;
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
			uint8_t command = buf[i++];			
			
			if (command == 0x50)
			{
				uint8_t encoder = buf[i++];
				int8_t delta = buf[i];

				lastDelta[encoder] = delta;
				this->encoderPositionChanged(encoder, delta);
				
				/*
				Serial.print(encoder);
				Serial.print(" ");
				Serial.println(delta);
				*/
			}
			else if (command == 0x51)
			{
				uint8_t encoder = buf[i++];
				this->buttonReleased(lastReleased = encoder);
			}
			else if (command == 0x52)
			{
				uint8_t encoder = buf[i++];
				this->buttonPressed(lastPressed = encoder);
			}
			else if (command == 0x31)
			{
			}
			else
			{
				//Serial.print("Unknown command: ");
				//Serial.println(command, HEX);
			}
		}
	}	
}