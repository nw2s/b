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


#include <SPI.h>
#include <SD.h>
#include "IO.h"
#include "SignalData.h"

using namespace nw2s;

bool SignalData::initialized = false;

SignalData* SignalData::fromSDFile(char *filepath)
{
	/* Make sure the card is plugged in, no way to gracefully recover */
	if (!SignalData::initialized && !SD.begin(SD_CS)) 
	{
		Serial.println("Card failed, or not present");
		return NULL;
	}
	
	SignalData::initialized = true;

	/* open the file that was requested */
  	File file = SD.open(filepath);

	if (file) 
	{
		int filebytes = file.size();
		int filewords = filebytes / 2;

		Serial.println("Loading Loop. Size: " + String(filebytes));

		/* Allocate as much space as we have reported by file size */
	 	unsigned short int *data = new unsigned short int[filewords];

		for (int i = 0; i < filewords; i++)
		{
			/* Read two bytes and make a word, little endian */
			unsigned char d0 = file.read();
			unsigned short int d1 = file.read();
			
			data[i] = ((d1 << 8) + d0) >> 4;
		}

		file.close();
		
		return new SignalData(data, filewords);
	}  
	else 
	{
		Serial.println("error opening file");
	} 
}

SignalData::SignalData(short unsigned int *data, long size)
{
	this->data = data;
	this->size = size;
}

long SignalData::getSize()
{
	return this->size;
}

unsigned short int SignalData::getSample(long sample)
{
	return this->data[sample];
}

