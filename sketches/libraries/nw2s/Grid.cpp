


#include "Grid.h"


/* These are all identification attribtues. I'm not sure if we need them or not */
char applicationName[] = "grid"; 
char accessoryName[] = "nw2s-b"; 
char companyName[] = "nw2s";
char versionNumber[] = "1.0";
char serialNumber[] = "1";
char url[] = "http://nw2s.net";



const uint32_t USBGrid::epDataInIndex  = 1;
const uint32_t USBGrid::epDataOutIndex = 2;

USBGrid::USBGrid(USBHost *p) :
		manufacturer(companyName),
		model(accessoryName),
		description(accessoryName),
		version(versionNumber),
		uri(url),
		serial(serialNumber),
		pUsb(p),
		bAddress(0),
		bNumEP(1),
		ready(false)
{
	
	for (uint32_t i = 0; i < MAX_ENDPOINTS; ++i)
	{
		epInfo[i].deviceEpNum	= 0;
		epInfo[i].hostPipeNum	= 0;
		epInfo[i].maxPktSize	= (i) ? 0 : 8;
		epInfo[i].epAttribs		= 0;
		epInfo[i].bmNakPower  	= (i) ? USB_NAK_NOWAIT : USB_NAK_MAX_POWER;
	}

	// Register in USB subsystem
	if (pUsb)
	{
		pUsb->RegisterDeviceClass(this);
	}
}


uint32_t USBGrid::Init(uint32_t parent, uint32_t port, uint32_t lowspeed)
{
	uint8_t		buf[sizeof(USB_DEVICE_DESCRIPTOR)];
	uint32_t	rcode = 0;
	UsbDevice	*p = NULL;
	EpInfo		*oldep_ptr = NULL;
	uint32_t 	adkproto = -1;
	uint32_t	num_of_conf = 0;

	/* Get memory address of USB device address pool */
	AddressPool	&addrPool = pUsb->GetAddressPool();

	Serial.println("USBGrid::Init");

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

	Serial.print("Max packet size: ");
	Serial.println(epInfo[0].maxPktSize);

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

	Serial.print("Found a number of configurarions: ");
	Serial.println(num_of_conf);

	for (uint32_t i = 0; i < num_of_conf; ++i)
	{
		ConfigDescParser<0, 0, 0, 0> confDescrParser(this);

		delay(1);
		rcode = pUsb->getConfDescr(bAddress, 0, i, &confDescrParser);

		if (rcode)
		{
			Serial.print("Failed to get configurarion descriptors: ");
			Serial.println(rcode);
			return rcode;
		}

		if (bNumEP > 2)
		{
			break;
		}
	}

	if (bNumEP == 3)
	{
		/* Assign epInfo to epinfo pointer - this time all 3 endpoins */
		rcode = pUsb->setEpInfoEntry(bAddress, 3, epInfo);
		if (rcode)
		{
			Serial.print("Failed setEpInfoEntry: ");
			Serial.println(rcode);
		}
	}

	/* Set Configuration Value */
	rcode = pUsb->setConf(bAddress, 0, bConfNum);
	if (rcode)
	{
		Serial.print("setConf failed: ");
		Serial.println(rcode);
		return rcode;
	}

	/* Assume for now that the control interface is 0 */
	bControlIface = 0;

	/* Send the line control information */
    rcode = SetControlLineState(3);

    if (rcode)
    {
		Serial.print("SetControlLineState failed: ");
		Serial.println(rcode);
        return rcode;
    }
	
    LINE_CODING	lc;
    lc.dwDTERate	= 115200;
    lc.bCharFormat	= 0;
    lc.bParityType	= 0;
    lc.bDataBits	= 8;

    rcode = SetLineCoding(&lc);

    if (rcode)
	{
		Serial.print("SetLineCoding failed: ");
		Serial.println(rcode);
		return rcode;
	}
	
	Serial.println("Device configured successfully");
	ready = true;

	return 0;
}



void USBGrid::EndpointXtract(uint32_t conf, uint32_t iface, uint32_t alt, uint32_t proto, const USB_ENDPOINT_DESCRIPTOR *pep)
{	
	Serial.println("");
	Serial.println("Xtracting: ");
	Serial.print("     address: ");
	Serial.println(pep->bEndpointAddress, HEX);
	Serial.print("   attribute: ");
	Serial.println(pep->bmAttributes, HEX);
	Serial.println("");
	
	if (pep->bmAttributes != 2) return;
	
	if (bNumEP == MAX_ENDPOINTS)
	{
		return;
	}

	bConfNum = conf;

	uint32_t index = 0;
	uint32_t pipe = 0;

	if ((pep->bmAttributes & 0x02) == 2)
	{
		index = ((pep->bEndpointAddress & 0x80) == 0x80) ? epDataInIndex : epDataOutIndex;
	}

	/* Fill in the endpoint info structure */
	epInfo[index].deviceEpNum = pep->bEndpointAddress & 0x0F;
	epInfo[index].maxPktSize = pep->wMaxPacketSize;


	Serial.println("Found new endpoint");
	Serial.print(" deviceEpNum: ");
	Serial.println(epInfo[index].deviceEpNum);
	Serial.print(" maxPktSize: "); 
	Serial.println(epInfo[index].maxPktSize);
	Serial.print(" index: "); 
	Serial.println(index);


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


uint32_t USBGrid::Release()
{
	UHD_Pipe_Free(epInfo[epDataInIndex].hostPipeNum);
	UHD_Pipe_Free(epInfo[epDataOutIndex].hostPipeNum);

	// Free allocated USB address
	pUsb->GetAddressPool().FreeAddress(bAddress);

	// Must have to be reset to 1
	bNumEP = 1;

	bAddress = 0;
	ready = false;

	return 0;
}

uint32_t USBGrid::read(uint32_t *nreadbytes, uint32_t datalen, uint8_t *dataptr)
{
	*nreadbytes = datalen;
	return pUsb->inTransfer(bAddress, epInfo[epDataInIndex].deviceEpNum, nreadbytes, dataptr);
}

uint32_t USBGrid::write(uint32_t datalen, uint8_t *dataptr)
{
	return pUsb->outTransfer(bAddress, epInfo[epDataOutIndex].deviceEpNum, datalen, dataptr);
}

uint8_t USBGrid::SetControlLineState(uint8_t state) 
{
	return ( pUsb->ctrlReq(bAddress, 0, bmREQ_CDCOUT, CDC_SET_CONTROL_LINE_STATE, state, 0, bControlIface, 0, 0, NULL, NULL));
}

uint8_t USBGrid::SetLineCoding(const LINE_CODING *dataptr) 
{
	return ( pUsb->ctrlReq(bAddress, 0, bmREQ_CDCOUT, CDC_SET_LINE_CODING, 0x00, 0x00, bControlIface, sizeof (LINE_CODING), sizeof (LINE_CODING), (uint8_t*)dataptr, NULL));
}


