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

#ifndef Slew_h
#define Slew_h

#ifdef __AVR__
#define CVTYPE unsigned char
#else
#define CVTYPE int
#endif

namespace nw2s
{
	class Slew;
	class DecaySlew;
	class LinearSlew;
}

class nw2s::Slew
{
	public: 
		virtual CVTYPE calculate_value(CVTYPE input_value, int t) = 0;
};

class nw2s::DecaySlew : public Slew
{
	public:
		static DecaySlew* create(int duration);
		virtual CVTYPE calculate_value(CVTYPE input_value, int t);
		
	private:
		bool initialized;
		int speed;
		CVTYPE lastvalue;
		DecaySlew(int speed);
};

class nw2s::LinearSlew : public Slew
{
	public:
		static LinearSlew* create(int speed);
		virtual CVTYPE calculate_value(CVTYPE input_value, int t);

	private:
		bool initialized;
		int speed;
		CVTYPE lastvalue;
		LinearSlew(int speed);
};


#endif



