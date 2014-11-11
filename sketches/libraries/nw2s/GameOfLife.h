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

#include "b.h"
#include "Grid.h"
#include "IO.h"
#include "Clock.h"
#include "Gate.h"
#include "JSONUtil.h"

namespace nw2s
{
	class GameOfLife;
	class GameOfLifeConfig;
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
	
class nw2s::GameOfLifeConfig
{
	public:
		int gateMode[14] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,}; // 0 == trigger, 1 == gate // only 14 because first 2 are always gates (too full / too empty)
		int cvMode[16] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}; // 0 == CV, 1 == note // for now always CV, note mode to be added in v2
		int cvRangeMin[16] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
		int cvRangeMax[16] = {4096,4096,4096,4096,4096,4096,4096,4096,4096,4096,4096,4096,4096,4096,4096,4096};
		int noteScale[16] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}; // not used right now, to be added in v2
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
		
		void readConfig();
		void saveConfig();
		void renderCVControlColumn(int column);
		void renderGateModeRow();
		void renderCVModeRow();
		void renderControlPage();
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

		PinAnalogIn newCellShapeCV			= DUE_IN_A01;
		PinAnalogIn newCellXCV				= DUE_IN_A02;
		PinAnalogIn newCellYCV				= DUE_IN_A03;
		PinAnalogIn minNewCV				= DUE_IN_A04;
		PinAnalogIn maxNewCV				= DUE_IN_A05;
		PinAnalogIn minSurviveCV			= DUE_IN_A06;
		PinAnalogIn maxSurviveCV			= DUE_IN_A07;
		PinAnalogIn randomDensityCV			= DUE_IN_A08;

		AnalogOut* cvout[16];
		
		GameOfLifeConfig config;
		int debug = 0;
		int lifecells[2][16][16]; // indices are generation, column, row
		int generation = 0; // currently displayed generation

		// variables to track trigger inputs
		unsigned long clockState = 0;
		unsigned long newCellTriggerState = 0;
		unsigned long generateRandomTriggerState = 0;
		unsigned long cvRulesOnSwitchState = 0;
		unsigned long shiftLeftTriggerState = 0;
		unsigned long shiftRightTriggerState = 0;
		unsigned long shiftUpTriggerState = 0;
		unsigned long shiftDownTriggerState = 0;
		
		bool isControl = false; // true when in control mode
		bool controlButton1 = false;
		bool controlButton2 = false;
		unsigned long currentTime = 0;
		unsigned long triggerStart = 0;
		int populationThreshold = 0;
		char * jsonBuffer = (char*) malloc(512);
};

#endif