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

#include "Entropy.h"
#include <Arduino.h>

using namespace nw2s;

bool Entropy::seeded = false;


bool Entropy::getBit()
{
	bool random1 = random(1);
	bool random2 = digitalRead(DUE_IN_DIGITAL_NOISE);
	
	return random1 ^ random2;	
}

long Entropy::getValue()
{
	if (!seeded)
	{
		randomSeed(micros());
		Entropy::seeded = true;
	}

	unsigned long noisedata = 0;
	
	/* Read 31 bits worth of data */
	for (int j = 0; j < 31; j++)
	{
		//TODO: Change this to use the register directly
		noisedata = (noisedata << 1) | digitalRead(DUE_IN_DIGITAL_NOISE);
	}

	/* xor the PRNG number with the noise number */
	long value = random(0, 2147483647) ^ noisedata;

	return value > -1 ? value : value * -1;
}

long Entropy::getValue(long max)
{
	/* Yes, this is not the ideal, but we're on a deadline here! */
	return (getValue() % (max + 1));
}

long Entropy::getValue(long min, long max)
{
	/* Yes, this is not the ideal, but we're on a deadline here! */
	return (min + (getValue() % (max - min + 1)));
}

