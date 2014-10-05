#include <Usb.h>

USBHost usb;
boolean found = false;

void setup() { 

  byte rcode;
  Serial.begin(115200);

  while(!found) //Get string descriptors
  {
    usb.Task();
    usb.ForEachUsbDevice(GetStringDescriptors);
  }
  //get device descriptors
  if( usb.getUsbTaskState() == USB_STATE_RUNNING ) {  //state configuring or higher
    rcode = getdevdescr(1);   //hardcoded device address
    if( rcode ) {
      Serial.print("\r\nRequest error. Error code:\t");
      Serial.println( rcode, HEX );
    }
  }
  // get configuration descriptors
  if( usb.getUsbTaskState() == USB_STATE_RUNNING ) {  //state configuring or higher
    rcode = getconfdescr( 1, 0 );   //get configuration descriptor
    if( rcode ) {
      Serial.println( rcode, HEX );
    }
  }
}


void loop() {

}

// function to get all string descriptors
void GetStringDescriptors(UsbDevice *pdev)
{ 
  byte rcode;
  usb.Task();
  if( usb.getUsbTaskState() >= 0x80 ) {  // state configuring or higher
    found = true;
    USB_DEVICE_DESCRIPTOR buf; 
    rcode = usb.getDevDescr( 1, 0, 0x12, ( byte *)&buf );
    Serial.println("String Descriptors:");
    if( buf.iManufacturer > 0 ) {
      Serial.print("Manufacturer:\t\t");
      rcode = getstrdescr( 1, buf.iManufacturer );   // get manufacturer string
      if( rcode ) {
        Serial.println( rcode, HEX );
      }
    }
    if( buf.iProduct > 0 ) {
      Serial.print("Product:\t\t");
      rcode = getstrdescr( 1, buf.iProduct );        // get product string
      if( rcode ) {
        Serial.println( rcode, HEX );
      }
    }
    if( buf.iSerialNumber > 0 ) {
      Serial.print("Serial:\t\t\t");
      rcode = getstrdescr( 1, buf.iSerialNumber );   // get serial string
      if( rcode ) {
        Serial.println( rcode, HEX );
      }
    }
  }
}

//  function to get single string description
unsigned int getstrdescr( unsigned int addr, byte idx )
{
  byte buf[ 66 ];
  unsigned int rcode;
  byte length;
  byte i;
  unsigned short langid;
  rcode = usb.getStrDescr( addr, 0, 1, 0, 0, buf );  //get language table length
  if( rcode ) {
    Serial.println("Error retrieving LangID table length");
    return( rcode );
  }
  length = buf[ 0 ];      //length is the first byte
  rcode = usb.getStrDescr( addr, 0, length, 0, 0, buf );  //get language table
  if( rcode ) {
    Serial.print("Error retrieving LangID table ");
    return( rcode );
  }
  langid = word(buf[3], buf[2]); 
  rcode = usb.getStrDescr( addr, 0, 1, idx, langid, buf );
  if( rcode ) {
    Serial.print("Error retrieving string length ");
    return( rcode );
  }
  length = buf[ 0 ];
  rcode = usb.getStrDescr( addr, 0, length, idx, langid, buf );
  if( rcode ) {
    Serial.print("Error retrieving string ");
    return( rcode );
  }
  for( i = 2; i < length; i+=2 ) {
    Serial.print((char) buf[i]);
  }
  Serial.println();
  return( rcode );
}

// function to get device descriptor
byte getdevdescr( unsigned int addr )
{
  USB_DEVICE_DESCRIPTOR buf;
  byte rcode;
  rcode = usb.getDevDescr( addr, 0, 0x12, ( byte *)&buf );
  if( rcode ) {
    return( rcode );
  }
  Serial.println("\r\nDevice Descriptor:");
  Serial.print("Descriptor Length:\t");
  Serial.print( buf.bLength, 8 );
  Serial.print("\r\nDescriptor type:\t");
  Serial.print( buf.bDescriptorType, 8 );
  Serial.print("\r\nUSB version:\t\t");
  Serial.print( buf.bcdUSB, 16 );
  Serial.print("\r\nDevice class:\t\t");
  Serial.print( buf.bDeviceClass, 8 );
  Serial.print("\r\nDevice Subclass:\t");
  Serial.print( buf.bDeviceSubClass, 8 );
  Serial.print("\r\nDevice Protocol:\t");
  Serial.print( buf.bDeviceProtocol, 8 );
  Serial.print("\r\nMax.packet size:\t");
  Serial.print( buf.bMaxPacketSize0, 8 );
  Serial.print("\r\nVendor  ID:\t\t");
  Serial.print( buf.idVendor, 16 );
  Serial.print("\r\nProduct ID:\t\t");
  Serial.print( buf.idProduct, 16 );
  Serial.print("\r\nRevision ID:\t\t");
  Serial.print( buf.bcdDevice, 16 );
  Serial.print("\r\nMfg.string index:\t");
  Serial.print( buf.iManufacturer, 8 );
  Serial.print("\r\nProd.string index:\t");
  Serial.print( buf.iProduct, 8 );
  Serial.print("\r\nSerial number index:\t");
  Serial.print( buf.iSerialNumber, 8 );
  Serial.print("\r\nNumber of conf.:\t");
  Serial.println( buf.bNumConfigurations, 8 );
  return( 0 );
}

// function to get configuration descriptor
byte getconfdescr( byte addr, byte conf )
{
  byte buf[256];
  byte* buf_ptr = buf;
  unsigned int rcode;
  byte descr_length;
  byte descr_type;
  unsigned int total_length;
  rcode = usb.getConfDescr( addr, 0, 4, conf, buf );  //get total length

  total_length = word(buf[3], buf[2]);

  if( total_length > 256 ) {    //check if total length is larger than buffer
    Serial.println("Configuration buffer was truncated to 256 bytes");
    total_length = 256;
  }
  rcode = usb.getConfDescr( addr, 0, total_length, conf, buf ); //get the whole descriptor
  while( buf_ptr < buf + total_length ) {  //parsing descriptors
    descr_length = *( buf_ptr );
    descr_type = *( buf_ptr + 1 );
    switch( descr_type ) {
      case( USB_DESCRIPTOR_CONFIGURATION ):
      printconfdescr( buf_ptr );
      break;
      case( USB_DESCRIPTOR_INTERFACE ):
      printintfdescr( buf_ptr );
      break;
      case( USB_DESCRIPTOR_ENDPOINT ):
      printepdescr( buf_ptr );
      break;
    default:
      break;
    }//switch( descr_type
    buf_ptr = ( buf_ptr + descr_length );    //advance buffer pointer
  }//while( buf_ptr <=...
  return( 0 );
}

/* function to print configuration descriptor */
void printconfdescr( byte* descr_ptr )
{
  USB_CONFIGURATION_DESCRIPTOR* conf_ptr = ( USB_CONFIGURATION_DESCRIPTOR* )descr_ptr;
  Serial.println("\r\nConfiguration Descriptor:");
  Serial.print("Length:\t\t\t");
  Serial.print( conf_ptr->bLength, 16);
  Serial.print("\r\nDescriptor Type:\t");
  Serial.print( conf_ptr->bDescriptorType, 16);
  Serial.print("\r\nTotal length:\t\t");
  Serial.print( conf_ptr->wTotalLength, 16 );
  Serial.print("\r\nNum.intf:\t\t");
  Serial.print( conf_ptr->bNumInterfaces, 8 );
  Serial.print("\r\nConf.value:\t\t");
  Serial.print( conf_ptr->bConfigurationValue, 8 );
  Serial.print("\r\nConf.string:\t\t");
  Serial.print( conf_ptr->iConfiguration, 8 );
  Serial.print("\r\nAttr.:\t\t\t");
  Serial.print( conf_ptr->bmAttributes, 8 );
  Serial.print("\r\nMax.pwr:\t\t");
  Serial.print( conf_ptr->bMaxPower*2, 10 );
  Serial.print( " mA" );
  return;
}
/* function to print interface descriptor */
void printintfdescr( byte* descr_ptr )
{
  USB_INTERFACE_DESCRIPTOR* intf_ptr = ( USB_INTERFACE_DESCRIPTOR* )descr_ptr;
  Serial.println("\r\n\r\nInterface Descriptor:");
  Serial.print("Intf.number:\t\t");
  Serial.print( intf_ptr->bInterfaceNumber, 8 );
  Serial.print("\r\nAlt.:\t\t\t");
  Serial.print( intf_ptr->bAlternateSetting, 8 );
  Serial.print("\r\nEndpoints:\t\t");
  Serial.print( intf_ptr->bNumEndpoints, 8 );
  Serial.print("\r\nClass:\t\t\t");
  Serial.print( intf_ptr->bInterfaceClass, 8 );
  Serial.print("\r\nSubclass:\t\t");
  Serial.print( intf_ptr->bInterfaceSubClass, 8 );
  Serial.print("\r\nProtocol:\t\t");
  Serial.print( intf_ptr->bInterfaceProtocol, 8 );
  Serial.print("\r\nIntf.string:\t\t");
  Serial.println( intf_ptr->iInterface, 8 );
  return;
}
/* function to print endpoint descriptor */
void printepdescr( byte* descr_ptr )
{
  USB_ENDPOINT_DESCRIPTOR* ep_ptr = ( USB_ENDPOINT_DESCRIPTOR* )descr_ptr;
  Serial.println("\r\nEndpoint Descriptor:");
  Serial.print("Endpoint address:\t");
  Serial.print( ep_ptr->bEndpointAddress, 8 );
  Serial.print("\r\nAttr.:\t\t\t");
  Serial.print( ep_ptr->bmAttributes, 8 );
  Serial.print("\r\nMax.pkt size:\t\t");
  Serial.print( ep_ptr->wMaxPacketSize, 16 );
  Serial.print("\r\nPolling interval:\t");
  Serial.println( ep_ptr->bInterval, 8 );
  return;
}