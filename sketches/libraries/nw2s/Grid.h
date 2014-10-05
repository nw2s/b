

#ifndef GRID_H
#define GRID_H

#include "usb_ch9.h"
#include "Usb.h"
#include "confdescparser.h"

#define MAX_ENDPOINTS 3

class USBGrid : public USBDeviceConfig, public UsbConfigXtracter
{
	private:
		
		/* ID strings */
		const char* manufacturer;
		const char* model;
		const char* description;
		const char* version;
		const char* uri;
		const char* serial;

	protected:

		static const uint32_t epDataInIndex;			// DataIn endpoint index
		static const uint32_t epDataOutIndex;			// DataOUT endpoint index

		/* Mandatory members */
		USBHost		*pUsb;
		uint32_t	bAddress;							// Device USB address
		uint32_t	bConfNum;							// configuration number

		uint32_t	bNumEP;								// total number of EP in the configuration
		bool		ready;

		/* Endpoint data structure describing the device EP */
		EpInfo		epInfo[MAX_ENDPOINTS];

	public:
		
		USBGrid(USBHost *pUsb);

		// Methods for receiving and sending data
		//TODO: Once it's working, we should abstract the data layer?
		uint32_t read(uint32_t *nreadbytes, uint32_t datalen, uint8_t *dataptr);
		uint32_t write(uint32_t datalen, uint8_t *dataptr);


		// USBDeviceConfig implementation
		virtual uint32_t Init(uint32_t parent, uint32_t port, uint32_t lowspeed);
		virtual uint32_t Release();
		virtual uint32_t Poll() { return 0; };	// not implemented
		virtual uint32_t GetAddress() { return bAddress; };
		virtual bool isReady() { return ready; };

		// UsbConfigXtracter implementation
		virtual void EndpointXtract(uint32_t conf, uint32_t iface, uint32_t alt, uint32_t proto, const USB_ENDPOINT_DESCRIPTOR *ep);
};

#endif


