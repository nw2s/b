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

#include "GameOfLife.h"

#define TRIGGER_LENGTH 40

using namespace nw2s;

GameOfLife* GameOfLife::create(GridDevice deviceType, uint8_t columnCount, uint8_t rowCount, bool varibright)
{
	return new GameOfLife(deviceType, columnCount, rowCount, varibright);
}


GameOfLife* GameOfLife::create(aJsonObject* data)
{
	static const char rowsNodeName[] = "rows";
	static const char columnsNodeName[] = "columns";
	static const char clockNodeName[] = "externalClock";
	static const char varibrightNodeName[] = "varibright";

	GridDevice device = getGridDeviceFromJSON(data);
	int rows = getIntFromJSON(data, rowsNodeName, 8, 8, 16);
	int columns = getIntFromJSON(data, columnsNodeName, 8, 8, 16);
	bool varibright = getBoolFromJSON(data, varibrightNodeName, false);

	PinDigitalIn clockPin = getDigitalInputFromJSON(data, clockNodeName);
	
	GameOfLife* grid = new GameOfLife(device, columns, rows, varibright);
	
	if (clockPin != DIGITAL_IN_NONE)
	{
		grid->setClockInput(clockPin);
	}
			
	return grid;
}

GameOfLife::GameOfLife(GridDevice deviceType, uint8_t columnCount, uint8_t rowCount, bool varibright) : USBGridController(deviceType, columnCount, rowCount)
{
	this->varibright = varibright;

	this->beat = 0;
	this->clock_division = DIV_SIXTEENTH; // TODO pass as a parameter

	for (uint8_t page = 0; page < 16; page++)
	{
		for (uint8_t column = 0; column < 16; column++)
		{
			for (uint8_t row = 0; row < 16; row++)
			{
				this->cells[page][column][row] = 0;
			}
		}
	}

	memoryInitialized = true;
	
	// assign analog outs
	for (int i = 0; i < 16; i++)
		cvout[i] = AnalogOut::create(INDEX_ANALOG_OUT[i+1]);

	/* Give it a moment... */
	delay(200);		
}

void GameOfLife::timer(unsigned long t)
{
	currentTime = t;
	bool refresh = false;
	
	if (!newCellTriggerState && digitalRead(newCellTrigger))
	{
		newCellTriggerState = t;
		/*
		int x = analogRead(newCellXCV) * columnCount / 4096; // TODO FIX
		int y = analogRead(newCellYCV) * rowCount / 4096; // TODO FIX
		this->cells[0][x][y] = 15;
		lifecells[generation][x][y] = 15;
		refresh = true;
		*/
	}
	if ((newCellTriggerState != 0) && ((newCellTriggerState + 20) < t) && !digitalRead(newCellTrigger))
	{
		newCellTriggerState = 0;
	}
	
	if (!shiftLeftTriggerState && digitalRead(shiftLeftTrigger))
	{
		shiftLeftTriggerState = t;
		shiftCells(-1, 0);
		refresh = true;
	}
	if ((shiftLeftTriggerState != 0) && ((shiftLeftTriggerState + 20) < t) && !digitalRead(shiftLeftTrigger))
	{
		shiftLeftTriggerState = 0;
	}
	
	if (!shiftRightTriggerState && digitalRead(shiftRightTrigger))
	{
		shiftRightTriggerState = t;
		shiftCells(1, 0);
		refresh = true;
	}
	if ((shiftRightTriggerState != 0) && ((shiftRightTriggerState + 20) < t) && !digitalRead(shiftRightTrigger))
	{
		shiftRightTriggerState = 0;
	}
	
	if (!shiftDownTriggerState && digitalRead(shiftDownTrigger))
	{
		shiftDownTriggerState = t;
		shiftCells(0, -1);
		refresh = true;
	}
	if ((shiftDownTriggerState != 0) && ((shiftDownTriggerState + 20) < t) && !digitalRead(shiftDownTrigger))
	{
		shiftDownTriggerState = 0;
	}

	if (!shiftUpTriggerState && digitalRead(shiftUpTrigger))
	{
		shiftUpTriggerState = t;
		shiftCells(0, 1);
		refresh = true;
	}
	if ((shiftUpTriggerState != 0) && ((shiftUpTriggerState + 20) < t) && !digitalRead(shiftUpTrigger))
	{
		shiftUpTriggerState = 0;
	}

	if (triggerStart + TRIGGER_LENGTH > t)
	{
		for (int i = 0; i < columnCount; i++)
		{
			digitalWrite(INDEX_DIGITAL_OUT[i + 1], LOW);
		}
		triggerStart = 0;
	}
	
	/* Clock is rising */
	if (this->clockInput != DIGITAL_IN_NONE && !this->clockState && digitalRead(this->clockInput))
	{
		this->clockState = t;

		/* Simulate a clock pulse */
		this->reset();
	}
	else if (refresh && isReady())
	{
		this->refreshGrid();
	}

	/* Clock is falling and at least 20ms after rising */
	if (this->clockInput != DIGITAL_IN_NONE && (this->clockState != 0) && ((this->clockState + 20) < t) && !digitalRead(this->clockInput))
	{
		this->clockState = 0;
	}
}

void GameOfLife::reset()
{
	beat = (beat + 1) % columnCount;
	nextGeneration();

	if (isReady()) this->refreshGrid();
}

void GameOfLife::setClockInput(PinDigitalIn input)
{
	this->clockInput = input;
}

int GameOfLife::wrapX(int x)
{
	return (x + columnCount) % columnCount;
}

int GameOfLife::wrapY(int y)
{
	return (y + rowCount) % rowCount;
}

int GameOfLife::calculateNeighbours(int gen, int j, int  k)
{
	// could be a double for loop but just easier and faster to do it this way
	// but would be interesting to put the range of neighbours under control as well, 
	// and factor in distance when calculating the total neighbour count - TODO
	return 
		(lifecells[gen][wrapX(j-1)][wrapY(k-1)] ? 1 : 0) + 
		(lifecells[gen][wrapX(j-1)][wrapY(k)] ? 1 : 0) + 
		(lifecells[gen][wrapX(j-1)][wrapY(k+1)] ? 1 : 0) + 
		(lifecells[gen][wrapX(j)][wrapY(k-1)] ? 1 : 0) + 
		(lifecells[gen][wrapX(j)][wrapY(k+1)] ? 1 : 0) + 
		(lifecells[gen][wrapX(j+1)][wrapY(k-1)] ? 1 : 0) + 
		(lifecells[gen][wrapX(j+1)][wrapY(k)] ? 1 : 0) + 
		(lifecells[gen][wrapX(j+1)][wrapY(k+1)] ? 1 : 0);
}

int GameOfLife::countColumn(int gen, int column)
{
	int cv = 0;
	for (int j = 0; j < rowCount; j++)
		cv += lifecells[gen][column][j] ? 1 : 0;
	return cv;
}

void GameOfLife::copyCellsToGrid(int gen)
{
	for (uint8_t column = 0; column < columnCount; column++)
	{
		for (uint8_t row = 0; row < rowCount; row++)
		{
			this->cells[0][column][row] = lifecells[gen][column][row];
		}
	}
}

void GameOfLife::shiftCells(int deltaX, int deltaY)
{
	int nextGen = (generation + 1) % 2;
	for (uint8_t column = 0; column < columnCount; column++)
	{
		for (uint8_t row = 0; row < rowCount; row++)
		{
			lifecells[nextGen][wrapX(column + deltaX)][wrapY(row + deltaY)] = lifecells[generation][column][row];
		}
	}
	copyCellsToGrid(nextGen);
	generation = nextGen;
}

void GameOfLife::nextGeneration()
{
	int nextGen = (generation + 1) % 2;
	bool isRandom = digitalRead(generateRandomTrigger);

	for (int column = 0; column < columnCount; column++)
	{
		for (int row = 0; row < rowCount; row++)
		{
			if (isRandom)
			{
				lifecells[nextGen][column][row] = random(0, 2) ? 15 : 0;
			}
			else
			{
				int neighbours = calculateNeighbours(generation, column, row);
				if (debug)
				{
					Serial.print(neighbours);
				}
				lifecells[nextGen][column][row] = 0;
				if ((neighbours == 3 || neighbours == 2) && lifecells[generation][column][row])
					lifecells[nextGen][column][row] = constrain(lifecells[generation][column][row] - 1, 5, 15);
				else if ((neighbours == 3) && !lifecells[generation][column][row])
					lifecells[nextGen][column][row] = 15;
			}
		}
		if (debug)
		{
			Serial.println();
		}
	}

	this->copyCellsToGrid(nextGen);
	
	for (int i = 0; i < columnCount; i++)
	{
		digitalWrite(INDEX_DIGITAL_OUT[i + 1], !lifecells[generation][i][0] && lifecells[nextGen][i][0] ? HIGH : LOW);
	}
	triggerStart = currentTime;
	
	if (debug)
	{
		Serial.println("-- generation --");
		for (int i = 0; i < columnCount; i++)
		{
			for (int j = 0; j < rowCount; j++)
			{
				Serial.print(lifecells[generation][i][j]);
			}
			Serial.println();
		}
		Serial.println("-- neighbours --");
		for (int i = 0; i < columnCount; i++)
		{
			for (int j = 0; j < rowCount; j++)
			{
				Serial.print(calculateNeighbours(generation, i, j));
			}
			Serial.println();
		}
		Serial.println("-- next generation --");
		for (int i = 0; i < columnCount; i++)
		{
			for (int j = 0; j < rowCount; j++)
			{
				Serial.print(lifecells[nextGen][i][j]);
			}
			Serial.println();
		}
		debug = 0;
	}

	for (int i = 0; i < columnCount; i++)
		cvout[i]->outputCV((4096 / rowCount * countColumn(nextGen, i)) % 4096); // TODO convert to bit ops
	
	generation = nextGen;
}

void GameOfLife::buttonPressed(uint8_t column, uint8_t row)
{	
	/*
	if (column == 0 && row == 0)
	{
		debug = 1; 
	}
	*/
	
	if (lifecells[generation][column][row])
	{
		this->cells[0][column][row] = 0;
		lifecells[generation][column][row] = 0;
		digitalWrite(INDEX_DIGITAL_OUT[column + 1], !lifecells[generation][column][0] && lifecells[(generation + 1) % 2][column][0] ? HIGH : LOW);
	}
	else
	{
		this->cells[0][column][row] = 15;
		lifecells[generation][column][row] = 15;
		digitalWrite(INDEX_DIGITAL_OUT[column + 1], LOW);
	}

	if (isReady()) this->refreshGrid();
	delay(5);
	// Serial.println("\npressed");
}

void GameOfLife::buttonReleased(uint8_t column, uint8_t row)
{
	//Serial.println("");
}


