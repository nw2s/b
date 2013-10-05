
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

