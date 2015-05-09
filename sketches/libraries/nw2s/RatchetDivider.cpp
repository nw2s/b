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

#include "RatchetDivider.h"
#include "IO.h"
#include "aJson.h"
#include "JSONUtil.h"

using namespace nw2s;

RatchetDivider* RatchetDivider::create(RatchetLimit limit, PinAnalogIn divisorInput, PinAnalogIn densityInput, PinDigitalOut output)
{	
	return new RatchetDivider(limit, divisorInput, densityInput, output);
}

RatchetDivider* RatchetDivider::create(aJsonObject* data)
{
	// static const char outputNodeName[] = "triggerOutput";
	//
	// int clock_division = getDivisionFromJSON(data);
	// PinDigitalOut output = getDigitalOutputFromJSON(data, outputNodeName);
	//
	// return new RatchetDivider(output, clock_division);
}

RatchetDivider::RatchetDivider(RatchetLimit limit, PinAnalogIn divisorInput, PinAnalogIn densityInput, PinDigitalOut output)
{
	
	/* Make sure the pin is low */
	pinMode(output, OUTPUT);
	this->output = output;
	this->state = LOW;
	this->t_start = 0;
	digitalWrite(this->output, LOW);		
		
	this->clock_division = this->calculateClockDivision();
}

void RatchetDivider::reset()
{
	/* Reset turns the trigger on */
	this->t_start = 0;
	this->state = HIGH;
	digitalWrite(this->output, HIGH);
	
	this->clock_division = this->calculateClockDivision();
}

void RatchetDivider::timer(unsigned long t)
{	
	/* If the state is low, nothing else to do */
	if (this->state == LOW) return;
	
	if (this->t_start == 0)
	{
		this->t_start = t;
	}
	else
	{
		if ((this->state == HIGH) && (t - this->t_start > RATCHET_TRIGGER_TIME))
		{
			this->state = LOW;
			digitalWrite(this->output, LOW);
		}	
	}	
	
	//TODO: Occasionally check the potentiometer to see if the division changed	
}

uint32_t RatchetDivider::calculateClockDivision()
{
	int divisorVal = analogRead(this->divisorInput);
	uint8_t divisor = 1;
	
	if (mode == RATCHET_LIMIT_OFF)
	{
		divisor = (divisorVal >> 5) + 1;
	}
	else if (mode == RATCHET_LIMIT_ODD)
	{
		divisor = ((divisorVal >> 6) << 1) + 1;
	}
	else if (mode == RATCHET_LIMIT_EVEN)
	{
		divisor = ((divisorVal >> 6) << 1) + 2;
	}
	else if (mode == RATCHET_LIMIT_PRIMES)
	{
		divisor = primes[divisorVal >> 7];
	}
	else if (mode == RATCHET_LIMIT_TRIP)
	{
		divisor = trip[divisorVal >> 8];
	}
	else if (mode == RATCHET_LIMIT_FIBONACCI)
	{
		divisor = fib[divisorVal >> 8];
	}
	else if (mode == RATCHET_LIMIT_POWEROF2)
	{
		divisor = power2[divisorVal >> 9];		
	}
	
	return (1000 / divisor);
}


