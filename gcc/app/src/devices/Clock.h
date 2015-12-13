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
#include "aJSON/aJSON.h"

#include <iterator>
#include <vector>

#define CLOCK_DIVISION_COUNT 15

using namespace std;
using namespace nw2s;

namespace nw2s
{
	static const int DIV_NEVER = -1;
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
	
	static const int CLOCK_DIVISIONS[CLOCK_DIVISION_COUNT] = { DIV_WHOLE, DIV_HALF_DOT, DIV_HALF, DIV_QUARTER_DOT, DIV_QUARTER, DIV_QUARTER_TRIPLET, DIV_EIGHTH_DOT, DIV_EIGHTH, DIV_EIGHTH_TRIPLET, DIV_SIXTEENTH_DOT, DIV_SIXTEENTH, DIV_SIXTEENTH_TRIPLET, DIV_THIRTYSECOND_DOT, DIV_THIRTYSECOND, DIV_THIRTYSECOND_TRIPLET };
	
	class BeatDevice;
	class Clock;
	class FixedClock;
	class VariableClock;
	class RandomTempoClock;
	class TapTempoClock;
	class PassthruClock;
	
	int clockDivisionFromName(char* name);
}

class nw2s::BeatDevice : public TimeBasedDevice
{
	public:
		virtual int getclockdivision();
		virtual void reset() = 0;
		virtual void calculate();
		void setNextTime(unsigned long t);
		unsigned long getNextTime();
		
		void setStopInput(PinDigitalIn input);
		void setClockDivisionInput(PinAnalogIn input);
		bool isStopped();
		
	protected:
		
		int clock_division;
				
		BeatDevice();
		
		
	private:
		unsigned long next_time = 0;
		PinDigitalIn stopInput = DIGITAL_IN_NONE;
		PinAnalogIn clockDivisionInput = ANALOG_IN_NONE;
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
		static VariableClock* create(aJsonObject* data);

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
		static RandomTempoClock* create(aJsonObject* data);

	private:
		int mintempo;
		int maxtempo;

		RandomTempoClock(int mintempo, int maxtempo, unsigned char beats_per_measure);
		virtual void updateTempo(unsigned long t);
};

class nw2s::TapTempoClock : public Clock
{
	public: 
		static TapTempoClock* create(PinDigitalIn input, PinDigitalIn resetInput, unsigned char beats_per_measure);
		static TapTempoClock* create(aJsonObject* data);
	
	private:
		
		/* Note - since this is interrupt-driven, only one can catch the taps at a time */
		static TapTempoClock* tapTempoClock;
		
		uint32_t lastT = 0;
		uint32_t tempo = 0;
		volatile static bool tapping;
		
		/* These remember the last time we saw the input high to avoid "falling" taps */
		uint32_t lastTapStateT = 0;
		
		PinDigitalIn input;
		PinDigitalIn resetInput;
		
		TapTempoClock(PinDigitalIn input, PinDigitalIn resetInput, unsigned char beats_per_measure);
		virtual void updateTempo(unsigned long t);
		virtual void timer(uint32_t t);
		void reset();
		void tap(uint32_t t);
		static void onTempoTap();
};

class nw2s::PassthruClock : public Clock
{
	public: 
		static PassthruClock* create(PinDigitalIn input, unsigned char beats_per_measure);
		static PassthruClock* create(aJsonObject* data);
	
	private:
		
		/* Note - since this is interrupt-driven, only one can catch the taps at a time */
		static PassthruClock* passthruClock;
		
		uint32_t lastT = 0;
		uint32_t tempo = 0;
		volatile static bool tapping;
		
		/* These remember the last time we saw the input high to avoid "falling" taps */
		uint32_t lastTapStateT = 0;
		
		PinDigitalIn input;
		
		PassthruClock(PinDigitalIn input, unsigned char beats_per_measure);
		virtual void updateTempo(unsigned long t);
		virtual void timer(uint32_t t);
		void reset();
		void tap(uint32_t t);
		static void onTap();
};

#endif

