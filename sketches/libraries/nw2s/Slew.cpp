
#include "Slew.h"


using namespace nw2s;


DecaySlew* DecaySlew::create(int speed)
{
	return new DecaySlew(speed);
}

LinearSlew* LinearSlew::create(int speed)
{
	return new LinearSlew(speed);
}


DecaySlew::DecaySlew(int speed)
{
	this->initialized = false;
	this->speed = speed;
}

int DecaySlew::calculate_value(int input, int t)
{
	//TODO: Can't retrigger?
	//TODO: There seems to be a weird thing at the end when slewing up.
	
	if (!initialized)
	{
		lastvalue = input * 100L; 
		initialized = true;
	}
	
	long scale_input = input * 100L;

	this->lastvalue = scale_input + ( ( ( this->lastvalue - scale_input ) * ( ( ( this->speed - ((t + (this->speed/5))/(this->speed/5))) * 100L) / this->speed ) ) / 100L ); 
	
	return lastvalue / 100;
}

LinearSlew::LinearSlew(int speed)
{
	this->initialized = false;
	this->speed = speed;
}

int LinearSlew::calculate_value(int input, int t)
{	
	if (!initialized)
	{
		lastvalue = input; 
		initialized = true;
	}

	if (lastvalue == input)
	{
		return input;
	}
	else if (lastvalue > input)
	{
		return --lastvalue;
	}
	else
	{
		return ++lastvalue;
	}
}










