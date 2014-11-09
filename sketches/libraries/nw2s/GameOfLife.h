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

#ifndef GAMEOFLIFE_H
#define GAMEOFLIFE_H

#include "Grid.h"
#include "IO.h"
#include "Clock.h"
#include "Gate.h"
#include "JSONUtil.h"

namespace nw2s
{
	class GameOfLife;
}

static const int newShapesCount = 13;
static const int newShapes[newShapesCount][3][3] 
	{
		{0,0,0,0,1,0,0,0,0},
		{0,0,0,0,1,1,0,0,0},
		{0,0,0,1,1,1,0,0,0},
		{0,0,0,0,1,1,0,1,0},
		{0,0,0,0,1,1,0,1,1},
		{0,0,0,1,1,1,0,1,0},
		{0,1,0,1,1,1,0,1,0},
		{0,1,0,1,0,1,0,1,0},
		{1,0,1,0,0,0,1,0,1},
		{1,1,1,1,0,0,0,1,0},
		{0,1,1,1,0,1,0,0,1},
		{0,1,0,0,0,1,1,1,1},
		{1,0,0,1,0,1,1,1,0}
	};
	
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
		void copyCellsToGrid(int gen);
		void shiftCells(int deltaX, int deltaY);
		void nextGeneration();
		int aRead(PinAnalogIn analogIn);
		
		PinDigitalIn clockInput				= DIGITAL_IN_NONE;
		PinDigitalIn newCellTrigger 		= DUE_IN_D1;
		PinDigitalIn generateRandomTrigger	= DUE_IN_D2;
		PinDigitalIn cvRulesOnSwitch		= DUE_IN_D3;
		PinDigitalIn shiftLeftTrigger		= DUE_IN_D4;
		PinDigitalIn shiftRightTrigger		= DUE_IN_D5;
		PinDigitalIn shiftUpTrigger			= DUE_IN_D6;
		PinDigitalIn shiftDownTrigger		= DUE_IN_D7;

		unsigned long clockState = 0;
		unsigned long newCellTriggerState = 0;
		unsigned long generateRandomTriggerState = 0;
		unsigned long cvRulesOnSwitchState = 0;
		unsigned long shiftLeftTriggerState = 0;
		unsigned long shiftRightTriggerState = 0;
		unsigned long shiftUpTriggerState = 0;
		unsigned long shiftDownTriggerState = 0;
		
		PinAnalogIn newCellShapeCV			= DUE_IN_A01;
		PinAnalogIn newCellXCV				= DUE_IN_A02;
		PinAnalogIn newCellYCV				= DUE_IN_A03;
		PinAnalogIn minNewCV				= DUE_IN_A04;
		PinAnalogIn maxNewCV				= DUE_IN_A05;
		PinAnalogIn minSurviveCV			= DUE_IN_A06;
		PinAnalogIn maxSurviveCV			= DUE_IN_A07;
		PinAnalogIn randomDensityCV			= DUE_IN_A08;
		
		int lifecells[2][16][16];
		int generation = 0;
		AnalogOut* cvout[16];
		int debug = 0;
		unsigned long currentTime = 0;
		unsigned long triggerStart = 0;
		int populationThreshold = 0;
};

#endif