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

#ifndef RatchetDivider_h
#define RatchetDivider_h

#include "IO.h"
#include "Clock.h"
#include "aJson.h"
#include "Gate.h"

namespace nw2s
{		
	class RatchetDivider;

	const int RATCHET_TRIGGER_TIME = 35;
	
	enum RatchetLimit
	{
		RATCHET_LIMIT_OFF,
		RATCHET_LIMIT_ODD,
		RATCHET_LIMIT_EVEN,
		RATCHET_LIMIT_PRIMES,
		RATCHET_LIMIT_TRIP,
		RATCHET_LIMIT_FIBONACCI,
		RATCHET_LIMIT_POWEROF2
	};
	
	static const uint8_t primes[32] = { 2, 3, 5, 7, 11, 13, 17, 19, 23, 29, 31, 37, 41, 43, 47, 53, 59, 61, 67, 71, 73, 79, 83, 89, 97, 101, 103, 107, 109, 113, 127, 127 };
	static const uint8_t trip[14] = { 1, 2, 3, 4, 6, 8, 12, 16, 24, 32, 48, 64, 96, 128 };
	static const uint8_t fib[13] = { 1, 2, 3, 5, 8, 8, 13, 21, 34, 55, 89, 89, 144 };
	static const uint8_t power2[8] = { 1, 2, 4, 8, 16, 32, 64, 128 };
}

class nw2s::RatchetDivider : public nw2s::BeatDevice
{
	public:
		
		static RatchetDivider* create(RatchetLimit limit, PinAnalogIn divisorInput, PinAnalogIn densityInput, PinDigitalOut output);
		static RatchetDivider* create(aJsonObject* data);

		virtual void timer(unsigned long t);
		virtual void reset();

	private:
		
		Gate* output;
		PinAnalogIn divisorInput = ANALOG_IN_NONE;
		PinAnalogIn densityInput = ANALOG_IN_NONE;
		uint32_t densityVal = 0;
		uint32_t millisTimer = 0;
		RatchetLimit mode = RATCHET_LIMIT_OFF;
		
		RatchetDivider(RatchetLimit limit, PinAnalogIn divisorInput, PinAnalogIn densityInput, PinDigitalOut output);
		static RatchetLimit getLimitFromString(char* value);
		uint32_t calculateClockDivision();		
};

#endif

