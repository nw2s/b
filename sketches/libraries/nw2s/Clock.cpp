
#include "EventManager.h"
#include "Clock.h"
#include "IO.h"
#include <Arduino.h>
#include <math.h>

using namespace nw2s;

FixedClock* FixedClock::create(int tempo, int output)
{	
	return new FixedClock(tempo, output);
}

VariableClock* VariableClock::create(int mintempo, int maxtempo, int input, int output)
{	
	return new VariableClock(mintempo, maxtempo, input, output);
}

RandomClock* RandomClock::create(int mintempo, int maxtempo, int output)
{	
	return new RandomClock(mintempo, maxtempo, output);
}

FixedClock::FixedClock(int tempo, int output)
{
	/* Make sure the pin is low */
	this->output = output;
	clock_state = LOW;
	digitalWrite(this->output, LOW);
		
	/* The fixed clock operates on a regular period based on the tempo */
	int normalized_tempo = (tempo < 1) ? 1 : (tempo > 500) ? 500 : tempo;
		
	float periodms = (1.0 / normalized_tempo) * 60000.0;

	this->period = round(periodms);	
}

void FixedClock::timer(unsigned long t)
{
	if ((this->clock_state == LOW) && (t % this->period < TRIGGER_TIME))
	{
		this->clock_state = HIGH;
		digitalWrite(this->output, HIGH);
	}
	else if ((clock_state == HIGH) && (t % this->period > TRIGGER_TIME))
	{
		this->clock_state = LOW;
		digitalWrite(this->output, LOW);
	}	
}

VariableClock::VariableClock(int mintempo, int maxtempo, int input, int output)
{
	/* Make sure the pin is low, initialize variables */
	this->input = input;
	this->output = output;
	this->clock_state = LOW;
	this->last_clock_t = 0;
	this->next_clock_t = 0;
	
	digitalWrite(this->output, LOW);
		
	/* The variable clock operates on a period based on an input voltage */
	int normalized_mintempo = (mintempo < 1) ? 1 : (mintempo > 500) ? 500 : mintempo;
	int normalized_maxtempo = (maxtempo < 1) ? 1 : (maxtempo > 500) ? 500 : maxtempo;
	
	this->mintempo = normalized_mintempo;
	this->maxtempo = normalized_maxtempo;
}

void VariableClock::timer(unsigned long t)
{	
	if (this->next_clock_t == 0)
	{
		this->update_tempo(t);
	}
	else if ((clock_state == LOW) && (t >= this->next_clock_t))
	{
		clock_state = HIGH;
		digitalWrite(this->output, HIGH);
		
		/* Calculation of the current tempo is only done on clock tick */
		this->last_clock_t = t;	
		this->update_tempo(t);
	}
	else if ((clock_state == HIGH) && (t > this->last_clock_t + TRIGGER_TIME))
	{
		clock_state = LOW;
		digitalWrite(this->output, LOW);
	}
	else if (t % 100 == 0)
	{
		/* Let's also check the tempo every 100ms */
		this->update_tempo(t);		
	}
}

void VariableClock::update_tempo(unsigned long t)
{
	float tempo = ((this->maxtempo - this->mintempo) * (analogRead(this->input) / 1024.0)) + this->mintempo;

	float periodms = (1.0 / tempo) * 60000.0;

	this->period = round(periodms);
	this->next_clock_t = (this->last_clock_t + this->period);	
}

RandomClock::RandomClock(int mintempo, int maxtempo, int output)
{
	/* Make sure the pin is low, initialize variables */
	this->output = output;
	this->clock_state = LOW;
	this->last_clock_t = 0;
	this->next_clock_t = 0;
	
	digitalWrite(this->output, LOW);
		
	/* The random clock operates on a random period based on an input voltage */
	this->mintempo = (mintempo < 1) ? 1 : (mintempo > 500) ? 500 : mintempo;
	this->maxtempo = (maxtempo < 1) ? 1 : (maxtempo > 500) ? 500 : maxtempo;	
}

void RandomClock::timer(unsigned long t)
{	
	if (this->next_clock_t == 0)
	{
		this->next_t(t);
	}
	else if ((clock_state == LOW) && (t >= this->next_clock_t))
	{
		clock_state = HIGH;
		digitalWrite(this->output, HIGH);
		
		/* Calculation of the current tempo is only done on clock tick */
		this->last_clock_t = t;	
		this->next_t(t);
	}
	else if ((clock_state == HIGH) && (t > this->last_clock_t + TRIGGER_TIME))
	{
		clock_state = LOW;
		digitalWrite(this->output, LOW);
	}
}

void RandomClock::next_t(unsigned long t)
{
	int tempo = random(mintempo, maxtempo);
	
	float periodms = (1.0 / tempo) * 60000.0;

	this->next_clock_t = (this->last_clock_t + round(periodms));	
}



