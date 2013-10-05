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

namespace nw2s
{		
	class Clock;
	class FixedClock;
	class VariableClock;
	class RandomClock;
	class DividedClock;
	
	class TimeBasedDevice;
}

class nw2s::Clock : public nw2s::TimeBasedDevice
{
	public:
		virtual void timer(unsigned long t) = 0;
		
};

class nw2s::FixedClock : public Clock
{
	public:
		static FixedClock* create(int tempo, int output);
		virtual void timer(unsigned long t);
		
	private:
		int period;
		int output;
		volatile int clock_state;
		
		FixedClock(int tempo, int output);		
};

class nw2s::VariableClock : public Clock
{
	public:
		static VariableClock* create(int mintempo, int maxtempo, int input, int output);
		virtual void timer(unsigned long t);

	private:
		int mintempo;
		int maxtempo;
		int input;
		int output;
		volatile int period;
		volatile int clock_state;
		volatile unsigned long last_clock_t;
		volatile unsigned long next_clock_t;

		VariableClock(int mintempo, int maxtempo, int input, int output);
		void update_tempo(unsigned long t);
};

class nw2s::RandomClock : public Clock
{
	public:
		static RandomClock* create(int mintempo, int maxtempo, int output);
		virtual void timer(unsigned long t);
	
	private:
		int mintempo;
		int maxtempo;
		int output;
		volatile int clock_state;
		volatile unsigned long last_clock_t;
		volatile unsigned long next_clock_t;

		RandomClock(int mintempo, int maxtempo, int output);
		void next_t(unsigned long t);
			
};

//TODO: Divided Clock
//TODO: Gaussian Clock
//TODO: Missed Beat Clock

class nw2s::DividedClock : public Clock
{
	public:
		DividedClock(int tempo, double divisions[], int output);
			
};

#endif

