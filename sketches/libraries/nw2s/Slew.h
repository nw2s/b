

#ifndef Slew_h
#define Slew_h


namespace nw2s
{
	class Slew;
	class DecaySlew;
	class LinearSlew;
}

class nw2s::Slew
{
	public: 
		virtual int calculate_value(int input_value, int t) = 0;
};

class nw2s::DecaySlew : public Slew
{
	public:
		static DecaySlew* create(int duration);
		virtual int calculate_value(int input_value, int t);
		
	private:
		bool initialized;
		int speed;
		int lastvalue;
		DecaySlew(int speed);
};

class nw2s::LinearSlew : public Slew
{
	public:
		static LinearSlew* create(int speed);
		virtual int calculate_value(int input_value, int t);

	private:
		bool initialized;
		int speed;
		int lastvalue;
		LinearSlew(int speed);
};


#endif



