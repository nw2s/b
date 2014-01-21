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

#ifndef Oscillator_h
#define Oscillator_h

#include "IO.h"
#include "AudioDevice.h"
#include "EventManager.h"

namespace nw2s
{
	class Oscillator;
	class VCO;
	class Saw;
	class DiscreteNoise;
	class AliasingFilter;
}

class nw2s::Oscillator : public AudioDevice 
{
	public:
		
		
	protected:
		PinAudioOut pinout;
		int channel;
		int dac;
		
		virtual void timer_handler();
		virtual int getSample() = 0;
		virtual void nextSample() = 0;
		
		Oscillator(PinAudioOut pinout);
};

class nw2s::VCO : public Oscillator, public TimeBasedDevice
{
	public:
		virtual void timer(unsigned long t);
			
	protected:
		int frequency;  // actually it's frequency * 100
		int samplespercycle;
		int phaseindex;

		PinAnalogIn pinin;		

		VCO(PinAudioOut pinout, PinAnalogIn pinin);
		virtual int getSample();
		virtual void nextSample();
		virtual int nextVCOSample() = 0;
		
	private:
		int sample;
		
};

class nw2s::Saw : public VCO
{
	public:
		static Saw* create(PinAudioOut pinout, PinAnalogIn pinin);
		
	private:
		Saw(PinAudioOut pinout, PinAnalogIn pinin);
		virtual int nextVCOSample();
};

class nw2s::DiscreteNoise : public VCO
{
	public:
		static DiscreteNoise* create(PinAudioOut pinout, PinAnalogIn pinin);
		
	private:		
		int currentvalue;
		
		DiscreteNoise(PinAudioOut pinout, PinAnalogIn pinin);
		virtual int nextVCOSample();
};


#endif


