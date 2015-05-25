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
#include "Entropy.h"

using namespace nw2s;

RatchetDivider* RatchetDivider::create(RatchetLimit limit, PinAnalogIn divisorInput, PinAnalogIn densityInput, PinDigitalOut output)
{	
	return new RatchetDivider(limit, divisorInput, densityInput, output);
}

RatchetDivider* RatchetDivider::create(aJsonObject* data)
{
	static const char outputNodeName[] = "triggerOutput";
	static const char divisorNodeName[] = "divisorInput";
	static const char densityNodeName[] = "densityInput";
	static const char limitNodeName[] = "limit";

	PinAnalogIn divisorInput = getAnalogInputFromJSON(data, divisorNodeName);
	PinAnalogIn densityInput = getAnalogInputFromJSON(data, densityNodeName);
	RatchetLimit limit = getLimitFromString(getStringFromJSON(data, limitNodeName));
	PinDigitalOut output = getDigitalOutputFromJSON(data, outputNodeName);
	
	return new RatchetDivider(limit, divisorInput, densityInput, output);
}

RatchetLimit RatchetDivider::getLimitFromString(char* value)
{	
	if (strcmp(value, "off") == 0)
	{
		return RATCHET_LIMIT_OFF;
	}

	if (strcmp(value, "even") == 0)
	{
		return RATCHET_LIMIT_EVEN;
	}

	if (strcmp(value, "odd") == 0)
	{
		return RATCHET_LIMIT_ODD;
	}

	if (strcmp(value, "primes") == 0)
	{
		return RATCHET_LIMIT_PRIMES;
	}

	if (strcmp(value, "trip") == 0)
	{
		return RATCHET_LIMIT_TRIP;
	}

	if (strcmp(value, "fibonacci") == 0)
	{
		return RATCHET_LIMIT_FIBONACCI;
	}

	if (strcmp(value, "poweroftwo") == 0)
	{
		return RATCHET_LIMIT_POWEROF2;
	}
}

RatchetDivider::RatchetDivider(RatchetLimit limit, PinAnalogIn divisorInput, PinAnalogIn densityInput, PinDigitalOut output)
{	
	this->divisorInput = divisorInput;
	this->densityInput = densityInput;
	this->mode = limit;
	this->output = Gate::create(output, 35);	
	this->clock_division = this->calculateClockDivision();
}

void RatchetDivider::reset()
{
	/* Check the density */
	if (densityInput != ANALOG_IN_NONE)
	{
		int density = (this->densityVal - 2048);

		/* Limit it to the positive range */
		density = (density < 0) ? 0 : (density > 2047) ? 2047 : density;
		
		if (density < Entropy::getValue(2047))
		{
			this->output->reset();
		}
	}
	else
	{
		this->output->reset();	
	}
}

void RatchetDivider::timer(unsigned long t)
{	
	this->output->timer(t);
	
	this->millisTimer++;
	
	if ((this->densityInput != ANALOG_IN_NONE) && (this->millisTimer % 251 == 0))
	{
		this->densityVal = analogRead(this->densityInput);
	}
	
	if (this->millisTimer % 252 == 0)
	{	
		this->clock_division = this->calculateClockDivision();
		
		//Serial.println(this->clock_division);
	}
}

uint32_t RatchetDivider::calculateClockDivision()
{
	int divisorVal = (analogRead(this->divisorInput) - 2048);
	
	/* Limit it to the positive range */
	divisorVal = (divisorVal < 0) ? 0 : (divisorVal > 2047) ? 2047 : divisorVal;
	
	uint32_t divisor = 1;
	
	if (mode == RATCHET_LIMIT_OFF)
	{
		/* Range 1 - 32 */
		divisor = (divisorVal >> 6) + 1;

	}
	else if (mode == RATCHET_LIMIT_ODD)
	{
		/* Range 1 - 33, odd only */
		divisor = ((divisorVal >> 7) << 1) + 1;
	}
	else if (mode == RATCHET_LIMIT_EVEN)
	{
		/* Range 2 - 34 even only */
		divisor = ((divisorVal >> 7) << 1) + 2;
	}
	else if (mode == RATCHET_LIMIT_PRIMES)
	{
		/* Limiting to the first 16 primes to make it more usable */
		divisor = primes[divisorVal >> 7];
	}
	else if (mode == RATCHET_LIMIT_TRIP)
	{
		/* Limiting to the first 8 */
		divisor = trip[divisorVal >> 8];
	}
	else if (mode == RATCHET_LIMIT_FIBONACCI)
	{
		/* Limiting to the first 8 */
		divisor = fib[divisorVal >> 8];
	}
	else if (mode == RATCHET_LIMIT_POWEROF2)
	{
		divisor = power2[divisorVal >> 9];		
	}
	
	return (1000 / divisor);
}


