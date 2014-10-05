

#ifndef GRID_H
#define GRID_H

#include "usb_ch9.h"
#include "Usb.h"
#include "confdescparser.h"

#define MAX_ENDPOINTS 3

typedef struct 
{
        uint32_t dwDTERate; // Data Terminal Rate in bits per second
        uint8_t bCharFormat; // 0 - 1 stop bit, 1 - 1.5 stop bits, 2 - 2 stop bits
        uint8_t bParityType; // 0 - None, 1 - Odd, 2 - Even, 3 - Mark, 4 - Space
        uint8_t bDataBits; // Data bits (5, 6, 7, 8 or 16)
} 
LINE_CODING;


class USBGrid128 : public USBDeviceConfig, public UsbConfigXtracter
{
	private:
		
		/* ID strings */
		// const char* manufacturer;
		// const char* model;
		// const char* description;
		// const char* version;
		// const char* uri;
		// const char* serial;

	protected:

		static const uint32_t epDataInIndex;			// DataIn endpoint index
		static const uint32_t epDataOutIndex;			// DataOUT endpoint index

		/* Mandatory members */
		USBHost		*pUsb;
		uint32_t	bAddress;							// Device USB address
		uint32_t	bConfNum;							// configuration number
		uint8_t 	bControlIface; 						// Control interface value
		uint32_t	bNumEP;								// total number of EP in the configuration
		bool		ready;

		/* Endpoint data structure describing the device EP */
		EpInfo		epInfo[MAX_ENDPOINTS];

	public:
		
		USBGrid128(USBHost *pUsb);

		// Methods for receiving and sending data
		//TODO: Once it's working, we should abstract the data layer?
		uint32_t read(uint32_t *nreadbytes, uint32_t datalen, uint8_t *dataptr);
		uint32_t write(uint32_t datalen, uint8_t *dataptr);

        uint8_t SetLineCoding(const LINE_CODING *dataptr);
        uint8_t SetControlLineState(uint8_t state);

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


