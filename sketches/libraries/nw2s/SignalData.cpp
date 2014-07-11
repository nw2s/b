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
#include "b.h"
#include "IO.h"
#include "SignalData.h"

using namespace nw2s;


static const int MAX_MEM_BUFFER_WORDS = 4000;
static const short int UNSIGNED_OFFSET = 0b0111111111111111;


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

SignalData* SignalData::fromArray(unsigned short int* source, long size)
{
 	unsigned short int *data = new unsigned short int[size];

	for (int i = 0; i < size; i++)
	{
		data[i] = source[i];
	}
	
	return new SignalData(data, size);
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


StreamingSignalData* StreamingSignalData::fromSDFile(char* foldername, char* subfoldername, char *filename, bool loop)
{	
	return new StreamingSignalData(foldername, subfoldername, filename, loop);
}

StreamingSignalData::StreamingSignalData(char* foldername, char* subfoldername, char *filename, bool loop)
{
	SdFile root;
	SdFile samplesDir;
	SdFile subDir;

	/* open the file that was requested */
	root = b::getSDRoot();

	if (!samplesDir.open(root, foldername, O_READ))
	{
	    Serial.println("Error opening samples folder. Are you sure it's there?");
	    return;
	}
	
	if (!subDir.open(samplesDir, subfoldername, O_READ))
	{
	    Serial.println("Error opening samples folder. Are you sure it's there?");
	    return;
	}
	
	if (!this->file.open(subDir, filename, O_READ))
	{
	    Serial.println("Error opening file. Are you sure it's there?");
	    return;
	}
			
	this->loop = loop;
	this->available = file.fileSize() > 0;
	this->size[0] = 0;
	this->size[1] = 0;

	this->writebufferindex = 0;
	this->readbufferindex = 1;

	this->refresh();
	
	this->readbufferindex = 0;
	this->nextsampleindex = 0;
}

bool StreamingSignalData::isAvailable()
{
	return available;
}

bool StreamingSignalData::isReadyForRefresh()
{
	return this->readbufferindex != this->writebufferindex;
}

void StreamingSignalData::refresh()
{
	unsigned char d[READ_BUFFER_SIZE];
	int dsize = 0;

	/* If we're not looping and have reached eof, then stop */
	if (!this->loop && (this->file.curPosition() >= this->file.fileSize() - 1))
	{
		this->available = false;
		return;
	}
	
	/* If we're reading from a different buffer, fill up the write buffer */
	if (this->readbufferindex != this->writebufferindex)
	{
		/* Fill the current write buffer up as much as possible */		
		if (this->available)
		{
			/* Read up to the buffer size number of bytes */
			dsize = this->file.read(d, READ_BUFFER_SIZE);
			this->available = dsize > -1;

			/* If we're looping and didn't get enough bytes, rewind and start over */
			while (loop && available && dsize < READ_BUFFER_SIZE)
			{
				this->file.rewind();
				dsize += this->file.read(&(d[dsize]), READ_BUFFER_SIZE - dsize);
			}

			/* Convert from signed little endian */
			for (int i = 0; i < dsize; i += 2)
			{
				/* Read two bytes and make a word */
				unsigned char d0 = d[i];
				unsigned short int d1 = d[i + 1];
	
				/* Get a signed value and make it unsigned */
				int value = ((d1 << 8) | d0) + UNSIGNED_OFFSET;

				/* We're throwing 4 bits away to make 12 bit audio - for now */
				this->buffer[writebufferindex][i / 2] = ((unsigned short int)value >> 4);
			}
		}
	
		this->size[writebufferindex] = dsize / 2;	
		this->writebufferindex = !this->writebufferindex;
	}
}

short unsigned int StreamingSignalData::getNextSample()
{
	if (!available)
	{
		return 0;
	}
	
	/* Get the next sample and hold on to it */
	short unsigned int nextsample = this->buffer[readbufferindex][nextsampleindex];

	/* If we're at the end of a buffer, move to the next */
	if (this->nextsampleindex == (this->size[readbufferindex] - 1))
	{
		this->readbufferindex = !this->readbufferindex;
		this->nextsampleindex = 0;
	}
	else
	{
		this->nextsampleindex++;		
	}	

	return nextsample;
}

void StreamingSignalData::reset()
{
	this->writebufferindex = !this->readbufferindex;

	this->file.rewind();	
	this->refresh();

	this->readbufferindex = this->writebufferindex;
	this->nextsampleindex = 0;	
}

