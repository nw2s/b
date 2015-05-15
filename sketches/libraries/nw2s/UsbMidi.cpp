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

#include "UsbMidi.h"
#include "JSONUtil.h"
#include "EventManager.h"

using namespace nw2s;

const uint32_t USBMidiDevice::epDataInIndex  = 1;
const uint32_t USBMidiDevice::epDataOutIndex = 2;
const uint32_t USBMidiDevice::epDataInIndexVSP  = 3;
const uint32_t USBMidiDevice::epDataOutIndexVSP = 4;


USBMidiDevice::USBMidiDevice() : bAddress(0), bNumEP(1), ready(false)
{
	this->pUsb = &EventManager::usbHost;
	// this->deviceType = deviceType;
	
	/* Setup an empty set of endpoints */
	for (uint32_t i = 0; i < MIDI_MAX_ENDPOINTS; ++i)
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

uint32_t USBMidiDevice::Init(uint32_t parent, uint32_t port, uint32_t lowspeed)
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

	/* Assign epInfo to epinfo pointer - this time all 3 endpoins */
	rcode = pUsb->setEpInfoEntry(bAddress, 1, epInfo);
	if (rcode)
	{
		Serial.print("Failed setEpInfoEntry: ");
		Serial.println(rcode);
	}

	for (uint32_t i = 0; i < num_of_conf; i++)
	{
	    parseConfigDescr(bAddress, i);
	    if (bNumEP > 1)
		{
			break;
		}
	}

    if (bConfNum == 0)
	{
		Serial.println("Device not found");
		return 1;
    }

    if (!isMidiFound)
	{ 
		Serial.println("No midi device found");
		return 1;
    }

    /* Assign epInfo to epinfo pointer */
    rcode = pUsb->setEpInfoEntry(bAddress, bNumEP, epInfo);

    Serial.print("Using configuration: ");
	Serial.println(bConfNum);

    /* Set Configuration Value */
    rcode = pUsb->setConf(bAddress, 0, bConfNum);
    
	if (rcode) 
	{
		Serial.println("Unable to set configuration");
    }

	Serial.println("USB Midi Device configured.");

    this->ready = true;
    return 0;
}


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

	/* get configuration descriptor size */
	rcode = pUsb->getConfDescr(addr, 0, 4, conf, buf);
	if (rcode)
	{
		Serial.print("Error getting config descriptor size for addr ");
		Serial.print(addr);
		Serial.print(", ");
		Serial.println(conf);
    	return;
  	}  

	total_length = buf[2] | ((int)buf[3] << 8);

	/* check if total length is larger than buffer */
	if (total_length > DESC_BUFF_SIZE) 
	{    
		total_length = DESC_BUFF_SIZE;
	}

	/* get whole configuration descriptor */
	rcode = pUsb->getConfDescr(addr, 0, total_length, conf, buf); 
  	if (rcode)
	{
		Serial.print("Error getting config descriptor for addr ");
		Serial.print(addr);
		Serial.print(", ");
		Serial.println(conf);
    	return;
	}  

	/* parsing descriptors */
	while (buf_ptr < buf + total_length)
	{  
		descr_length = *(buf_ptr);
		descr_type = *(buf_ptr + 1);
		
		switch( descr_type ) 
		{
			case USB_DESCRIPTOR_CONFIGURATION :
			
				bConfNum = buf_ptr[5];
				break;

			case  USB_DESCRIPTOR_INTERFACE :

				if (buf_ptr[5] == USB_CLASS_AUDIO && buf_ptr[6] == USB_SUBCLASS_MIDISTREAMING ) 
				{  
					/* p[5]; bInterfaceClass = 1(Audio), p[6]; bInterfaceSubClass = 3(MIDI Streaming) */
					Serial.println("Midi device found.");
					isMidiFound = true;
					isMidi = true;
				}
				else
				{
					isMidi = false;
				}
				
				break;

			case USB_DESCRIPTOR_ENDPOINT :

				epDesc = (USB_ENDPOINT_DESCRIPTOR *)buf_ptr;

				if ((epDesc->bmAttributes & 0x02) == 2) 
				{
					/* bulk */
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
					bNumEP++;
				}

				break;

			default:
        		
				break;
    	}  

		/* advance buffer pointer */
		buf_ptr += descr_length;    
  	}
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


uint32_t USBMidiDevice::Release()
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

uint32_t USBMidiDevice::read(uint32_t *nreadbytes, uint32_t datalen, uint8_t *dataptr)
{
	*nreadbytes = datalen;
	return pUsb->inTransfer(bAddress, epInfo[epDataInIndex].deviceEpNum, nreadbytes, dataptr);
}

uint32_t USBMidiDevice::write(uint32_t datalen, uint8_t *dataptr)
{
	if (datalen > 255) Serial.println("WARNING: Trying to send more than 255 bytes down the USB pipe!");

	return pUsb->outTransfer(bAddress, epInfo[epDataOutIndex].deviceEpNum, datalen, dataptr);
}

// uint8_t USBMidiDevice::setControlLineState(uint8_t state)
// {
// 	return ( pUsb->ctrlReq(bAddress, 0, USB_SETUP_HOST_TO_DEVICE|USB_SETUP_TYPE_CLASS|USB_SETUP_RECIPIENT_INTERFACE, CDC_SET_CONTROL_LINE_STATE, state, 0, bControlIface, 0, 0, NULL, NULL));
// }
//
// uint8_t USBMidiDevice::setLineCoding(const LineCoding *dataptr)
// {
// 	return ( pUsb->ctrlReq(bAddress, 0, USB_SETUP_HOST_TO_DEVICE|USB_SETUP_TYPE_CLASS|USB_SETUP_RECIPIENT_INTERFACE, CDC_SET_LINE_CODING, 0x00, 0x00, bControlIface, sizeof (LineCoding), sizeof (LineCoding), (uint8_t*)dataptr, NULL));
// }






