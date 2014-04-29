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

#include <Key.h>
#include <EventManager.h>
#include <Trigger.h>
#include <Clock.h>
#include <Slew.h>
#include <Sequence.h>
#include <IO.h>
#include <SPI.h>
#include <SD.h>
#include <Wire.h>


using namespace nw2s;

/*

	This sketch will test access to the SD reader It will display the status of the SD card
	and then continue to render the contents of /test/test-data.txt. By default this is the 
	contents of Alice's Adventures in Wonderland.

	Note that most of these test sketches are not meant to be examples of how best to perform
	any particular task, but rather they are meant to test raw functionality. Please don't 
	use these as sample of Good Things.
	
*/


/* Set up the variables */
Sd2Card card;
SdVolume volume;
SdFile root;
File dataFile;

void setup() 
{
	Serial.begin(19200);
	Serial.print("\n\nStarting...\n");

	EventManager::initialize();
	
	Serial.print("\nInitializing SD card...");
	
	if (!card.init(SPI_HALF_SPEED, SD_CS)) 
	{
	    Serial.println("initialization failed. Things to check:");
	    Serial.println("* is a card is inserted?");
	    Serial.println("* Is your wiring correct?");
	    Serial.println("* did you change the chipSelect pin to match your shield or module?");
	    return;
	} 
	else 
	{
		Serial.println("Wiring is correct and a card is present."); 
	}
	
	Serial.print("\nCard type: ");
	
	switch(card.type()) 
	{
	    case SD_CARD_TYPE_SD1:
	    	Serial.println("SD1");
	      	break;
	    case SD_CARD_TYPE_SD2:
	      	Serial.println("SD2");
	      	break;
	    case SD_CARD_TYPE_SDHC:
	      	Serial.println("SDHC");
	      	break;
	    default:
	      Serial.println("Unknown");
	}

	if (!volume.init(card)) 
	{
		Serial.println("Could not find FAT16/FAT32 partition.\nMake sure you've formatted the card");
	    return;
	}

    uint32_t volumesize;
    Serial.print("\nVolume type is FAT");
    Serial.println(volume.fatType(), DEC);
    Serial.println();
  
    volumesize = volume.blocksPerCluster();    // clusters are collections of blocks
    volumesize *= volume.clusterCount();       // we'll have a lot of clusters
    volumesize *= 512;                            // SD card blocks are always 512 bytes
    Serial.print("Volume size (bytes): ");
    Serial.println(volumesize);
    Serial.print("Volume size (Kbytes): ");
    volumesize /= 1024;
    Serial.println(volumesize);
    Serial.print("Volume size (Mbytes): ");
    volumesize /= 1024;
    Serial.println(volumesize);

  
    Serial.println("\nFiles found on the card (name, date and size in bytes): ");
    root.openRoot(volume);
  
    // list all files in the card with date and size
    root.ls(LS_R | LS_DATE | LS_SIZE);

	dataFile = SD.open("test/testdata.txt");
	
	// if the file isn't open, pop up an error:
	if (!dataFile)
	{
	    Serial.println("error opening test/testdata.txt");
	}	
}

void loop() 
{
	if (dataFile) 
	{
	    while (dataFile.available()) 
		{
			char c = dataFile.read();
	    	Serial.print(c);
			
			if (c == '\n')
			{
				delay(10);
				break;
			}
	    }
	}  
		
	
}
