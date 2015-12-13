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

#include "SlideRuleGrid.h"
#include "JSONUtil.h"
#include "Entropy.h"

#define GATE_DURATION 50

using namespace nw2s;


SlideRuleGrid* SlideRuleGrid::create(GridDevice deviceType, uint8_t columnCount, uint8_t rowCount, int clockDivision, PinDigitalOut outd0, PinAnalogOut outa0, PinDigitalOut outd1, PinAnalogOut outa1, PinDigitalOut outd2, PinAnalogOut outa2, PinDigitalOut outd3, PinAnalogOut outa3)
{
	return new SlideRuleGrid(deviceType, columnCount, rowCount, clockDivision, outd0, outa0, outd1, outa1, outd2, outa2, outd3, outa3);
}


SlideRuleGrid* SlideRuleGrid::create(aJsonObject* data)
{
	static const char rowsNodeName[] = "rows";
	static const char columnsNodeName[] = "columns";
	static const char clockNodeName[] = "externalClock";

	GridDevice device = getGridDeviceFromJSON(data);
	int rows = getIntFromJSON(data, rowsNodeName, 8, 8, 16);
	int columns = getIntFromJSON(data, columnsNodeName, 8, 8, 16);
	int division = getDivisionFromJSON(data);
	
	static const char d0NodeName[] = "d0";
	static const char d1NodeName[] = "d1";
	static const char d2NodeName[] = "d2";
	static const char d3NodeName[] = "d3";

	static const char a0NodeName[] = "a0";
	static const char a1NodeName[] = "a1";
	static const char a2NodeName[] = "a2";
	static const char a3NodeName[] = "a3";

	PinDigitalOut outd0 = getDigitalOutputFromJSON(data, d0NodeName);
	PinAnalogOut outa0 = getAnalogOutputFromJSON(data, a0NodeName);

	PinDigitalOut outd1 = getDigitalOutputFromJSON(data, d1NodeName);
	PinAnalogOut outa1 = getAnalogOutputFromJSON(data, a1NodeName);

	PinDigitalOut outd2 = getDigitalOutputFromJSON(data, d2NodeName);
	PinAnalogOut outa2 = getAnalogOutputFromJSON(data, a2NodeName);

	PinDigitalOut outd3 = getDigitalOutputFromJSON(data, d3NodeName);
	PinAnalogOut outa3 = getAnalogOutputFromJSON(data, a3NodeName);

	PinDigitalIn clockPin = getDigitalInputFromJSON(data, clockNodeName);
	
	SlideRuleGrid* grid = new SlideRuleGrid(device, columns, rows, division, outd0, outa0, outd1, outa1, outd2, outa2, outd3, outa3);
	
	if (clockPin != DIGITAL_IN_NONE)
	{
		grid->setClockInput(clockPin);
	}
			
	return grid;
}


SlideRuleGrid::SlideRuleGrid(GridDevice deviceType, uint8_t columnCount, uint8_t rowCount, int clockDivision, PinDigitalOut outd0, PinAnalogOut outa0, PinDigitalOut outd1, PinAnalogOut outa1, PinDigitalOut outd2, PinAnalogOut outa2, PinDigitalOut outd3, PinAnalogOut outa3) : USBGridController(deviceType, columnCount, rowCount)
{
	this->beat = 0;
	this->clock_division = clockDivision;
	
	if (deviceType == DEVICE_GRIDS)
	{
		this->varibright = true;
	}

	gates[0] = Gate::create(outd0, GATE_DURATION);
	gates[1] = Gate::create(outd1, GATE_DURATION);
	gates[2] = Gate::create(outd2, GATE_DURATION);
	gates[3] = Gate::create(outd3, GATE_DURATION);

	outs[0] = AnalogOut::create(outa0);
	outs[1] = AnalogOut::create(outa1);
	outs[2] = AnalogOut::create(outa2);
	outs[3] = AnalogOut::create(outa3);
	
	/* initialize the memory */
	if (!memoryInitialized)
	{
		for (uint8_t page = 0; page < 16; page++)
		{
			for (uint8_t column = 0; column < 16; column++)
			{
				for (uint8_t row = 0; row < 16; row++)
				{
					if ((page == column) && (row == 0))
					{
						this->cells[page][column][row] = 1;
					}
					else
					{
						this->cells[page][column][row] = 0;
					}
				}
			}
		}
		
		memoryInitialized = true;
	}
	
	/* Initialize the offsets and divisions */
	for (int voice = 0; voice < 4; voice++)
	{
		for (int page = 0; page < 4; page++)
		{
			for (int row = 0; row < 14; row++)
			{
				divisions[voice][page][row] = 0;
				offsets[voice][page][row] = 0;
			}
		}
	}

	/* Give it a moment... */
	delay(200);	
}


void SlideRuleGrid::timer(unsigned long t)
{
	this->gates[0]->timer(t);
	this->gates[1]->timer(t);
	this->gates[2]->timer(t);
	this->gates[3]->timer(t);

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

	/* Next page trigger is rising */
	if (this->nextPageInput != DIGITAL_IN_NONE && !this->nextPageState && digitalRead(this->nextPageInput))
	{
		this->nextPageState = t;
		this->currentPage = (this->currentPage + 1) % this->columnCount;

		this->refreshGrid();

		this->setLED(this->currentPage, this->currentPage, 0, 16);
	}

	/* Next page trigger is falling and at least 20ms after rising */
	if (this->nextPageInput != DIGITAL_IN_NONE && (this->nextPageState != 0) && ((this->nextPageState + 20) < t) && !digitalRead(this->nextPageInput))
	{
		this->nextPageState = 0;
	}
}

void SlideRuleGrid::reset()
{
	if (isReady() && this->currentPage != this->beat) this->clearLED(this->currentPage, beat, 0);

	this->beat = (this->beat + 1) % this->columnCount;

	if (isReady()) this->setLED(this->currentPage, beat, 0, 16);

	/* For each voice */
	for (uint8_t voice = 0; voice < 4; voice++)
	{
		/* Set the CV to the fractional value of sum of rows */
		uint8_t voicePage = (this->currentPage % 4) + (voice * 4);
		uint16_t cv = (5000UL * this->cells[voicePage][this->beat][1]) / 16;
		this->outs[voice]->outputCV(cv);

		/* If row 1 of the current beat column is non-zero */
		if (this->cells[voicePage][this->beat][1])
		{
			/* If the mode is probability mode */
			if (digitalRead(this->probabilityMode))
			{
				/* Take a chance and see if we fire the trigger */
				if (Entropy::getValue(this->rowCount) <= this->cells[voicePage][this->beat][1])
				{
					this->gates[voice]->reset();
				}
			}	
			else
			{
				/* Fire a trigger */
				this->gates[voice]->reset();
			}		
		}
	}
}

void SlideRuleGrid::setClockInput(PinDigitalIn input)
{
	this->clockInput = input;
}

void SlideRuleGrid::buttonPressed(uint8_t column, uint8_t row)
{		
	/* Top row is reserved for the clock - pressing sets the current page */
	if (row == 0) 
	{		
		this->switchPage(column);		
	}
	else if (row == 1)
	{
		/* Really don't want to do anything when row 1 is pressed */
	}		
	else
	{	
		uint8_t currentVoice = this->currentPage / 4;
		uint8_t currentVoicePage = this->currentPage % 4; 	
		
		if (divisions[currentVoice][currentVoicePage][row] == 0)
		{
			/* If press happened on a line with no divisions, simply increment and display witih offset */
			divisions[currentVoice][currentVoicePage][row] = (divisions[currentVoice][currentVoicePage][row] + 1) % this->columnCount;
		}
		else
		{
			/* If the press happened on a line with division */
			
			if (cells[this->currentPage][column][row])
			{
				/* If the press was on a non-zero cell, increase divisions with same offset */
				divisions[currentVoice][currentVoicePage][row] = (divisions[currentVoice][currentVoicePage][row] + 1) % this->columnCount;
			}
			else
			{
				/* If the press was on a zero cell, change offset */
				offsets[currentVoice][currentVoicePage][row] = column;
			}
		}
				
		/* Recalculate */
		if (divisions[currentVoice][currentVoicePage][row])
		{
			for (uint8_t i = 0; i < this->columnCount; i++)
			{
				//if ((this->columnCount - divisions[currentVoice][currentVoicePage][row]) % (i + offsets[currentVoice][currentVoicePage][row]) == 0)
				if (((int)i - (int)offsets[currentVoice][currentVoicePage][row]) % (this->columnCount - (divisions[currentVoice][currentVoicePage][row] - 1)) == 0)
				{
					this->cells[this->currentPage][i][row] = 8;
				}
				else
				{
					this->cells[this->currentPage][i][row] = 0;
				}
			}
		}
		else
		{
			/* Back to zero, reset */
			for (uint8_t i = 0; i < this->columnCount; i++)
			{
				this->cells[this->currentPage][i][row] = 0;
			}
		}
				
		/* Add up the values for bottom 6 rows and put on row 1 */
		for (uint8_t column = 0; column < this->columnCount; column++)
		{
			uint8_t sum = 0;
			
			for (uint8_t row = 2; row < this->rowCount; row++)
			{
				if (this->cells[this->currentPage][column][row]) sum++;
			}
			
			if (!sum)
			{
				this->cells[this->currentPage][column][1] = 0;
			}
			else
			{
				sum += 4;
				this->cells[this->currentPage][column][1] = (sum > 15) ? 15 : sum;
			}			
		}
				
		/* Redraw */	
		if (isReady()) this->refreshGrid();
	}
}

void SlideRuleGrid::buttonReleased(uint8_t column, uint8_t row)
{
	/* This guy doesn't really care */
	return;
}


