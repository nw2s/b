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

using namespace nw2s;

GameOfLife* GameOfLife::create(GridDevice deviceType, uint8_t columnCount, uint8_t rowCount, bool varibright)
{
	return new GameOfLife(deviceType, columnCount, rowCount, varibright);
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
	cvout[0] = AnalogOut::create(DUE_SPI_4822_00);
	cvout[1] = AnalogOut::create(DUE_SPI_4822_01);
	cvout[2] = AnalogOut::create(DUE_SPI_4822_02);
	cvout[3] = AnalogOut::create(DUE_SPI_4822_03);
	cvout[4] = AnalogOut::create(DUE_SPI_4822_04);
	cvout[5] = AnalogOut::create(DUE_SPI_4822_05);
	cvout[6] = AnalogOut::create(DUE_SPI_4822_06);
	cvout[7] = AnalogOut::create(DUE_SPI_4822_07);
	cvout[8] = AnalogOut::create(DUE_SPI_4822_08);
	cvout[9] = AnalogOut::create(DUE_SPI_4822_09);
	cvout[10] = AnalogOut::create(DUE_SPI_4822_10);
	cvout[11] = AnalogOut::create(DUE_SPI_4822_11);
	cvout[12] = AnalogOut::create(DUE_SPI_4822_12);
	cvout[13] = AnalogOut::create(DUE_SPI_4822_13);
	cvout[14] = AnalogOut::create(DUE_SPI_4822_14);
	cvout[15] = AnalogOut::create(DUE_SPI_4822_15);

	/* Give it a moment... */
	delay(200);		
}

void GameOfLife::timer(unsigned long t)
{
	/* Clock is rising */
	if (this->clockInput != DIGITAL_IN_NONE && !this->clockState && digitalRead(this->clockInput))
	{
		this->clockState = t;

		/* Simulate a clock pulse */
		this->reset();
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

void GameOfLife::nextGeneration()
{
	int nextGen = (generation + 1) % 2;

	for (int column = 0; column < columnCount; column++)
	{
		for (int row = 0; row < rowCount; row++)
		{
			int neighbours = calculateNeighbours(generation, column, row);
			if (debug)
			{
				Serial.print(neighbours);
			}
			lifecells[nextGen][column][row] = 0;
			if ((neighbours == 3 || neighbours == 2) && lifecells[generation][column][row])
				lifecells[nextGen][column][row] = constrain(lifecells[generation][column][row] - 2, 5, 15);
			else if ((neighbours == 3) && !lifecells[generation][column][row])
				lifecells[nextGen][column][row] = 15;
		}
		if (debug)
		{
			Serial.println();
		}
	}

	for (uint8_t column = 0; column < columnCount; column++)
	{
		for (uint8_t row = 0; row < rowCount; row++)
		{
			this->cells[0][column][row] = lifecells[nextGen][column][row];
		}
	}
	
	// TODO add array for digital outs, replace with a loop
	// If you're looking for that array, see IO.h array INDEX_DIGITAL_OUT
	digitalWrite(DUE_OUT_D00, lifecells[nextGen][0][0] == 0 ? LOW : HIGH);
	digitalWrite(DUE_OUT_D01, lifecells[nextGen][1][0] == 0 ? LOW : HIGH);
	digitalWrite(DUE_OUT_D02, lifecells[nextGen][2][0] == 0 ? LOW : HIGH);
	digitalWrite(DUE_OUT_D03, lifecells[nextGen][3][0] == 0 ? LOW : HIGH);
	digitalWrite(DUE_OUT_D04, lifecells[nextGen][4][0] == 0 ? LOW : HIGH);
	digitalWrite(DUE_OUT_D05, lifecells[nextGen][5][0] == 0 ? LOW : HIGH);
	digitalWrite(DUE_OUT_D06, lifecells[nextGen][6][0] == 0 ? LOW : HIGH);
	digitalWrite(DUE_OUT_D07, lifecells[nextGen][7][0] == 0 ? LOW : HIGH);
	digitalWrite(DUE_OUT_D08, lifecells[nextGen][8][0] == 0 ? LOW : HIGH);
	digitalWrite(DUE_OUT_D09, lifecells[nextGen][9][0] == 0 ? LOW : HIGH);
	digitalWrite(DUE_OUT_D10, lifecells[nextGen][10][0] == 0 ? LOW : HIGH);
	digitalWrite(DUE_OUT_D11, lifecells[nextGen][11][0] == 0 ? LOW : HIGH);
	digitalWrite(DUE_OUT_D12, lifecells[nextGen][12][0] == 0 ? LOW : HIGH);
	digitalWrite(DUE_OUT_D13, lifecells[nextGen][13][0] == 0 ? LOW : HIGH);
	digitalWrite(DUE_OUT_D14, lifecells[nextGen][14][0] == 0 ? LOW : HIGH);
	digitalWrite(DUE_OUT_D15, lifecells[nextGen][15][0] == 0 ? LOW : HIGH);

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
	if (column == 0 && row == 0)
	{
		debug = 1;
	}

	if (lifecells[generation][column][row])
	{
		this->cells[0][column][row] = 0;
		lifecells[generation][column][row] = 0;
	}
	else
	{
		this->cells[0][column][row] = 15;
		lifecells[generation][column][row] = 15;
	}

	if (isReady()) this->refreshGrid();
	delay(5);
	// Serial.println("\npressed");
}

void GameOfLife::buttonReleased(uint8_t column, uint8_t row)
{
	//this->clearLED(0, column, row);
	//delay(5);
	//Serial.println("released");
	//Serial.println("");
}


