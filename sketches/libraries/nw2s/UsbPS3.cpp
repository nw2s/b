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

#include "UsbPS3.h"
#include "JSONUtil.h"
#include "EventManager.h"

using namespace nw2s;

// const uint32_t nw2s::UsbPS3::epDataInIndex  = 1;
// const uint32_t nw2s::UsbPS3::epDataOutIndex = 2;


UsbPS3::UsbPS3() : bAddress(0), bNumEP(1), ready(false)
{
	this->pUsb = &EventManager::usbHost;
	
	/* Setup an empty set of endpoints */
    for (uint8_t i = 0; i < PS3_MAX_ENDPOINTS; i++) 
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

uint32_t UsbPS3::Init(uint32_t parent, uint32_t port, uint32_t lowspeed)
{
	uint8_t		buf[sizeof(USB_DEVICE_DESCRIPTOR)];
    USB_DEVICE_DESCRIPTOR* udd = reinterpret_cast<USB_DEVICE_DESCRIPTOR*>(buf);
	uint32_t	rcode = 0;
	UsbDevice	*p = NULL;
	EpInfo		*oldep_ptr = NULL;
    uint16_t 	PID;
    uint16_t 	VID;
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

	/* Check that it's a known device */
    VID = udd->idVendor;
    PID = udd->idProduct;

    if(VID != PS3_VID || (PID != PS3_PID && PID != PS3NAVIGATION_PID && PID != PS3MOVE_PID))
	{
		Serial.print("Unknown vendor/prodct : ");
		Serial.print(VID);
		Serial.print(" ");
		Serial.println(PID);
		return 1;
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


	/* The application will work in reduced host mode, so we can save program and data
	memory space. After verifying the PID and VID we will use known values for the
	configuration values for device, interface, endpoints and HID for the PS3 Controllers */

	/* Initialize data structures for endpoints of device */
	epInfo[ PS3_OUTPUT_PIPE ].deviceEpNum = 0x02; // PS3 output endpoint
	epInfo[ PS3_OUTPUT_PIPE ].epAttribs = USB_TRANSFER_TYPE_INTERRUPT;
	epInfo[ PS3_OUTPUT_PIPE ].bmNakPower = USB_NAK_NOWAIT; // Only poll once for interrupt endpoints
	epInfo[ PS3_OUTPUT_PIPE ].maxPktSize = EP_MAXPKTSIZE;
	epInfo[ PS3_OUTPUT_PIPE ].bmSndToggle = 0;
	epInfo[ PS3_OUTPUT_PIPE ].bmRcvToggle = 0;
	epInfo[ PS3_INPUT_PIPE ].deviceEpNum = 0x01; // PS3 report endpoint
	epInfo[ PS3_INPUT_PIPE ].epAttribs = USB_TRANSFER_TYPE_INTERRUPT;
	epInfo[ PS3_INPUT_PIPE ].bmNakPower = USB_NAK_NOWAIT; // Only poll once for interrupt endpoints
	epInfo[ PS3_INPUT_PIPE ].maxPktSize = EP_MAXPKTSIZE;
	epInfo[ PS3_INPUT_PIPE ].bmSndToggle = 0;
	epInfo[ PS3_INPUT_PIPE ].bmRcvToggle = 0;

	rcode = pUsb->setEpInfoEntry(bAddress, 3, epInfo);
	if (rcode)
	{
		Serial.print("Failed setEpInfoEntry: ");
		Serial.println(rcode);
	}

    delay(200); //Give time for address change


	/* Set Configuration Value */
    rcode = pUsb->setConf(bAddress, epInfo[ PS3_CONTROL_PIPE ].deviceEpNum, 1);

	if (rcode)
	{
		Serial.print("setConf failed: ");
		Serial.println(rcode);
		return rcode;
	}

	
	if(PID == PS3_PID || PID == PS3NAVIGATION_PID) 
	{
		if(PID == PS3_PID) 
		{
			Serial.println("Dualshock 3 Controller Connected");
            PS3Connected = true;
		} 
		else 
		{ 
			/* must be a navigation controller */
			Serial.println("Navigation Controller Connected");
			PS3NavigationConnected = true;
		}
		
		/* The PS3 controller needs a special command before it starts sending data */        
		enable_sixaxis(); 

		/* Needed for PS3 Dualshock and Navigation commands to work */
		for (uint8_t i = 0; i < PS3_REPORT_BUFFER_SIZE; i++)
		{
            writeBuf[i] = pgm_read_byte(&PS3_REPORT_BUFFER[i]);
		}

		for (uint8_t i = 6; i < 10; i++)
		{
            readBuf[i] = 0x7F; // Set the analog joystick values to center position
		}
	} 

	ready = true;
	bPollEnable = true;
	
	Serial.println("Device configured.");

	return 0;
}

uint32_t UsbPS3::Poll() 
{
	if(!bPollEnable)
	{
        return 0;
	}

    if(PS3Connected || PS3NavigationConnected) 
	{
		/* This is completely different from the USB Host implementation. Not sure how theirs is supposed to work? */
		uint32_t BUFFER_SIZE = EP_MAXPKTSIZE;
	    pUsb->ctrlReq(bAddress, epInfo[PS3_CONTROL_PIPE].deviceEpNum, bmREQ_HID_IN, HID_REQUEST_GET_REPORT, 0x01, 0x01, 0x00, BUFFER_SIZE, BUFFER_SIZE, readBuf, NULL);		
						
		//TODO: Do we want to do this every Poll?
		ButtonState = (uint32_t)(readBuf[2] | ((uint16_t)readBuf[3] << 8) | ((uint32_t)readBuf[4] << 16));
		
		if (ButtonState != OldButtonState)
		{
			/* Update click state variable */
			ButtonClickState = ButtonState & ~OldButtonState;
			OldButtonState = ButtonState;
		}
	} 

	return 0;
}

bool UsbPS3::getButtonPress(ButtonEnum b) 
{		
	return (ButtonState & PS3_BUTTONS[(uint8_t)b]);
}

bool UsbPS3::getButtonClick(ButtonEnum b) 
{
    uint32_t button = pgm_read_dword(&PS3_BUTTONS[(uint8_t)b]);
    bool click = (ButtonClickState & button);
    ButtonClickState &= ~button; // Clear "click" event
    return click;
}

uint8_t UsbPS3::getAnalogButton(ButtonEnum a) 
{
	return (uint8_t)(readBuf[(pgm_read_byte(&PS3_ANALOG_BUTTONS[(uint8_t)a])) - 9]);
}

uint8_t UsbPS3::getAnalogHat(AnalogHatEnum a) 
{
	return (uint8_t)(readBuf[((uint8_t)a + 6)]);
}

uint16_t UsbPS3::getSensor(SensorEnum a) 
{
	return ((readBuf[((uint16_t)a) - 9] << 8) | readBuf[((uint16_t)a + 1) - 9]);
}

double UsbPS3::getAngle(AngleEnum a) 
{
	/* Warning - this could be slowish on a Due, better to use raw values */
    if(PS3Connected) 
	{
            double accXval;
            double accYval;
            double accZval;

            // Data for the Kionix KXPC4 used in the DualShock 3
            const double zeroG = 511.5; // 1.65/3.3*1023 (1,65V)
            accXval = -((double)getSensor(aX) - zeroG);
            accYval = -((double)getSensor(aY) - zeroG);
            accZval = -((double)getSensor(aZ) - zeroG);

            // Convert to 360 degrees resolution
            // atan2 outputs the value of -π to π (radians)
            // We are then converting it to 0 to 2π and then to degrees
            if(a == Pitch)
			{
                return (atan2(accYval, accZval) + PI) * RAD_TO_DEG;
			}
            else
			{
                return (atan2(accXval, accZval) + PI) * RAD_TO_DEG;
			}
    } 
	else
	{
        return 0;
	}
}

bool UsbPS3::getStatus(StatusEnum c) 
{
	return (readBuf[((uint16_t)c >> 8) - 9] == ((uint8_t)c & 0xff));
}

void UsbPS3::PS3_Command(uint8_t *data, uint16_t nbytes) 
{
	/* Playstation Sixaxis Dualshock and Navigation Controller commands */
    /*  bmRequest = Host to device (0x00) | Class (0x20) | Interface (0x01) = 0x21, bRequest = Set Report (0x09), Report ID (0x01), Report Type (Output 0x02), interface (0x00), datalength, datalength, data) */
    pUsb->ctrlReq(bAddress, epInfo[PS3_CONTROL_PIPE].deviceEpNum, bmREQ_HID_OUT, HID_REQUEST_SET_REPORT, 0x01, 0x02, 0x00, nbytes, nbytes, data, NULL);
}

void UsbPS3::setAllOff() 
{
    for(uint8_t i = 0; i < PS3_REPORT_BUFFER_SIZE; i++)
	{
        writeBuf[i] = pgm_read_byte(&PS3_REPORT_BUFFER[i]); // Reset buffer
	}

	PS3_Command(writeBuf, PS3_REPORT_BUFFER_SIZE);
}

void UsbPS3::setRumbleOff() 
{
    writeBuf[1] = 0x00;
    writeBuf[2] = 0x00; // Low mode off
    writeBuf[3] = 0x00;
    writeBuf[4] = 0x00; // High mode off

	PS3_Command(writeBuf, PS3_REPORT_BUFFER_SIZE);
}

void UsbPS3::setRumbleOn(RumbleEnum mode) 
{
	if((mode & 0x30) > 0x00) 
	{
		uint8_t power[2] = {0xff, 0x00}; // Defaults to RumbleLow
		if(mode == RumbleHigh) 
		{
			power[0] = 0x00;
			power[1] = 0xff;
		}
		
	    setRumbleOn(0xfe, power[0], 0xfe, power[1]);
	}
}

void UsbPS3::setRumbleOn(uint8_t rightDuration, uint8_t rightPower, uint8_t leftDuration, uint8_t leftPower) 
{
	writeBuf[1] = rightDuration;
	writeBuf[2] = rightPower;
	writeBuf[3] = leftDuration;
	writeBuf[4] = leftPower;
	
	PS3_Command(writeBuf, PS3_REPORT_BUFFER_SIZE);
}

void UsbPS3::setLedRaw(uint8_t value) 
{
	writeBuf[9] = value << 1;
	
	PS3_Command(writeBuf, PS3_REPORT_BUFFER_SIZE);
}

void UsbPS3::setLedOff(LEDEnum a) 
{
	writeBuf[9] &= ~((uint8_t)((pgm_read_byte(&PS3_LEDS[(uint8_t)a]) & 0x0f) << 1));

	PS3_Command(writeBuf, PS3_REPORT_BUFFER_SIZE);
}

void UsbPS3::setLedOn(LEDEnum a) 
{
	if (a == OFF)
	{
        setLedRaw(0);
	}
	else 
	{
		writeBuf[9] |= (uint8_t)((pgm_read_byte(&PS3_LEDS[(uint8_t)a]) & 0x0f) << 1);
		PS3_Command(writeBuf, PS3_REPORT_BUFFER_SIZE);
	}
}

void UsbPS3::setLedToggle(LEDEnum a) 
{
	writeBuf[9] ^= (uint8_t)((pgm_read_byte(&PS3_LEDS[(uint8_t)a]) & 0x0f) << 1);
	PS3_Command(writeBuf, PS3_REPORT_BUFFER_SIZE);
}

void UsbPS3::enable_sixaxis() 
{ 
	/* Command used to enable the Dualshock 3 and Navigation controller to send data via USB */
	uint8_t cmd_buf[4];
	
	/* Special PS3 Controller enable commands */
	cmd_buf[0] = 0x42; 
	cmd_buf[1] = 0x0c;
	cmd_buf[2] = 0x00;
	cmd_buf[3] = 0x00;

	pUsb->ctrlReq(bAddress, epInfo[PS3_CONTROL_PIPE].deviceEpNum, bmREQ_HID_OUT, HID_REQUEST_SET_REPORT, 0xF4, 0x03, 0x00, 4, 4, cmd_buf, NULL);
}

uint32_t UsbPS3::Release() 
{
	PS3Connected = false;
	PS3MoveConnected = false;
	PS3NavigationConnected = false;
	pUsb->GetAddressPool().FreeAddress(bAddress);
	bAddress = 0;
	bPollEnable = false;
	return 0;
}
