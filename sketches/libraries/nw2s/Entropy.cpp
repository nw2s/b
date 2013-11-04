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
#include "MD5.h"
#include <Arduino.h>

using namespace nw2s;

#ifdef __AVR__

bool Entropy::seeded = false;

int Entropy::getValue()
{
	if (!seeded)
	{
		randomSeed(micros());
		Entropy::seeded = true;
	}
	
	return random(2147483647);
}

int Entropy::getValue(long max)
{
	if (!seeded)
	{
		randomSeed(micros());
		Entropy::seeded = true;
	}
	
	return random(max);
}

long Entropy::getValue(long min, long max)
{
	if (!seeded)
	{
		randomSeed(micros());
		Entropy::seeded = true;
	}
	
	return random(min, max);
}


#else

long Entropy::getValue()
{
	char data[17];
	
	/* Read 8 bytes worth of data */
	for (int k = 0; k < 17; k++)
	{
		unsigned char test = 0;

		/* Read 8 bits for a byte's worth of data */
		for (int j = 0; j < 8; j++)
		{
			test = (test << 1) | digitalRead(DUE_IN_DIGITAL_NOISE);
		}

		data[k] = test;
	}
	
	data[17] = 0;

	unsigned char* hash = MD5::make_hash(data);

	/* Convert the 128 bit MD5 to an integer */
	long value = 0;
	value = hash[0] ^ hash[1] ^ hash[2] ^ hash[3];
	value = (value << 8) + hash[4] ^ hash[5] ^ hash[6] ^ hash[7];
	value = (value << 8) + hash[8] ^ hash[9] ^ hash[10] ^ hash[11];
	value = (value << 8) + hash[12] ^ hash[13] ^ hash[14] ^ hash[15];	  	

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

#endif