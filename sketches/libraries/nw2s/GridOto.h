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

#include "Grid.h"
#include "IO.h"
#include "Clock.h"
#include "Gate.h"
#include "Key.h"

namespace nw2s
{
	class GridOto;
	
	enum OtoCellDirection
	{
		CELL_UP = 0,
		CELL_RIGHT = 1,
		CELL_DOWN = 2,
		CELL_LEFT = 3,
	};

	struct OtoCell
	{
		bool isActive;
		uint8_t column;
		uint8_t row;
		OtoCellDirection direction;
	};
}



class nw2s::GridOto : public BeatDevice, public USBGridController
{
	public:
		
		static GridOto* create(GridDevice deviceType, uint8_t columnCount, uint8_t rowCount, int clockDivision, NoteName key, Scale scale, PinDigitalOut outd0, PinAnalogOut outa0, int notes0[][2], PinDigitalOut outd1, PinAnalogOut outa1, int notes1[][2], PinDigitalOut outd2, PinAnalogOut outa2, int notes2[][2], PinDigitalOut outd3, PinAnalogOut outa3, int notes3[][2]);
		static GridOto* create(aJsonObject* data);

		virtual void timer(unsigned long t);
		virtual void reset();
		
		void setClockInput(PinDigitalIn input);	

	protected:
		
		virtual void buttonPressed(uint8_t column, uint8_t row);
		virtual void buttonReleased(uint8_t column, uint8_t row);
		
	private:
		
		GridOto(GridDevice deviceType, uint8_t columnCount, uint8_t rowCount, int clockDivision, NoteName key, Scale scale, PinDigitalOut outd0, PinAnalogOut outa0, int notes0[][2], PinDigitalOut outd1, PinAnalogOut outa1, int notes1[][2], PinDigitalOut outd2, PinAnalogOut outa2, int notes2[][2], PinDigitalOut outd3, PinAnalogOut outa3, int notes3[][2]);
		
		Key* key;
		Gate* gates[4];	
		AnalogOut* outs[4];
		int notes[4][16][2];

		uint8_t maxcells = 0;
		OtoCell otocells[32];

		unsigned long clockState = 0;
		
		PinDigitalIn clockInput = DIGITAL_IN_NONE;		
};