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

	Based on code originally from
	USB-MIDI class driver for USB Host Shield 2.0 Library
	which is opyright 2012-2014 Yuuichi Akagawa

	Which in turn was based on an idea from 
	LPK25 USB-MIDI to Serial MIDI converter
	by Collin Cunningham - makezine.com, narbotic.com
*/


#include "UsbMidi.h"
#include "JSONUtil.h"
#include "EventManager.h"

using namespace nw2s;

const uint32_t	USBMidiDevice::epDataInIndex  = 1;
const uint32_t	USBMidiDevice::epDataOutIndex = 2;
const uint32_t	USBMidiDevice::epDataInIndexVSP  = 3;
const uint32_t	USBMidiDevice::epDataOutIndexVSP = 4;

USBMidiDevice::USBMidiDevice()
{
	this->pUsb = &EventManager::usbHost;

    this->bAddress = 0;
    this->bNumEP = 1;
    this->bPollEnable  = false;
    this->isMidiFound = false;
    this->readPtr = 0;
	
	/* Setup an empty set of endpoints */
	for (uint32_t i = 0; i < MIDI_MAX_ENDPOINTS; ++i)
	{
		epInfo[i].deviceEpNum	= 0;
		epInfo[i].maxPktSize	= (i) ? 0 : 8;
		epInfo[i].epAttribs		= 0;
		epInfo[i].bmNakPower  	= (i) ? USB_NAK_NOWAIT : 4;
	}

	/* Register ourselves in USB subsystem */
	if (pUsb)
	{
		Serial.println("Registering with USB");
		pUsb->RegisterDeviceClass(this);
	}
}

uint32_t USBMidiDevice::Init(uint32_t parent, uint32_t port, uint32_t lowspeed)
{
	Serial.println("Initializing midi device");
	
	uint8_t		buf[DESC_BUFF_SIZE];
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
	Serial.println("getting device descriptor");
	rcode = pUsb->getDevDescr(0, 0, DESC_BUFF_SIZE, (uint8_t*)buf);

	Serial.println("got device descriptor");

    // vid = (uint16_t)buf[8]  + ((uint16_t)buf[9]  << 8);
    // pid = (uint16_t)buf[10] + ((uint16_t)buf[11] << 8);

	/* Restore p->epinfo */
	p->epinfo = oldep_ptr;

	if (rcode)
	{
		Serial.print("Failed to get device descriptor: ");
		Serial.println(rcode);
		return rcode;
	}

	Serial.println("Starting to allocate address.");

	/* Allocate new address according to device class */
	bAddress = addrPool.AllocAddress(parent, false, port);

	Serial.print("Allocating address ");
	Serial.print(bAddress);

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
	    parseConfigDescr(bAddress, i);

	    if (bNumEP > 1) break;
	}

	Serial.print("Num EP: ");
	Serial.println(bNumEP);

    if (bConfNum == 0)
	{   
		Serial.println("Device not found");    
		return 1;
    }

    // if( !isMidiFound ){ //MIDI Device not found. Try first Bulk transfer device
    //   epInfo[epDataInIndex].epAddr		= epInfo[epDataInIndexVSP].epAddr;
    //   epInfo[epDataInIndex].maxPktSize	= epInfo[epDataInIndexVSP].maxPktSize;
    //   epInfo[epDataOutIndex].epAddr		= epInfo[epDataOutIndexVSP].epAddr;
    //   epInfo[epDataOutIndex].maxPktSize	= epInfo[epDataOutIndexVSP].maxPktSize;
    // }

	/* Assign epInfo to epinfo pointer */
	rcode = pUsb->setEpInfoEntry(bAddress, bNumEP, epInfo);
	
	if (rcode)
	{
		Serial.print("Failed setEpInfoEntry: ");
		Serial.println(rcode);
	}

	/* Set Configuration Value */
	rcode = pUsb->setConf(bAddress, 0, bConfNum);

	if (rcode)
	{
		Serial.print("setConf failed: ");
		Serial.println(rcode);
		return rcode;
	}

	bPollEnable = true;
	
	Serial.println("USB Midi configured.");

	return 0;
}

// void USBMidiDevice::EndpointXtract(uint32_t conf, uint32_t iface, uint32_t alt, uint32_t proto, const USB_ENDPOINT_DESCRIPTOR *pep)
// {
// 	if (pep->bmAttributes != 2) return;
//
// 	if (bNumEP == MAX_ENDPOINTS)
// 	{
// 		return;
// 	}
//
// 	bConfNum = conf;
//
// 	uint32_t index = 0;
// 	uint32_t pipe = 0;
//
// 	if ((pep->bmAttributes & 0x02) == 2)
// 	{
// 		index = ((pep->bEndpointAddress & 0x80) == 0x80) ? epDataInIndex : epDataOutIndex;
// 	}
//
// 	/* Fill in the endpoint info structure */
// 	epInfo[index].deviceEpNum = pep->bEndpointAddress & 0x0F;
// 	epInfo[index].maxPktSize = pep->wMaxPacketSize;
//
// 	if (index == epDataInIndex)
// 	{
// 		pipe = UHD_Pipe_Alloc(bAddress, epInfo[index].deviceEpNum, UOTGHS_HSTPIPCFG_PTYPE_BLK, UOTGHS_HSTPIPCFG_PTOKEN_IN, epInfo[index].maxPktSize, 0, UOTGHS_HSTPIPCFG_PBK_1_BANK);
// 	}
// 	else if (index == epDataOutIndex)
// 	{
// 		pipe = UHD_Pipe_Alloc(bAddress, epInfo[index].deviceEpNum, UOTGHS_HSTPIPCFG_PTYPE_BLK, UOTGHS_HSTPIPCFG_PTOKEN_OUT, epInfo[index].maxPktSize, 0, UOTGHS_HSTPIPCFG_PBK_1_BANK);
// 	}
//
// 	/* Ensure pipe allocation is okay */
// 	if (pipe == 0)
// 	{
// 		Serial.println("Pipe allocation failure");
// 		return;
// 	}
//
// 	epInfo[index].hostPipeNum = pipe;
//
// 	bNumEP++;
// }


void USBMidiDevice::parseConfigDescr(uint32_t addr, uint32_t conf)
{
	uint8_t buf[DESC_BUFF_SIZE];
	uint8_t* buf_ptr = buf;
	uint32_t rcode;
	uint32_t descr_length;
	uint32_t descr_type;
	uint32_t total_length;
	USB_ENDPOINT_DESCRIPTOR *epDesc;
	boolean isMidi = false;

 	/* get configuration descriptor (get descriptor size only) */
	rcode = pUsb->getConfDescr(addr, 0, 4, conf, buf);

	if (rcode)
	{
		Serial.println("Errir getting configuration descriptor size");
		return;
	}  

	total_length = buf[2] | ((int)buf[3] << 8);

	/* check if total length is larger than buffer */
	if (total_length > DESC_BUFF_SIZE) 
	{
		total_length = DESC_BUFF_SIZE;
	}

	/* get configuration descriptor (all) */
	rcode = pUsb->getConfDescr(addr, 0, total_length, conf, buf ); 

	if (rcode)
	{
		Serial.println("Errir getting configuration descriptor");
    	return;
	}  

	/* parse descriptors */
	while (buf_ptr < buf + total_length) 
	{  
		descr_length = *buf_ptr;
		descr_type = *(buf_ptr + 1);
		
		switch (descr_type) 
		{
			case USB_DESCRIPTOR_CONFIGURATION :
			
				bConfNum = buf_ptr[5];
        		break;
				
      		case  USB_DESCRIPTOR_INTERFACE :
			
        		if( buf_ptr[5] == USB_CLASS_AUDIO && buf_ptr[6] == USB_SUBCLASS_MIDISTREAMING ) 
				{  
					/* p[5]; bInterfaceClass = 1(Audio), p[6]; bInterfaceSubClass = 3(MIDI Streaming) */
					isMidiFound = true;
					isMidi = true;
        		}
				else
				{
					Serial.print("No MIDI Device\n");
					isMidi = false;
				}
				
				break;
      
			case USB_DESCRIPTOR_ENDPOINT :

				epDesc = (USB_ENDPOINT_DESCRIPTOR *)buf_ptr;
				
				if ((epDesc->bmAttributes & 0x02) == 2) 
				{
          			uint32_t index;

					if (isMidi)
					{
		                index = ((epDesc->bEndpointAddress & 0x80) == 0x80) ? epDataInIndex : epDataOutIndex;
					}
          		  	else
					{
		                index = ((epDesc->bEndpointAddress & 0x80) == 0x80) ? epDataInIndexVSP : epDataOutIndexVSP;
					}

					epInfo[index].deviceEpNum = (epDesc->bEndpointAddress & 0x0F);
					epInfo[index].maxPktSize = (uint8_t)epDesc->wMaxPacketSize;
					
					bNumEP ++;
        		}
				
				break;
			
			default:
				
				break;
    
		}  
    
		buf_ptr += descr_length;
	}
}

uint32_t USBMidiDevice::Release()
{
    pUsb->GetAddressPool().FreeAddress(bAddress);
    
	bNumEP = 1;	
    bAddress = 0;
    bPollEnable = false;
    readPtr = 0;
	return 0;
}




