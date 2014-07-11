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

#ifndef Loop_h
#define Loop_h

#include "IO.h"
#include "SignalData.h"
#include "AudioDevice.h"
#include "Clock.h"
#include "../aJSON/aJSON.h"

namespace nw2s 
{
	typedef int SampleRateInterrupt;
	
	static const SampleRateInterrupt SR_10000 = 1050; 
	static const SampleRateInterrupt SR_12000 = 875; 
	static const SampleRateInterrupt SR_24000 = 437;   // Close enough 
	static const SampleRateInterrupt SR_48000 = 219;   // Closer... 
	static const SampleRateInterrupt SR_44100 = 238;   // Really close!  
	
	class Looper;
	class ClockedLooper;
	class BeatDevice;
}

SampleRateInterrupt sampleRateFromName(char* name);

class nw2s::Looper : public AudioDevice, public nw2s::TimeBasedDevice
{
	public:
		static Looper* create(PinAudioOut pin, char* subfoldername, char* filename, SampleRateInterrupt sri);
		static Looper* create(aJsonObject* data);
		virtual void timer(unsigned long t);
		virtual void timer_handler();
			
	protected:
		PinAudioOut pin;
		StreamingSignalData* signalData;
		int channel;
		int dac;

		Looper(PinAudioOut pin, char* subfoldername, char* filename, SampleRateInterrupt sri);
		
	private:
		static boolean initialized;
};

class nw2s::ClockedLooper : public nw2s::Looper, public nw2s::BeatDevice
{
	public:
		static ClockedLooper* create(PinAudioOut pin, char* subfoldername, char* filename, SampleRateInterrupt sri, int beats, int clockdivision);
		virtual void timer(unsigned long t);
		virtual void reset();
		
	private:
		int beats;
		int currentbeat;
		
		ClockedLooper(PinAudioOut pin, char* subfoldername, char* filename, SampleRateInterrupt sri, int beats, int clockdivision);

	};



#endif