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
	Based on code from USB-MIDI class driver for USB Host Shield 2.0 Library
	which was Copyright 2012-2013 Yuuichi Akagawa

	which in turn was based on an idea from LPK25 USB-MIDI to Serial MIDI converter
	by Collin Cunningham - makezine.com, narbotic.com
*/

#ifndef UsbMidi_H
#define UsbMidi_H

#include "Usb.h"
#include "EventManager.h"
#include "confdescparser.h"
#include "../aJson/aJson.h"
#include "IO.h"
#include "Gate.h"

/* endpoint 0, bulk_IN(MIDI), bulk_OUT(MIDI), bulk_IN(VSP), bulk_OUT(VSP) */
#define MIDI_MAX_ENDPOINTS 5 
#define USB_SUBCLASS_MIDISTREAMING 3
#define DESC_BUFF_SIZE 256
#define MIDI_EVENT_PACKET_SIZE 4

#define GET_MIDI_COMMAND(X)		X >> 4
#define GET_MIDI_CHANNEL(X) 	X & 0x0F
#define GET_MIDI_14BIT(X, Y)	(X << 7) | Y

#define MIDI_NOTE_OFF 	0b00001000
#define MIDI_NOTE_ON	0b00001001
#define MIDI_PRESSURE	0b00001010
#define MIDI_CONTROL	0b00001011
#define MIDI_PROGRAM	0b00001100
#define MIDI_ATOUCH		0b00001101
#define MIDI_PITCHBEND	0b00001110


namespace nw2s
{
	typedef struct ControlOutput
	{
		uint32_t ccNumber;
		AnalogOut* output;
	};
	
	
	class USBMidiDevice;
	class USBMidiController;
	class USBMidiCCController;
	class USBMonophonicMidiController;
}

class nw2s::USBMidiDevice : public USBDeviceConfig, public UsbBasedDevice 
{
	protected:

		static const uint32_t epDataInIndex;			// DataIn endpoint index
		static const uint32_t epDataOutIndex;			// DataOUT endpoint index
	    static const uint32_t epDataInIndexVSP;			// DataIn endpoint index(Vendor Specific Protocl)
	    static const uint32_t epDataOutIndexVSP;		// DataOUT endpoint index(Vendor Specific Protocl)

		/* Mandatory members */
		uint32_t	bAddress;							// Device USB address
		uint32_t	bConfNum;							// configuration number
		uint8_t 	bControlIface; 						// Control interface value
		uint32_t	bNumEP;								// total number of EP in the configuration
		bool		ready;
	    bool		isMidiFound;

		/* Endpoint data structure describing the device EP */
		EpInfo		epInfo[MIDI_MAX_ENDPOINTS];
		
	    /* MIDI Event packet buffer */
	    uint8_t midiPacket[MIDI_EVENT_PACKET_SIZE];
	    //uint8_t readPtr;

	    void parseConfigDescr(uint32_t addr, uint32_t conf);
	    //uint32_t lookupMsgSize(uint8_t midiMsg);

	public:
		
		USBMidiDevice();

		/* Basic IO */
		uint32_t read(uint32_t *nreadbytes, uint32_t datalen, uint8_t *dataptr);
		uint32_t write(uint32_t datalen, uint8_t *dataptr);

	    uint32_t recvData(uint32_t *bytes_rcvd, uint8_t *dataptr);
	    uint32_t recvData(uint8_t *outBuf);

		/* USBDeviceConfig implementation */
		virtual uint32_t Init(uint32_t parent, uint32_t port, uint32_t lowspeed);
		virtual uint32_t Release();
		virtual uint32_t Poll() { return 0; };	// not implemented
		virtual uint32_t GetAddress() { return bAddress; };
		virtual bool isReady() { return ready; };
		virtual void task() { UsbBasedDevice::task(); };
};

class nw2s::USBMidiController : public USBMidiDevice
{
	public: 
		
		virtual void task();
		
	protected:
		
		virtual void onNoteOn(uint32_t channel, uint32_t note, uint32_t velocity) {};
		virtual void onNoteOff(uint32_t channel, uint32_t note, uint32_t velocity) {};
		virtual void onPressure(uint32_t channel, uint32_t note, uint32_t pressure) {};
		virtual void onControlChange(uint32_t channel, uint32_t controller, uint32_t value) {};
		virtual void onProgramChange(uint32_t channel, uint32_t program) {};
		virtual void onAftertouch(uint32_t channel, uint32_t value) {};
		virtual void onPitchbend(uint32_t channel, uint32_t value) {};		
		
		USBMidiController();

	private: 
	
		void processMessage(uint32_t size, uint8_t *dataptr);
		
};

class nw2s::USBMidiCCController : public nw2s::USBMidiController
{
	public:

		static void create();
		void addControlPin(uint32_t controller, PinAnalogOut output);

	protected:
		
		USBMidiCCController();
		virtual void onControlChange(uint32_t channel, uint32_t controller, uint32_t value);

	private:

		std::vector<ControlOutput> outputs;
};

class nw2s::USBMonophonicMidiController : public nw2s::USBMidiCCController, public nw2s::TimeBasedDevice
{
	
	public: 
	
		static USBMonophonicMidiController* create(PinDigitalOut gatePin, PinDigitalOut triggerOn, PinDigitalOut triggerOff, PinAnalogOut pitchPin, PinAnalogOut velocityPin, PinAnalogOut pressurePin);

		void timer(uint32_t t);
			
		//TODO: monophonic note stack
		//TODO: limit to key
		
	protected:
		
		virtual void onNoteOn(uint32_t channel, uint32_t note, uint32_t velocity);
		virtual void onNoteOff(uint32_t channel, uint32_t note, uint32_t velocity);
		virtual void onPressure(uint32_t channel, uint32_t note, uint32_t pressure);
		virtual void onAftertouch(uint32_t channel, uint32_t value);
		virtual void onPitchbend(uint32_t channel, uint32_t value);		
		
	private:
		
		PinDigitalOut gate;
		Gate* triggerOn = NULL;
		Gate* triggerOff = NULL;
		AnalogOut* pitch;
		AnalogOut* velocity;
		AnalogOut* pressure;
		
		USBMonophonicMidiController(PinDigitalOut gatePin, PinDigitalOut triggerOn, PinDigitalOut triggerOff, PinAnalogOut pitchPin, PinAnalogOut velocityPin, PinAnalogOut pressureOut);
};

#endif



