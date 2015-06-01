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
	Base driver based on code from USB-MIDI class driver for USB Host Shield 2.0 Library
	which was Copyright 2012-2013 Yuuichi Akagawa

	which in turn was based on an idea from LPK25 USB-MIDI to Serial MIDI converter
	by Collin Cunningham - makezine.com, narbotic.com
*/

#include "UsbMidi.h"
#include "JSONUtil.h"
#include "EventManager.h"
#include "Key.h"

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
	rcode = pUsb->getDevDescr(0, 0, DESC_BUFF_SIZE, (uint8_t*)buf);

	/* Restore p->epinfo */
	p->epinfo = oldep_ptr;

	if (rcode)
	{
		Serial.print("Failed to get device descriptor. Trying again ");
		Serial.println(rcode);

		rcode = pUsb->getDevDescr(0, 0, sizeof(USB_DEVICE_DESCRIPTOR), (uint8_t*)buf);

		if (rcode)
		{
			Serial.print("Failed to get device descriptor for good: ");
			Serial.println(rcode);

			return rcode;
		}
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

/* Receive data from MIDI device */
uint32_t USBMidiDevice::recvData(uint8_t *outBuf)
{
	uint32_t bytesReceived = 0;
	uint32_t rcode;

	if (this->ready == false)
	{
		return 0;
	}

	/* Clear out the packet buffer */
	midiPacket[0] = 0;
	midiPacket[1] = 0;
	midiPacket[2] = 0;
	midiPacket[3] = 0;

	/* Read a single packet */
	rcode = this->read(&bytesReceived, MIDI_EVENT_PACKET_SIZE, midiPacket);

	if (rcode != 0)
	{
		return 0;
	}
  
	/* if all data is zero, no valid data received. */
	if (midiPacket[0] == 0 && midiPacket[1] == 0 && midiPacket[2] == 0 && midiPacket[3] == 0)
	{
		//Serial.println("Empty data packet received.");
	    return 0;
	}

	outBuf[0] = midiPacket[1];
	outBuf[1] = midiPacket[2];
	outBuf[2] = midiPacket[3];

	return 3;
}

void USBMidiController::task()
{
	USBMidiDevice::task();

	/* When USB is ready, start reading midi commands */
    if (this->isReady())
    {
	    uint8_t buffer[3];
		uint32_t size = 0;
	
		/* As long as there are bytes to read, keep reading */
	    do
		{
			if ((size = this->recvData(buffer)) > 0)
			{
				this->processMessage(size, buffer);
			}
	    }
		while (size > 0);
    }
}	

void USBMidiController::processMessage(uint32_t size, uint8_t* buffer)
{
	uint32_t command = GET_MIDI_COMMAND(buffer[0]);
	uint32_t channel = GET_MIDI_CHANNEL(buffer[0]);

	switch (command)
	{
		case MIDI_NOTE_OFF:

			this->onNoteOff(channel, buffer[1], buffer[2]);
			break;

		case MIDI_NOTE_ON:

			this->onNoteOn(channel, buffer[1], buffer[2]);
			break;

		case MIDI_PRESSURE:

			this->onPressure(channel, buffer[1], buffer[2]);
			break;

		case MIDI_CONTROL:

			this->onControlChange(channel, buffer[1], buffer[2]);
			break;

		case MIDI_PROGRAM:

			this->onProgramChange(channel, buffer[1]);
			break;

		case MIDI_ATOUCH:

			this->onAftertouch(channel, buffer[1]);
			break;

		case MIDI_PITCHBEND:

			this->onPitchbend(channel, GET_MIDI_14BIT(buffer[2],buffer[1]));
			break;

		default:
		
			/* Unsupported */
			break;
	}
}

USBMidiController::USBMidiController() : USBMidiDevice()
{
}

USBMidiCCController* USBMidiCCController::create()
{
	return new USBMidiCCController();
}

USBMidiCCController* USBMidiCCController::create(aJsonObject* data)
{
	USBMidiCCController* controller = new USBMidiCCController();

	controller->addControlPins(data);
	
	return controller;
}

void USBMidiCCController::addControlPins(aJsonObject* data)
{
	const char pinsNodeName[] = "controllerMap";
	const char controllerNodeName[] = "controlNumber";
	const char outputNodeName[] = "output";
	const char pressureNodeName[] = "bipolar";

	aJsonObject* controllerMap = aJson.getObjectItem(data, pinsNodeName);

	if (controllerMap == NULL) return;
	
	for (int i = 0; i < aJson.getArraySize(controllerMap); i++)
	{
		aJsonObject* controllerNode = aJson.getArrayItem(controllerMap, i);

		int controller = getIntFromJSON(controllerNode, controllerNodeName, 1, 0, 127);
		PinAnalogOut output = getAnalogOutputFromJSON(controllerNode, outputNodeName);
		bool bipolar = getBoolFromJSON(controllerNode, pressureNodeName, false);
		
		this->addControlPin(controller, output, bipolar ? CC_RANGE_BIPOLAR : CC_RANGE_UNIPOLAR);
	}		
}

USBMidiCCController::USBMidiCCController() : USBMidiController()
{
}

void USBMidiCCController::addControlPin(uint32_t controller, PinAnalogOut output, CCRange range)
{
	ControlOutput outconfig;
	
	outconfig.controller = controller;
	outconfig.output = AnalogOut::create(output);
	outconfig.range = range;
	
	outputs.push_back(outconfig);
}

void USBMidiCCController::timer(uint32_t t)
{
	
}

void USBMidiCCController::onControlChange(uint32_t channel, uint32_t controller, uint32_t value)
{
	for (uint32_t i = 0; i < outputs.size(); i++)
	{
		if (controller == outputs[i].controller)
		{
			if (outputs[i].range == CC_RANGE_BIPOLAR)
			{
				outputs[i].output->outputRaw(GET_BIPOLAR_12BITCV(value));
			}
			else
			{
				outputs[i].output->outputRaw(GET_12BITCV(value));
			}
		}
	}
}


USBMonophonicMidiController* USBMonophonicMidiController::create(aJsonObject* data)
{
	const char gateNodeName[] = "gate";
	const char pitchNodeName[] = "pitch";
	const char velocityNodeName[] = "velocity";
	const char pressureNodeName[] = "pressure";
	const char aftertouchNodeName[] = "aftertouch";
	const char triggerOnNodeName[] = "triggerOn";
	const char triggerOffNodeName[] = "triggerOff";
	
	PinDigitalOut gate = getDigitalOutputFromJSON(data, gateNodeName);
	PinDigitalOut triggerOn = getDigitalOutputFromJSON(data, triggerOnNodeName);
	PinDigitalOut triggerOff = getDigitalOutputFromJSON(data, triggerOnNodeName);
	PinAnalogOut pitch = getAnalogOutputFromJSON(data, pitchNodeName);
	PinAnalogOut velocity = getAnalogOutputFromJSON(data, velocityNodeName);
	PinAnalogOut pressure = getAnalogOutputFromJSON(data, pressureNodeName);
	PinAnalogOut aftertouch = getAnalogOutputFromJSON(data, aftertouchNodeName);
				
	USBMonophonicMidiController* controller = new USBMonophonicMidiController(gate, triggerOn, triggerOff, pitch, velocity, pressure, aftertouch);	
	
	controller->addControlPins(data);

	return controller;
}

//TODO: Too much duplicate code between these!
USBMonophonicMidiController* USBMonophonicMidiController::create(PinDigitalOut gatePin, PinDigitalOut triggerOn, PinDigitalOut triggerOff, PinAnalogOut pitchPin, PinAnalogOut velocityPin, PinAnalogOut pressureOut, PinAnalogOut afterTouchOut)
{
	return new USBMonophonicMidiController(gatePin, triggerOn, triggerOff, pitchPin, velocityPin, pressureOut, afterTouchOut);
}

USBMonophonicMidiController::USBMonophonicMidiController(PinDigitalOut gatePin, PinDigitalOut triggerOn, PinDigitalOut triggerOff, PinAnalogOut pitchPin, PinAnalogOut velocityPin, PinAnalogOut pressurePin, PinAnalogOut afterTouchPin) : USBMidiCCController()
{
	this->gate = gatePin;
	this->pitch = (pitchPin != ANALOG_OUT_NONE) ? AnalogOut::create(pitchPin) : NULL;
	this->velocity = (velocityPin != ANALOG_OUT_NONE) ? AnalogOut::create(velocityPin) : NULL;
	this->pressure = (pressurePin != ANALOG_OUT_NONE) ? AnalogOut::create(pressurePin) : NULL;
	this->afterTouch = (afterTouchPin != ANALOG_OUT_NONE) ? AnalogOut::create(afterTouchPin) : NULL;
	this->triggerOn = (triggerOn != DIGITAL_OUT_NONE) ? Gate::create(triggerOn, 30) : NULL;
	this->triggerOff = (triggerOff != DIGITAL_OUT_NONE) ? Gate::create(triggerOff, 30) : NULL;
}

void USBMonophonicMidiController::timer(uint32_t t)
{
	if (this->triggerOn != NULL) this->triggerOn->timer(t);	
	if (this->triggerOff != NULL) this->triggerOff->timer(t);
}

void USBMonophonicMidiController::onNoteOn(uint32_t channel, uint32_t note, uint32_t velocity)
{
	/* Set the pitch and be sure to include any pitchbend */
	this->pitchValue = millivoltFromMidiNote(note);
	if (this->pitch != NULL) this->pitch->outputCV(this->pitchValue + this->pitchbendValue);
	
	/* Update the velocity output */
	if (this->velocity != NULL) this->velocity->outputRaw(GET_12BITCV(velocity));

	/* Trigger note-on */
	if (this->triggerOn != NULL) this->triggerOn->reset();

	/* Open the gate */
	if (this->gate != DIGITAL_OUT_NONE) digitalWrite(this->gate, HIGH);	
	
	/* Keep track of it in the note stack */
	this->noteStack.noteOn(note, velocity);
}

void USBMonophonicMidiController::onNoteOff(uint32_t channel, uint32_t note, uint32_t velocity)
{
	NoteListEntry playingNow = this->noteStack.mostRecentNote();
	
	/* Remove from the note stack */
	this->noteStack.noteOff(note);

	/* See if ihe note that turned off was the currently playing one */
	if (playingNow.note == note)
	{
		/* Signal note-off trigger */
		if (this->triggerOff != NULL) this->triggerOff->reset();
	
		/* See if any more notes are pressed */
		if (this->noteStack.getSize() == 0)
		{
			/* Close the gate */
			if (this->gate != DIGITAL_OUT_NONE) digitalWrite(this->gate, LOW);

			/* Reset velocity, pressure, and aftertouch */
			if (this->velocity != NULL) this->velocity->outputRaw(2048);
		}
		else
		{
			/* The one that was let go was currently playing and still at least one note pressed... */
			NoteListEntry mostRecent = this->noteStack.mostRecentNote();

			/* Set the pitch and be sure to include any pitchbend */
			this->pitchValue = millivoltFromMidiNote(mostRecent.note);
			if (this->pitch != NULL) this->pitch->outputCV(this->pitchValue + this->pitchbendValue);

			/* Update the velocity output */
			if (this->velocity != NULL) this->velocity->outputRaw(GET_12BITCV(mostRecent.velocity));
		}
	}
}

void USBMonophonicMidiController::onPressure(uint32_t channel, uint32_t note, uint32_t pressure)
{
	/* Only respond if this is the current note playing */
	if (this->pressure != NULL && note == this->noteStack.mostRecentNote().note) 
	{
		this->pressure->outputRaw(GET_12BITCV(pressure));
	}
}

void USBMonophonicMidiController::onAftertouch(uint32_t channel, uint32_t value)
{
	if (this->afterTouch != NULL) this->afterTouch->outputRaw(GET_12BITCV(value));
}

void USBMonophonicMidiController::onPitchbend(uint32_t channel, uint32_t value)
{
	/* Keep track of how much pitch bend we have and add/subtract the current pitch */
	this->pitchbendValue = ((value - 0x2000) * 12) / 1000;
	if (this->pitch != NULL) this->pitch->outputCV(this->pitchValue + this->pitchbendValue);
}	
	

USBSplitMonoMidiController* USBSplitMonoMidiController::create(PinDigitalOut gatePin1, PinDigitalOut triggerOn1, PinDigitalOut triggerOff1, PinAnalogOut pitchPin1, PinAnalogOut velocityPin1, PinAnalogOut pressureOut1, PinDigitalOut gatePin2, PinDigitalOut triggerOn2, PinDigitalOut triggerOff2, PinAnalogOut pitchPin2, PinAnalogOut velocityPin2, PinAnalogOut pressureOut2, PinAnalogOut afterTouchOut, uint32_t splitNote)
{
	return new USBSplitMonoMidiController(gatePin1, triggerOn1, triggerOff1, pitchPin1, velocityPin1, pressureOut1, gatePin2, triggerOn2, triggerOff2, pitchPin2, velocityPin2, pressureOut2, afterTouchOut, splitNote);
}

USBSplitMonoMidiController* USBSplitMonoMidiController::create(aJsonObject* data)
{
	const char gateNodeName1[] = "gate1";
	const char pitchNodeName1[] = "pitch1";
	const char velocityNodeName1[] = "velocity1";
	const char pressureNodeName1[] = "pressure1";
	const char triggerOnNodeName1[] = "triggerOn1";
	const char triggerOffNodeName1[] = "triggerOff1";
	const char gateNodeName2[] = "gate2";
	const char pitchNodeName2[] = "pitch2";
	const char velocityNodeName2[] = "velocity2";
	const char pressureNodeName2[] = "pressure2";
	const char triggerOnNodeName2[] = "triggerOn2";
	const char triggerOffNodeName2[] = "triggerOff2";
	const char aftertouchNodeName[] = "aftertouch";
	const char noteNodeName[] = "splitNote";
	
	PinDigitalOut gate1 = getDigitalOutputFromJSON(data, gateNodeName1);
	PinDigitalOut triggerOn1 = getDigitalOutputFromJSON(data, triggerOnNodeName1);
	PinDigitalOut triggerOff1 = getDigitalOutputFromJSON(data, triggerOnNodeName1);
	PinAnalogOut pitch1 = getAnalogOutputFromJSON(data, pitchNodeName1);
	PinAnalogOut velocity1 = getAnalogOutputFromJSON(data, velocityNodeName1);
	PinAnalogOut pressure1 = getAnalogOutputFromJSON(data, pressureNodeName1);
	PinDigitalOut gate2 = getDigitalOutputFromJSON(data, gateNodeName2);
	PinDigitalOut triggerOn2 = getDigitalOutputFromJSON(data, triggerOnNodeName2);
	PinDigitalOut triggerOff2 = getDigitalOutputFromJSON(data, triggerOnNodeName2);
	PinAnalogOut pitch2 = getAnalogOutputFromJSON(data, pitchNodeName2);
	PinAnalogOut velocity2 = getAnalogOutputFromJSON(data, velocityNodeName2);
	PinAnalogOut pressure2 = getAnalogOutputFromJSON(data, pressureNodeName2);
	PinAnalogOut aftertouch = getAnalogOutputFromJSON(data, aftertouchNodeName);
	int splitNote = getIntFromJSON(data, noteNodeName, 50, 0, 127);
				
	USBSplitMonoMidiController* controller = new USBSplitMonoMidiController(gate1, triggerOn1, triggerOff1, pitch1, velocity1, pressure1, gate2, triggerOn2, triggerOff2, pitch2, velocity2, pressure2, aftertouch, splitNote);	

	controller->addControlPins(data);	
	
	return controller;
}

USBSplitMonoMidiController::USBSplitMonoMidiController(PinDigitalOut gatePin1, PinDigitalOut triggerOn1, PinDigitalOut triggerOff1, PinAnalogOut pitchPin1, PinAnalogOut velocityPin1, PinAnalogOut pressurePin1, PinDigitalOut gatePin2, PinDigitalOut triggerOn2, PinDigitalOut triggerOff2, PinAnalogOut pitchPin2, PinAnalogOut velocityPin2, PinAnalogOut pressurePin2, PinAnalogOut afterTouchOut, uint32_t splitNote) : USBMidiCCController()
{
	this->gate1 = gatePin1;
	this->pitch1 = (pitchPin1 != ANALOG_OUT_NONE) ? AnalogOut::create(pitchPin1) : NULL;
	this->velocity1 = (velocityPin1 != ANALOG_OUT_NONE) ? AnalogOut::create(velocityPin1) : NULL;
	this->pressure1 = (pressurePin1 != ANALOG_OUT_NONE) ? AnalogOut::create(pressurePin1) : NULL;
	this->triggerOn1 = (triggerOn1 != DIGITAL_OUT_NONE) ? Gate::create(triggerOn1, 30) : NULL;
	this->triggerOff1 = (triggerOff1 != DIGITAL_OUT_NONE) ? Gate::create(triggerOff1, 30) : NULL;	

	this->gate2 = gatePin2;
	this->pitch2 = (pitchPin2 != ANALOG_OUT_NONE) ? AnalogOut::create(pitchPin2) : NULL;
	this->velocity2 = (velocityPin2 != ANALOG_OUT_NONE) ? AnalogOut::create(velocityPin2) : NULL;
	this->pressure2 = (pressurePin2 != ANALOG_OUT_NONE) ? AnalogOut::create(pressurePin2) : NULL;
	this->triggerOn2 = (triggerOn2 != DIGITAL_OUT_NONE) ? Gate::create(triggerOn2, 30) : NULL;
	this->triggerOff2 = (triggerOff2 != DIGITAL_OUT_NONE) ? Gate::create(triggerOff2, 30) : NULL;

	this->afterTouch = (afterTouchOut != ANALOG_OUT_NONE) ? AnalogOut::create(afterTouchOut) : NULL;
	this->splitNote = splitNote;
}

void USBSplitMonoMidiController::timer(uint32_t t)
{
	if (this->triggerOn1 != NULL) this->triggerOn1->timer(t);	
	if (this->triggerOff1 != NULL) this->triggerOff1->timer(t);

	if (this->triggerOn2 != NULL) this->triggerOn2->timer(t);	
	if (this->triggerOff2 != NULL) this->triggerOff2->timer(t);
}

void USBSplitMonoMidiController::onNoteOn(uint32_t channel, uint32_t note, uint32_t velocity)
{
	if (note < this->splitNote)
	{
		/* Set the pitch and be sure to include any pitchbend */
		this->pitchValue1 = millivoltFromMidiNote(note);
		if (this->pitch1 != NULL) this->pitch1->outputCV(this->pitchValue1 + this->pitchbendValue);
	
		/* Update the velocity output */
		if (this->velocity1 != NULL) this->velocity1->outputRaw(GET_12BITCV(velocity));

		/* Trigger note-on */
		if (this->triggerOn1 != NULL) this->triggerOn1->reset();

		/* Open the gate */
		if (this->gate1 != DIGITAL_OUT_NONE) digitalWrite(this->gate1, HIGH);	
	
		/* Keep track of it in the note stack */
		this->noteStack1.noteOn(note, velocity);
	}
	else
	{
		/* Set the pitch and be sure to include any pitchbend */
		this->pitchValue2 = millivoltFromMidiNote(note);
		if (this->pitch2 != NULL) this->pitch2->outputCV(this->pitchValue2 + this->pitchbendValue);
	
		/* Update the velocity output */
		if (this->velocity2 != NULL) this->velocity2->outputRaw(GET_12BITCV(velocity));

		/* Trigger note-on */
		if (this->triggerOn2 != NULL) this->triggerOn2->reset();

		/* Open the gate */
		if (this->gate2 != DIGITAL_OUT_NONE) digitalWrite(this->gate2, HIGH);	
	
		/* Keep track of it in the note stack */
		this->noteStack2.noteOn(note, velocity);
	}
}

void USBSplitMonoMidiController::onNoteOff(uint32_t channel, uint32_t note, uint32_t velocity)
{
	if (note < this->splitNote)
	{
		NoteListEntry playingNow = this->noteStack1.mostRecentNote();
	
		/* Remove from the note stack */
		this->noteStack1.noteOff(note);

		/* See if ihe note that turned off was the currently playing one */
		if (playingNow.note == note)
		{
			/* Signal note-off trigger */
			if (this->triggerOff1 != NULL) this->triggerOff1->reset();
	
			/* See if any more notes are pressed */
			if (this->noteStack1.getSize() == 0)
			{
				/* Close the gate */
				if (this->gate1 != DIGITAL_OUT_NONE) digitalWrite(this->gate1, LOW);

				/* Reset velocity, pressure, and aftertouch */
				if (this->velocity1 != NULL) this->velocity1->outputRaw(velocity);
			}
			else
			{
				/* The one that was let go was currently playing and still at least one note pressed... */
				NoteListEntry mostRecent = this->noteStack1.mostRecentNote();

				/* Set the pitch and be sure to include any pitchbend */
				this->pitchValue1 = millivoltFromMidiNote(mostRecent.note);
				if (this->pitch1 != NULL) this->pitch1->outputCV(this->pitchValue1 + this->pitchbendValue);

				/* Update the velocity output */
				if (this->velocity1 != NULL) this->velocity1->outputRaw(GET_12BITCV(mostRecent.velocity));
			}
		}
	}
	else
	{
		NoteListEntry playingNow = this->noteStack2.mostRecentNote();
	
		/* Remove from the note stack */
		this->noteStack2.noteOff(note);

		/* See if ihe note that turned off was the currently playing one */
		if (playingNow.note == note)
		{
			/* Signal note-off trigger */
			if (this->triggerOff2 != NULL) this->triggerOff2->reset();
	
			/* See if any more notes are pressed */
			if (this->noteStack2.getSize() == 0)
			{
				/* Close the gate */
				if (this->gate2 != DIGITAL_OUT_NONE) digitalWrite(this->gate2, LOW);

				/* Reset velocity, pressure, and aftertouch */
				if (this->velocity2 != NULL) this->velocity2->outputRaw(2048);
			}
			else
			{
				/* The one that was let go was currently playing and still at least one note pressed... */
				NoteListEntry mostRecent = this->noteStack2.mostRecentNote();

				/* Set the pitch and be sure to include any pitchbend */
				this->pitchValue2 = millivoltFromMidiNote(mostRecent.note);
				if (this->pitch2 != NULL) this->pitch2->outputCV(this->pitchValue2 + this->pitchbendValue);

				/* Update the velocity output */
				if (this->velocity2 != NULL) this->velocity2->outputRaw(GET_12BITCV(mostRecent.velocity));
			}
		}
	}
}

void USBSplitMonoMidiController::onPressure(uint32_t channel, uint32_t note, uint32_t pressure)
{
	if (note < this->splitNote)
	{
		/* Only respond if this is the current note playing */
		if (this->pressure1 != NULL && note == this->noteStack1.mostRecentNote().note) 
		{
			this->pressure1->outputRaw(GET_12BITCV(pressure));
		}
	}
	else
	{
		/* Only respond if this is the current note playing */
		if (this->pressure2 != NULL && note == this->noteStack2.mostRecentNote().note) 
		{
			this->pressure2->outputRaw(GET_12BITCV(pressure));
		}
	}
}

void USBSplitMonoMidiController::onAftertouch(uint32_t channel, uint32_t value)
{
	if (this->afterTouch != NULL) this->afterTouch->outputRaw(GET_12BITCV(value));
}

void USBSplitMonoMidiController::onPitchbend(uint32_t channel, uint32_t value)
{
	/* Keep track of how much pitch bend we have and add/subtract the current pitch */
	this->pitchbendValue = ((value - 0x2000) * 12) / 1000;

	if (this->pitch1 != NULL) this->pitch1->outputCV(this->pitchValue1 + this->pitchbendValue);
	if (this->pitch2 != NULL) this->pitch2->outputCV(this->pitchValue2 + this->pitchbendValue);
}	

USBPolyphonicMidiController* USBPolyphonicMidiController::create(PinAnalogOut afterTouchOut)
{
	return new USBPolyphonicMidiController(afterTouchOut);
}

USBPolyphonicMidiController* USBPolyphonicMidiController::create(aJsonObject* data)
{
}

USBPolyphonicMidiController::USBPolyphonicMidiController(PinAnalogOut afterTouchPin) : USBMidiCCController()
{
	this->afterTouch = (afterTouchPin != ANALOG_OUT_NONE) ? AnalogOut::create(afterTouchPin) : NULL;
}

void USBPolyphonicMidiController::addVoice(PinDigitalOut gatePin, PinDigitalOut triggerOn, PinDigitalOut triggerOff, PinAnalogOut pitchPin, PinAnalogOut velocityPin, PinAnalogOut pressurePin)
{
	Voice voice;
	
	voice.gate = gatePin;
	voice.pitch = (pitchPin != ANALOG_OUT_NONE) ? AnalogOut::create(pitchPin) : NULL;
	voice.velocity = (velocityPin != ANALOG_OUT_NONE) ? AnalogOut::create(velocityPin) : NULL;
	voice.pressure = (pressurePin != ANALOG_OUT_NONE) ? AnalogOut::create(pressurePin) : NULL;
	voice.triggerOn = (triggerOn != DIGITAL_OUT_NONE) ? Gate::create(triggerOn, 30) : NULL;
	voice.triggerOff = (triggerOff != DIGITAL_OUT_NONE) ? Gate::create(triggerOff, 30) : NULL;
	
	this->voices.push_back(voice);
}

void USBPolyphonicMidiController::onNoteOn(uint32_t channel, uint32_t note, uint32_t velocity)
{
	// if (this->voices.size() > 0)
	// {
	// 	/* If the current voice is already in use, don't steal it. Ignore the note on */
	//  //TODO: configurable voice allocation strategies
	// 	if (!this->voices[this->currentVoice].allocated)
	// 	{
	// 		/* Set the pitch and be sure to include any pitchbend */
	// 		this->pitchValue = millivoltFromMidiNote(note);
	// 		if (this->pitch != NULL) this->pitch->outputCV(this->pitchValue + this->pitchbendValue);
	//
	// 		/* Update the velocity output */
	// 		if (this->velocity != NULL) this->velocity->outputRaw(4095 - (velocity << 4));
	//
	// 		/* Trigger note-on */
	// 		if (this->triggerOn != NULL) this->triggerOn->reset();
	//
	// 		/* Open the gate */
	// 		if (this->gate != DIGITAL_OUT_NONE) digitalWrite(this->gate, HIGH);
	// 	}
	// }
}
	
USBMidiTriggers* USBMidiTriggers::create(aJsonObject* data)
{
	const char mapNodeName[] = "drumMap";
	const char noteNodeName[] = "note";
	const char outputNodeName[] = "output";
	const char velocityNodeName[] = "velocity";
	
	
	USBMidiTriggers* triggerController = new USBMidiTriggers();
	
	aJsonObject* drumMap = aJson.getObjectItem(data, mapNodeName);

	if (drumMap == NULL) return NULL;
	
	for (int i = 0; i < aJson.getArraySize(drumMap); i++)
	{
		aJsonObject* triggerNode = aJson.getArrayItem(drumMap, i);

		int note = getIntFromJSON(triggerNode, noteNodeName, 1, 0, 127);
		PinDigitalOut output = getDigitalOutputFromJSON(triggerNode, outputNodeName);
		PinAnalogOut velocity = getAnalogOutputFromJSON(triggerNode, velocityNodeName);
		
		triggerController->addTrigger(note, velocity, output);
	}		
	
	return triggerController;
}

USBMidiTriggers::USBMidiTriggers() : USBMidiCCController()
{
	//TODO: Trigger mode when we won't be getting note-offs?
}

void USBMidiTriggers::addTrigger(uint32_t note, PinAnalogOut velocity, PinDigitalOut output)
{
	AnalogOut* velocityOut = (velocity != ANALOG_OUT_NONE) ? AnalogOut::create(velocity) : NULL;
	
	this->outputs.push_back({ note, velocityOut, output });
}

void USBMidiTriggers::onNoteOn(uint32_t channel, uint32_t note, uint32_t velocity)
{
	for (uint32_t i = 0; i < this->outputs.size(); i++)
	{
		if (this->outputs[i].note == note)
		{
			Serial.println(note);
			/* Update the velocity output */
			if (this->outputs[i].velocity != NULL) this->outputs[i].velocity->outputRaw(GET_12BITCV(velocity));

			/* Open the gate */
			digitalWrite(this->outputs[i].output, HIGH);
		}
	}
}

void USBMidiTriggers::onNoteOff(uint32_t channel, uint32_t note, uint32_t velocity)
{
	for (uint32_t i = 0; i < this->outputs.size(); i++)
	{
		if (this->outputs[i].note == note)
		{
			/* Update the velocity output */
			if (this->outputs[i].velocity != NULL) this->outputs[i].velocity->outputRaw(GET_12BITCV(velocity));

			/* Close the gate */
			digitalWrite(this->outputs[i].output, LOW);
		}
	}
}

USBMidiApeggiator* USBMidiApeggiator::create(PinDigitalOut gatePin, PinDigitalOut triggerPin, PinAnalogOut pitchPin, PinAnalogOut velocityPin, PinAnalogOut pressurePin, PinAnalogOut afterTouchOut, PinAnalogIn density, NoteStackSortOrder sortOrder, PinAnalogIn octaves, PinDigitalIn latch)
{
	return new USBMidiApeggiator(gatePin, triggerPin, pitchPin, velocityPin, pressurePin, afterTouchOut, density, sortOrder, octaves, latch);
}


USBMidiApeggiator* USBMidiApeggiator::create(aJsonObject* data)
{

}


USBMidiApeggiator::USBMidiApeggiator(PinDigitalOut gatePin, PinDigitalOut triggerPin, PinAnalogOut pitchPin, PinAnalogOut velocityPin, PinAnalogOut pressurePin, PinAnalogOut afterTouchPin, PinAnalogIn density, NoteStackSortOrder sortOrder, PinAnalogIn octaves, PinDigitalIn latch)
{
	this->gate = gatePin;
	this->trigger = (triggerPin != DIGITAL_OUT_NONE) ? Gate::create(triggerPin, 30) : NULL;
	this->pitch = (pitchPin != ANALOG_OUT_NONE) ? AnalogOut::create(pitchPin) : NULL;
	this->velocity = (velocityPin != ANALOG_OUT_NONE) ? AnalogOut::create(velocityPin) : NULL;
	this->pressure = (pressurePin != ANALOG_OUT_NONE) ? AnalogOut::create(pressurePin) : NULL;
	this->afterTouch = (afterTouchPin != ANALOG_OUT_NONE) ? AnalogOut::create(afterTouchPin) : NULL;

	this->density = density;
	this->pattern = pattern;
	this->sortOrder = sortOrder;
	this->octaveInput = octaves;
	this->latch = latch;
}

void USBMidiApeggiator::onNoteOn(uint32_t channel, uint32_t note, uint32_t velocity)
{	
	/* Keep track of it in the note stack */
	this->noteStack.noteOn(note, velocity);
	
	/* If needs to be ordered, then sort */
	if ((this->sortOrder == NOTE_SORT_UPDOWN) || (this->sortOrder == NOTE_SORT_HIGHTOLOW) || (this->sortOrder == NOTE_SORT_LOWTOHIGH))
	{
		this->noteStack.sort();
	}
}

void USBMidiApeggiator::onNoteOff(uint32_t channel, uint32_t note, uint32_t velocity)
{	
	/* Remove from the note stack */
	this->noteStack.noteOff(note);
}

void USBMidiApeggiator::timer(uint32_t t)
{
	if (this->trigger != NULL) this->trigger->timer(t);
	
	/* Close the gate */
	if (this->noteStack.getSize() == 0)
	{
		if (this->gate != DIGITAL_OUT_NONE) digitalWrite(this->gate, LOW);
	}	
}

void USBMidiApeggiator::reset()
{
	//TODO: Order the sequence based on the setting
	//TODO: trigger input
	
	if (this->noteStack.getSize() > 0)
	{
		this->currentOctave = (this->currentOctave + 1) % (this->octaves + 1);
		
		this->noteIndex = (this->noteIndex + 1) % this->noteStack.getSize();

		NoteListEntry note = this->noteStack.getNote(this->noteIndex);

		/* Set the pitch and be sure to include any pitchbend */
		uint32_t pitchValue = (this->currentOctave * 1000) + millivoltFromMidiNote(note.note);
		if (this->pitch != NULL) this->pitch->outputCV(pitchValue + this->pitchbendValue);

		Serial.println(this->currentOctave);

		/* Update the velocity output */
		if (this->velocity != NULL) this->velocity->outputRaw(GET_12BITCV(note.velocity));

		/* Trigger note-on */
		if (this->trigger != NULL) this->trigger->reset();

		/* Open the gate */
		if (this->gate != DIGITAL_OUT_NONE) digitalWrite(this->gate, HIGH);
	}
	
	/* Read the octave input */
	if (this->octaveInput != ANALOG_IN_NONE)
	{
		int rawVal = (analogRead(this->octaveInput) - 2048);
		
		/* Limit it to the positive range */
		rawVal = (rawVal < 0) ? 0 : (rawVal > 2047) ? 2047 : rawVal;

		/* Scale 0 to 4 (up to 5 octaves) */
		this->octaves = rawVal / 410;
	}
}




