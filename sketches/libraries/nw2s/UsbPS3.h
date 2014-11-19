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

#ifndef USBPS3_H
#define USBPS3_H

//include "usb_ch9.h"
#include "Usb.h"
#include "EventManager.h"
#include "confdescparser.h"
#include "../aJson/aJson.h"

//define MAX_ENDPOINTS 3
#define EP_MAXPKTSIZE           64 // max size for data via USB
#define PS3_REPORT_BUFFER_SIZE  48

/* Names we give to the 3 ps3 pipes - this is only used for setting the bluetooth address into the ps3 controllers */
#define PS3_CONTROL_PIPE        0
#define PS3_OUTPUT_PIPE         1
#define PS3_INPUT_PIPE          2

//PID and VID of the different devices
#define PS3_VID                 0x054C  // Sony Corporation
#define PS3_PID                 0x0268  // PS3 Controller DualShock 3
#define PS3NAVIGATION_PID       0x042F  // Navigation controller
#define PS3MOVE_PID             0x03D5  // Motion controller

// Used in control endpoint header for HID Commands
#define bmREQ_HID_OUT           USB_SETUP_HOST_TO_DEVICE|USB_SETUP_TYPE_CLASS|USB_SETUP_RECIPIENT_INTERFACE
#define bmREQ_HID_IN            USB_SETUP_DEVICE_TO_HOST|USB_SETUP_TYPE_CLASS|USB_SETUP_RECIPIENT_INTERFACE

#define HID_REQUEST_GET_REPORT  0x01
#define HID_REQUEST_SET_REPORT  0x09

#define PS3_MAX_ENDPOINTS       3


namespace nw2s
{
	
	class UsbPS3;
	class UsbPS3CV;

	const uint8_t PS3_REPORT_BUFFER[PS3_REPORT_BUFFER_SIZE] = {
		
	        0x00, 0x00, 0x00, 0x00, 0x00,
	        0x00, 0x00, 0x00, 0x00, 0x00,
	        0xff, 0x27, 0x10, 0x00, 0x32,
	        0xff, 0x27, 0x10, 0x00, 0x32,
	        0xff, 0x27, 0x10, 0x00, 0x32,
	        0xff, 0x27, 0x10, 0x00, 0x32,
	        0x00, 0x00, 0x00, 0x00, 0x00,
	        0x00, 0x00, 0x00, 0x00, 0x00,
	        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
	};

	const uint8_t PS3_LEDS[] = {
	
	        0x00, // OFF
	        0x01, // LED1
	        0x02, // LED2
	        0x04, // LED3
	        0x08, // LED4

	        0x09, // LED5
	        0x0A, // LED6
	        0x0C, // LED7
	        0x0D, // LED8
	        0x0E, // LED9
	        0x0F, // LED10
	};


	const uint32_t PS3_BUTTONS[] = {
	
	        0x10, // UP
	        0x20, // RIGHT
	        0x40, // DOWN
	        0x80, // LEFT

	        0x01, // SELECT
	        0x08, // START
	        0x02, // L3
	        0x04, // R3

	        0x0100, // L2
	        0x0200, // R2
	        0x0400, // L1
	        0x0800, // R1

	        0x1000, // TRIANGLE
	        0x2000, // CIRCLE
	        0x4000, // CROSS
	        0x8000, // SQUARE

	        0x010000, // PS
	        0x080000, // MOVE - covers 12 bits - we only need to read the top 8
	        0x100000, // T - covers 12 bits - we only need to read the top 8
	};

	const uint8_t PS3_ANALOG_BUTTONS[] = 
	{
	        23, // UP_ANALOG
	        24, // RIGHT_ANALOG
	        25, // DOWN_ANALOG
	        26, // LEFT_ANALOG
	        0, 0, 0, 0, // Skip SELECT, L3, R3 and START

	        27, // L2_ANALOG
	        28, // R2_ANALOG
	        29, // L1_ANALOG
	        30, // R1_ANALOG
	        31, // TRIANGLE_ANALOG
	        32, // CIRCLE_ANALOG
	        33, // CROSS_ANALOG
	        34, // SQUARE_ANALOG
	        0, 0, // Skip PS and MOVE

	        // Playstation Move Controller
	        15, // T_ANALOG - Both at byte 14 (last reading) and byte 15 (current reading)
	};

	enum AngleEnum 
	{
	        Pitch = 0x01,
	        Roll = 0x02,
	};

	enum RumbleEnum 
	{
	        RumbleHigh = 0x10,
	        RumbleLow = 0x20,
	};

	enum SensorEnum 
	{
	        /** Accelerometer values */
	        aX = 50, aY = 52, aZ = 54,
	        /** Gyro z-axis */
	        gZ = 56,
	        gX, gY, // These are not available on the PS3 controller

	        /** Accelerometer x-axis */
	        aXmove = 28,
	        /** Accelerometer z-axis */
	        aZmove = 30,
	        /** Accelerometer y-axis */
	        aYmove = 32,

	        /** Gyro x-axis */
	        gXmove = 40,
	        /** Gyro z-axis */
	        gZmove = 42,
	        /** Gyro y-axis */
	        gYmove = 44,

	        /** Temperature sensor */
	        tempMove = 46,

	        /** Magnetometer x-axis */
	        mXmove = 47,
	        /** Magnetometer z-axis */
	        mZmove = 49,
	        /** Magnetometer y-axis */
	        mYmove = 50,
	};

	enum LEDEnum 
	{
	        OFF = 0,
	        LED1 = 1,
	        LED2 = 2,
	        LED3 = 3,
	        LED4 = 4,

	        LED5 = 5,
	        LED6 = 6,
	        LED7 = 7,
	        LED8 = 8,
	        LED9 = 9,
	        LED10 = 10,
	        /** Used to blink all LEDs on the Xbox controller */
	        ALL = 5,
	};

	enum StatusEnum 
	{
	        // Note that the location is shifted 9 when it's connected via USB
	        // Byte location | bit location
	        Plugged = (38 << 8) | 0x02,
	        Unplugged = (38 << 8) | 0x03,

	        Charging = (39 << 8) | 0xEE,
	        NotCharging = (39 << 8) | 0xF1,
	        Shutdown = (39 << 8) | 0x01,
	        Dying = (39 << 8) | 0x02,
	        Low = (39 << 8) | 0x03,
	        High = (39 << 8) | 0x04,
	        Full = (39 << 8) | 0x05,

	        MoveCharging = (21 << 8) | 0xEE,
	        MoveNotCharging = (21 << 8) | 0xF1,
	        MoveShutdown = (21 << 8) | 0x01,
	        MoveDying = (21 << 8) | 0x02,
	        MoveLow = (21 << 8) | 0x03,
	        MoveHigh = (21 << 8) | 0x04,
	        MoveFull = (21 << 8) | 0x05,

	        CableRumble = (40 << 8) | 0x10, // Operating by USB and rumble is turned on
	        Cable = (40 << 8) | 0x12, // Operating by USB and rumble is turned off
	        BluetoothRumble = (40 << 8) | 0x14, // Operating by Bluetooth and rumble is turned on
	        Bluetooth = (40 << 8) | 0x16, // Operating by Bluetooth and rumble is turned off
	};
	
	enum AnalogHatEnum 
	{
	        /** Left joystick x-axis */
	        LeftHatX = 0,
	        /** Left joystick y-axis */
	        LeftHatY = 1,
	        /** Right joystick x-axis */
	        RightHatX = 2,
	        /** Right joystick y-axis */
	        RightHatY = 3,
	};
	
	enum ButtonEnum 
	{
	        /** These buttons are available on all the the controllers */
	        UP = 0,
	        RIGHT = 1,
	        DOWN = 2,
	        LEFT = 3,

	        /** PS3 controllers buttons */
	        SELECT = 4,
	        START = 5,
	        L3 = 6,
	        R3 = 7,

	        L2 = 8,
	        R2 = 9,
	        L1 = 10,
	        R1 = 11,
	        TRIANGLE = 12,
	        CIRCLE = 13,
	        CROSS = 14,
	        SQUARE = 15,

	        PS = 16,

	        MOVE = 17, // Covers 12 bits - we only need to read the top 8
	        T = 18, // Covers 12 bits - we only need to read the top 8

	        /** PS4 controllers buttons - SHARE and OPTIONS are present instead of SELECT and START */
	        SHARE = 4,
	        OPTIONS = 5,
	        TOUCHPAD = 17
	};	
	
}

class nw2s::UsbPS3 : public USBDeviceConfig, public UsbBasedDevice
{
	protected:

		static const uint32_t epDataInIndex;			// DataIn endpoint index
		static const uint32_t epDataOutIndex;			// DataOUT endpoint index

		/* Mandatory members */
		uint32_t	bAddress;							// Device USB address
		uint32_t	bConfNum;							// configuration number
		uint8_t 	bControlIface; 						// Control interface value
		uint32_t	bNumEP;								// total number of EP in the configuration
		bool		ready;

		/* Endpoint data structure describing the device EP */
        EpInfo 		epInfo[PS3_MAX_ENDPOINTS];

	public:
		
		UsbPS3();

        bool PS3Connected;
        bool PS3MoveConnected;
        bool PS3NavigationConnected;
		
		/* USBDeviceConfig implementation */
		virtual uint32_t Init(uint32_t parent, uint32_t port, uint32_t lowspeed);
		virtual uint32_t Release();
		virtual uint32_t Poll();	
		virtual uint32_t GetAddress() { return bAddress; };
		virtual bool isReady() { return ready; };

		/* UsbConfigXtracter implementation */
		//virtual void EndpointXtract(uint32_t conf, uint32_t iface, uint32_t alt, uint32_t proto, const USB_ENDPOINT_DESCRIPTOR *ep);

        // virtual uint8_t Release();
        // virtual uint8_t Poll();
        // virtual bool isReady() {
        //         return bPollEnable;
        // };

        virtual boolean VIDPIDOK(uint16_t vid, uint16_t pid) {
                return (vid == PS3_VID && (pid == PS3_PID || pid == PS3NAVIGATION_PID || pid == PS3MOVE_PID));
        };
		
        bool getButtonPress(ButtonEnum b);
        bool getButtonClick(ButtonEnum b);
        uint8_t getAnalogButton(ButtonEnum a);
        uint8_t getAnalogHat(AnalogHatEnum a);
        uint16_t getSensor(SensorEnum a);
        double getAngle(AngleEnum a);
        bool getStatus(StatusEnum c);
        void setAllOff();
        void setRumbleOff();
        void setRumbleOn(RumbleEnum mode);
        void setRumbleOn(uint8_t rightDuration, uint8_t rightPower, uint8_t leftDuration, uint8_t leftPower);
        void setLedRaw(uint8_t value);
        void setLedOff() {
                setLedRaw(0);
        }
        void setLedOff(LEDEnum a);
        void setLedOn(LEDEnum a);
        void setLedToggle(LEDEnum a);
		
	private:
		
        bool bPollEnable;

		uint32_t timer;
		uint32_t ButtonState;
        uint32_t OldButtonState;
        uint32_t ButtonClickState;
		
        uint8_t readBuf[EP_MAXPKTSIZE]; 
        uint8_t writeBuf[EP_MAXPKTSIZE]; 		
		
        void readReport(); 
		
        /* Private commands */
        void PS3_Command(uint8_t *data, uint16_t nbytes);
        void enable_sixaxis();
		 
};

class nw2s::UsbPS3CV : public nw2s::UsbPS3
{
	/* 
	
		Triggers:
	
		1 U
		2 D 
		3 L
		4 R
		5 SELECT
		6 START
		7 L3
		8 R3
		9 L2
		10 R2
		11 L1
		12 R1
		13 TRIANGLE
		14 CIRCLE
		15 CROSS
		16 SQUARE
		
		CV:
	
		1 L2
		2 R2
		3 L1
		4 R1
		5 TRIANGLE
		6 CIRCLE
		7 CROSS
		8 SQUARE
		9 PITCH
		10 ROLL
		11 LEFT HAT X
		12 LEFT HAT Y
		13 RIGHT HAT X
		14 RIGHT HAT Y
	
		15 ACC Y
		16 ACC Z
	
		(TODO: Ignore front to back accelleration)
	
	*/
};

#endif


