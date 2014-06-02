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

#ifndef Clock_h
#define Clock_h

#include "EventManager.h"
#include "IO.h"
#include "../aJSON/aJSON.h"

#include <iterator>
#include <vector>

using namespace std;
using namespace nw2s;

namespace nw2s
{		
	static const int DIV_WHOLE = 4000;
	static const int DIV_HALF = 2000;
	static const int DIV_HALF_DOT = 3000;
	static const int DIV_QUARTER = 1000;
	static const int DIV_QUARTER_TRIPLET = 666;
	static const int DIV_QUARTER_DOT = 1500;
	static const int DIV_EIGHTH = 500;
	static const int DIV_EIGHTH_TRIPLET = 333;
	static const int DIV_EIGHTH_DOT = 750;
	static const int DIV_SIXTEENTH = 250;
	static const int DIV_SIXTEENTH_TRIPLET = 167;
	static const int DIV_SIXTEENTH_DOT = 375;
	static const int DIV_THIRTYSECOND = 125;
	static const int DIV_THIRTYSECOND_DOT = 188;
	static const int DIV_THIRTYSECOND_TRIPLET = 83;
	
	class BeatDevice;
	class Clock;
	class FixedClock;
	class VariableClock;
	class RandomTempoClock;
	// class SlaveClock;
	// class RandomDropoutClock;
}

class nw2s::BeatDevice : public TimeBasedDevice
{
	public:
		virtual int getclockdivision();
		virtual void reset() = 0;
		virtual void calculate();
		void setNextTime(unsigned long t);
		unsigned long getNextTime();
		
	protected:
		int clock_division;
		BeatDevice();
		
	private:
		unsigned long next_time;
};

class nw2s::Clock : public nw2s::TimeBasedDevice
{
	public:
		virtual void timer(unsigned long t);
 		void registerDevice(BeatDevice* device);
		void setSwing(int swingdivision, int swingpercentage);
		
	protected:
		volatile int period;
		volatile unsigned long last_clock_t;
		volatile unsigned long next_clock_t;
		unsigned char beats_per_measure;
		vector<BeatDevice*> devices;
		int beat;
		int swingpercentage;
		int swingdivision;

		Clock();
		
	private:
		virtual void updateTempo(unsigned long t);
};

class nw2s::FixedClock : public Clock
{
	public:
		static FixedClock* create(int tempo, unsigned char beats_per_measure);
		static FixedClock* create(aJsonObject* data);
		
	protected:		
		FixedClock(int tempo, unsigned char beats_per_measure);		

	private:
		virtual void updateTempo(unsigned long t);
};

class nw2s::VariableClock : public Clock
{
	public:
		static VariableClock* create(int mintempo, int maxtempo, PinAnalogIn input, unsigned char beats_per_measure);

	private:
		int mintempo;
		int maxtempo;
		int input;
		int pastvalues[4];
		int valuepointer;

		VariableClock(int mintempo, int maxtempo, PinAnalogIn input, unsigned char beats_per_measure);
		virtual void updateTempo(unsigned long t);
};

class nw2s::RandomTempoClock : public Clock
{
	public:
		static RandomTempoClock* create(int mintempo, int maxtempo, unsigned char beats_per_measure);

	private:
		int mintempo;
		int maxtempo;

		RandomTempoClock(int mintempo, int maxtempo, unsigned char beats_per_measure);
		virtual void updateTempo(unsigned long t);
};


// class nw2s::RandomDropoutClock : public FixedClock
// {
// 	public:
// 		static RandomDropoutClock* create(int tempo, unsigned char beats_per_measure, int chaos);
// 		virtual void timer(unsigned long t);
// 
// 	private:
// 		int chaos;
// 		
// 		RandomDropoutClock(int tempo, unsigned char beats_per_measure, int chaos);		
// };
//
// class nw2s::SlaveClock : public Clock
// {
// 	public:
// 		static SlaveClock* create(PinDigitalIn input, unsigned char beats_per_measure);
// 		virtual void timer(unsigned long t);
// 	
// 	private:
// 		static volatile bool trigger;
// 		static volatile unsigned long t;
// 		static volatile int period;
// 		static PinDigitalIn input;
// 		static volatile unsigned long last_clock_t;
// 		static volatile unsigned long next_clock_t;
// 
// 		SlaveClock(PinDigitalIn input, unsigned char beats_per_measure);
// 		static void isr();
// };

#endif

