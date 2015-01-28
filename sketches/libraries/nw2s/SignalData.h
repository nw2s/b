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


#ifndef SignalData_h
#define SignalData_h

#include <SD.h>

namespace nw2s
{		
	class SignalData;
	class StreamingSignalData;

	static const int STREAM_BUFFER_SIZE = 256;
	static const int READ_BUFFER_SIZE = STREAM_BUFFER_SIZE * 2;
}

class nw2s::SignalData
{
	public:
		static SignalData* fromArray(unsigned short int* source, long size);
		static SignalData* fromSDFile(char *filepath);
		long getSize();
		short int getSample(long sample);
		
	private:
		short int *data;
		int size;
		static bool initialized;
		
		SignalData(short int *data, long size);

};

class nw2s::StreamingSignalData
{
	public:
		static StreamingSignalData* fromSDFile(char *foldername, char* subfoldername, char *filename, bool loop = false);
		short int getNextSample();
		bool isAvailable();
		bool isReadyForRefresh();
		void refresh();
		void reset();
		void seekRandom();
		void reverse();

		void setStartFactor(uint16_t startfactor);
		void setEndFactor(uint16_t lengthFactor);
		void setFineEndFactor(uint16_t fineLengthFactor);
		
	private:
		int16_t buffer[2][nw2s::STREAM_BUFFER_SIZE];
		int16_t resetCache[nw2s::STREAM_BUFFER_SIZE];
		bool refreshCache = true;
		volatile uint8_t readbufferindex;
		volatile uint8_t writebufferindex;
		int size[2];
		uint32_t sampleCount = 0;
		uint16_t startFactor = 0;
		uint16_t endFactor = 4095;
		uint16_t fineEndFactor = 0;
		volatile int nextsampleindex;
		bool available;
		bool loop;
		bool reversed;
		SdFile file;

		uint32_t endIndex = 0;
		uint16_t subEndIndex = READ_BUFFER_SIZE;
		uint32_t startIndex = 0;
		
		StreamingSignalData(char *foldername, char* subfoldername, char *filename, bool loop = false);
		
		void calculateEndpoints();
};

#endif

