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
	
	Serial.print("Number endpoints: ");
	Serial.println(bNumEP);

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

	if (rcode)
	{
		Serial.print("Failed setEpInfoEntry: ");
		Serial.println(rcode);
	}

    Serial.print("Using configuration: ");
	Serial.println(bConfNum);

    /* Set Configuration Value */
    rcode = pUsb->setConf(bAddress, 0, bConfNum);
    
	if (rcode) 
	{
		Serial.println("Unable to set configuration");
		return 1;
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
					uint32_t pipe;

					if (isMidi)
					{
		                index = ((epDesc->bEndpointAddress & 0x80) == 0x80) ? epDataInIndex : epDataOutIndex;
					}
					else
					{
		                index = ((epDesc->bEndpointAddress & 0x80) == 0x80) ? epDataInIndexVSP : epDataOutIndexVSP;
					}

					/* Fill in the endpoint info structure */
					epInfo[index].deviceEpNum = (epDesc->bEndpointAddress & 0x0F);
					epInfo[index].maxPktSize = (uint8_t)epDesc->wMaxPacketSize;

					/* Allocate a pipe */
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
					
					Serial.println("Endpoint descriptor:");
					Serial.print("Length:\t\t");
					Serial.println(epDesc->bLength, HEX);
					Serial.print("Type:\t\t");
					Serial.println(epDesc->bDescriptorType, HEX);
					Serial.print("Address:\t");
					Serial.println(epDesc->bEndpointAddress, HEX);
					Serial.print("Attributes:\t");
					Serial.println(epDesc->bmAttributes, HEX);
					Serial.print("MaxPktSize:\t");
					Serial.println(epDesc->wMaxPacketSize, HEX);
					Serial.print("Poll Intrv:\t");
					Serial.println(epDesc->bInterval, HEX);
				}

				break;

			default:
        		
				break;
    	}  

		/* advance buffer pointer */
		buf_ptr += descr_length;    
  	}
}

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

uint32_t USBMidiDevice::lookupMsgSize(uint8_t midiMsg)
{
	uint8_t msgSize = 0;

	if (midiMsg < 0xf0)
	{
		midiMsg &= 0xf0;	
	} 

	switch(midiMsg) 
	{
		//3 bytes messages
		case 0xf2 : //system common message(SPP)
		case 0x80 : //Note off
		case 0x90 : //Note on
		case 0xa0 : //Poly KeyPress
		case 0xb0 : //Control Change
		case 0xe0 : //PitchBend Change
		  	msgSize = 3;
		  	break;

	    //2 bytes messages
	    case 0xf1 : //system common message(MTC)
	    case 0xf3 : //system common message(SongSelect)
	    case 0xc0 : //Program Change
	    case 0xd0 : //Channel Pressure
	    	msgSize = 2;
      		break;

	    //1 bytes messages
	    case 0xf8 : //system realtime message
	    case 0xf9 : //system realtime message
	    case 0xfa : //system realtime message
	    case 0xfb : //system realtime message
	    case 0xfc : //system realtime message
	    case 0xfe : //system realtime message
	    case 0xff : //system realtime message
	      	msgSize = 1;
	      	break;

	    //undefine messages
		default :
	      	break;
	  }
	  
	  return msgSize;
}

/* Receive data from MIDI device */
uint32_t USBMidiDevice::RecvData(uint32_t *bytes_rcvd, uint8_t *dataptr)
{
	bytes_rcvd[0] = (uint32_t)epInfo[epDataInIndex].maxPktSize;
	
	return pUsb->inTransfer(bAddress, epInfo[epDataInIndex].deviceEpNum, bytes_rcvd, dataptr);
}

/* Receive data from MIDI device */
uint32_t USBMidiDevice::RecvData(uint8_t *outBuf)
{
	uint32_t rcvd;
	uint32_t rcode;

	if (this->ready == false)
	{
		return 0;
	}
		
	/* Checking unprocessed message in buffer. */
	if (readPtr != 0 && readPtr < MIDI_EVENT_PACKET_SIZE)
	{
	    	if (recvBuf[readPtr] == 0 && recvBuf[readPtr + 1] == 0)
		{
			/* no unprocessed message left in the buffer. */
		}
		else
		{
		    readPtr++;
		    outBuf[0] = recvBuf[readPtr];
		    readPtr++;
		    outBuf[1] = recvBuf[readPtr];
		    readPtr++;
		    outBuf[2] = recvBuf[readPtr];
		    readPtr++;

		    return lookupMsgSize(outBuf[0]);
	    }
	}

	// uint32_t nbread = 0;
	//     uint8_t buf[64];
	//
	// /* See if there is any data to read */
	//     uint32_t rcode = read(&nbread, epInfo[epDataInIndex].maxPktSize, buf);
	//
	// if (rcode > 1)
	// {
	// 	Serial.print("Read error: ");
	// 	Serial.println(rcode, HEX);
	// 	return 0;
	// }


	readPtr = 0;
	rcode = RecvData(&rcvd, recvBuf);

	if (rcode != 0)
	{
		return 0;
	}
  
	/* if all data is zero, no valid data received. */
	if (recvBuf[0] == 0 && recvBuf[1] == 0 && recvBuf[2] == 0 && recvBuf[3] == 0)
	{
		Serial.println("Empty data packet received.");
	    	return 0;
	  	}

	readPtr++;
	outBuf[0] = recvBuf[readPtr];
	readPtr++;
	outBuf[1] = recvBuf[readPtr];
	readPtr++;
	outBuf[2] = recvBuf[readPtr];
	readPtr++;

	return lookupMsgSize(outBuf[0]);

	return 0;
}





