/*

	Conway's Game of Life device. To be used with Monome grids.
	Copyright (C) 2014 scanner darkly (fuzzybeacon@gmail.com)

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
#include "JSONUtil.h"

namespace nw2s
{
	class GameOfLife;
}

class nw2s::GameOfLife : public BeatDevice, public USBGridController
{
	public:
		
		static GameOfLife* create(GridDevice deviceType, uint8_t columnCount, uint8_t rowCount, bool varibright);
		static GameOfLife* create(aJsonObject* data);

		virtual void timer(unsigned long t);
		virtual void reset();

		void setClockInput(PinDigitalIn input);	
		
	protected:
		
		virtual void buttonPressed(uint8_t column, uint8_t row);
		virtual void buttonReleased(uint8_t column, uint8_t row);
		
	private:
		
		GameOfLife(GridDevice deviceType, uint8_t columnCount, uint8_t rowCount, bool varibright);

		int wrapX(int x);
		int wrapY(int y);
		int calculateNeighbours(int gen, int j, int  k);
		int countColumn(int gen, int column);
		void nextGeneration();
		
		unsigned long clockState = 0;
		PinDigitalIn clockInput = DIGITAL_IN_NONE;
		int lifecells[2][16][16];
		int generation = 0;
		AnalogOut* cvout[16];
		int debug = 0;
};