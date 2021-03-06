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

#ifndef EventManager_h
#define EventManager_h

#include <iterator>
#include <vector>
#include <usbhost/Usb.h>

using namespace std;

namespace nw2s
{		
	class EventManager;
	class TimeBasedDevice;
	class UsbBasedDevice;
}

class nw2s::TimeBasedDevice
{
	public:
		virtual void timer(unsigned long t) = 0;
};

class nw2s::UsbBasedDevice
{
	protected:
		USBHost	*pUsb;
		
	public:
		virtual void task();
};

class nw2s::EventManager
{
	public:
		static void initialize();
 		static void registerDevice(TimeBasedDevice* device);
		static void registerUsbDevice(UsbBasedDevice* usbDevice);
		static void loop();
		static unsigned long getT();
		static USBHost usbHost;
	
	private:
		static volatile unsigned long t;
		static vector<TimeBasedDevice*> timedevices;		

		//TODO: This is too specific - need to encapsulate only USB device
		static UsbBasedDevice* usbDevice;
};


#endif


