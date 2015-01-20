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


#ifndef b_h
#define b_h

#include <SD.h>


#define TUNE_SCALE_FACTOR 1000

namespace nw2s
{
	class b;

	enum cvGainModeValues 
	{
		CV_GAIN_HIGH = 1,  // This is for when you're biasing to -10V and +10V
		CV_GAIN_LOW = 0,   // This is for when you've biased to -5V to +5V
	};
	
	enum DeviceModel
	{
		NW2S_B_1_0_0
	};
}

class nw2s::b
{
	public:

		//TOD: most (all?) of these should be read-only getters
		static DeviceModel model;
		static bool debugMode;
		static bool cvGainMode;
		
		static bool softTune;
		static int16_t offset[16];
		static int32_t scale[16];
		static void configure();
		
		static SdFile getSDRoot();
		
	private:

		static SdFile root;
		static bool rootInitialized;
		static Sd2Card card;
		static SdVolume volume;		
};


#endif


