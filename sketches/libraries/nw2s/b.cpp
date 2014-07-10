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

#include <SD.h>
#include "IO.h"
#include "b.h"

using namespace nw2s;

/* GLOBAL SETTINGS */
bool b::debugMode = false;
bool b::cvGainMode = CV_GAIN_LOW;
bool b::rootInitialized = false;

SdFile b::root;
Sd2Card b::card;
SdVolume b::volume;


SdFile b::getSDRoot()
{
	if (rootInitialized)
	{
		return b::root;
	} 
	else
	{
		if (!card.init(SPI_HALF_SPEED, SD_CS)) 
		{
		    Serial.println("Initialization failed. Is a card is inserted?");
		    return SdFile();
		} 

		if (!volume.init(card)) 
		{
			Serial.println("Could not find FAT16/FAT32 partition.\nMake sure you've formatted the card");
		    return SdFile();
		}

		if (!root.openRoot(volume))
		{
		    Serial.println("Error opening root folder. Is something wrong with the card?");
		    return SdFile();
		}
		
		rootInitialized = true;
		b::root = root;
		return root;
	}	
}


