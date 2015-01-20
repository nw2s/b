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

bool b::softTune = false;
int16_t b::offset[16] = {-5, -2, -1, -3, -5, -6, -4, -6, -5, -31, -3, -4, -7, -5, -5, -8};
int32_t b::scale[16] = {998, 996, 997, 998, 996, 1002, 1011, 1003, 1000, 996, 1009, 1001, 997, 999, 996, 991};




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


