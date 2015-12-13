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

#include "GridOto.h"
#include "JSONUtil.h"

#define GATE_DURATION 50

using namespace nw2s;


GridOto* GridOto::create(GridDevice deviceType, uint8_t columnCount, uint8_t rowCount, int clockDivision, NoteName key, Scale scale, PinDigitalOut outd0, PinAnalogOut outa0, int notes0[][2], PinDigitalOut outd1, PinAnalogOut outa1, int notes1[][2], PinDigitalOut outd2, PinAnalogOut outa2, int notes2[][2], PinDigitalOut outd3, PinAnalogOut outa3, int notes3[][2])
{
	return new GridOto(deviceType, columnCount, rowCount, clockDivision, key, scale, outd0, outa0, notes0, outd1, outa1, notes1, outd2, outa2, notes2, outd3, outa3, notes3);
}

GridOto* GridOto::create(aJsonObject* data)
{
	static const char rowsNodeName[] = "rows";
	static const char columnsNodeName[] = "columns";
	static const char clockNodeName[] = "externalClock";

	static const char d0NodeName[] = "d0";
	static const char d1NodeName[] = "d1";
	static const char d2NodeName[] = "d2";
	static const char d3NodeName[] = "d3";

	static const char a0NodeName[] = "a0";
	static const char a1NodeName[] = "a1";
	static const char a2NodeName[] = "a2";
	static const char a3NodeName[] = "a3";

	static const char notes0NodeName[] = "notes0";
	static const char notes1NodeName[] = "notes1";
	static const char notes2NodeName[] = "notes2";
	static const char notes3NodeName[] = "notes3";


	GridDevice device = getGridDeviceFromJSON(data);
	const int rows = getIntFromJSON(data, rowsNodeName, 8, 8, 16);
	const int columns = getIntFromJSON(data, columnsNodeName, 8, 8, 16);
	int division = getDivisionFromJSON(data);
	NoteName key = getRootFromJSON(data);
	Scale scale = getScaleFromJSON(data);
	
	PinDigitalOut outd0 = getDigitalOutputFromJSON(data, d0NodeName);
	PinAnalogOut outa0 = getAnalogOutputFromJSON(data, a0NodeName);
	NoteSequenceData* notes0 = getNotesFromJSON(data, notes0NodeName);

	PinDigitalOut outd1 = getDigitalOutputFromJSON(data, d1NodeName);
	PinAnalogOut outa1 = getAnalogOutputFromJSON(data, a1NodeName);
	NoteSequenceData* notes1 = getNotesFromJSON(data, notes1NodeName);

	PinDigitalOut outd2 = getDigitalOutputFromJSON(data, d2NodeName);
	PinAnalogOut outa2 = getAnalogOutputFromJSON(data, a2NodeName);
	NoteSequenceData* notes2 = getNotesFromJSON(data, notes2NodeName);

	PinDigitalOut outd3 = getDigitalOutputFromJSON(data, d3NodeName);
	PinAnalogOut outa3 = getAnalogOutputFromJSON(data, a3NodeName);
	NoteSequenceData* notes3 = getNotesFromJSON(data, notes3NodeName);

	int notelist0[columns][2];
	int notelist1[rows][2];
	int notelist2[columns][2];
	int notelist3[rows][2];

	for (int i = 0; i < columns; i++)
	{
		notelist0[i][0] = (*notes0)[i].octave;
		notelist0[i][1] = (*notes0)[i].degree;
	}

	for (int i = 0; i < rows; i++)
	{
		notelist1[i][0] = (*notes1)[i].octave;
		notelist1[i][1] = (*notes1)[i].degree;
	}

	for (int i = 0; i < columns; i++)
	{
		notelist2[i][0] = (*notes2)[i].octave;
		notelist2[i][1] = (*notes2)[i].degree;
	}

	for (int i = 0; i < rows; i++)
	{
		notelist3[i][0] = (*notes3)[i].octave;
		notelist3[i][1] = (*notes3)[i].degree;
	}


	PinDigitalIn clockPin = getDigitalInputFromJSON(data, clockNodeName);
	
	GridOto* grid = new GridOto(device, columns, rows, division, key, scale, outd0, outa0, notelist0, outd1, outa1, notelist1, outd2, outa2, notelist2, outd3, outa3, notelist3);
		
	if (clockPin != DIGITAL_IN_NONE)
	{
		grid->setClockInput(clockPin);
	}
			
	return grid;
}


GridOto::GridOto(GridDevice deviceType, uint8_t columnCount, uint8_t rowCount, int clockDivision, NoteName key, Scale scale, PinDigitalOut outd0, PinAnalogOut outa0, int notes0[][2], PinDigitalOut outd1, PinAnalogOut outa1, int notes1[][2], PinDigitalOut outd2, PinAnalogOut outa2, int notes2[][2], PinDigitalOut outd3, PinAnalogOut outa3, int notes3[][2]) : USBGridController(deviceType, columnCount, rowCount)
{
	this->key = new Key(scale, key);
	this->beat = 0;
	this->clock_division = clockDivision;
	
	this->maxcells = columnCount + rowCount;
	
	/* Loop through and initialize all of the otocells */
	for (uint8_t i = 0; i < this->maxcells; i++)
	{
		this->otocells[i] = { false, 0, 0, CELL_UP };
	}

	gates[0] = Gate::create(outd0, GATE_DURATION);
	gates[1] = Gate::create(outd1, GATE_DURATION);
	gates[2] = Gate::create(outd2, GATE_DURATION);
	gates[3] = Gate::create(outd3, GATE_DURATION);

	outs[0] = AnalogOut::create(outa0);
	outs[1] = AnalogOut::create(outa1);
	outs[2] = AnalogOut::create(outa2);
	outs[3] = AnalogOut::create(outa3);
	
	/* Copy the notes to their final resting place */
	if (outa0 != ANALOG_OUT_NONE)
	{
		for (uint8_t column = 0; column < this->columnCount; column++)
		{
			this->notes[0][column][0] = notes0[column][0];
			this->notes[0][column][1] = notes0[column][1];
		}
	}

	if (outa1 != ANALOG_OUT_NONE)
	{
		for (uint8_t row = 0; row < this->rowCount; row++)
		{
			this->notes[1][row][0] = notes1[row][0];
			this->notes[1][row][1] = notes1[row][1];
		}
	}

	if (outa2 != ANALOG_OUT_NONE)
	{
		for (uint8_t column = 0; column < this->columnCount; column++)
		{
			this->notes[2][column][0] = notes2[column][0];
			this->notes[2][column][1] = notes2[column][1];
		}
	}

	if (outa3 != ANALOG_OUT_NONE)
	{
		for (uint8_t row = 0; row < this->rowCount; row++)
		{
			this->notes[3][row][0] = notes3[row][0];
			this->notes[3][row][1] = notes3[row][1];
		}
	}

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

	/* Give it a moment... */
	delay(200);		
}

void GridOto::setClockInput(PinDigitalIn input)
{
	this->clockInput = input;
}

void GridOto::timer(unsigned long t)
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
}

void GridOto::reset()
{
	beat = (beat + 1) % columnCount;

	OtoCell topevent = { false, 0, 0, CELL_UP };
	OtoCell bottomevent = { false, 0, 0, CELL_UP };
	OtoCell rightevent = { false, 0, 0, CELL_UP };
	OtoCell leftevent = { false, 0, 0, CELL_UP };

	/* Clear the matrix so we can set the new positions */
	for (uint8_t column = 0; column < this->columnCount; column++)
	{
		for (uint8_t row = 0; row < this->rowCount; row++)
		{
			this->cells[0][column][row] = 0;
		}
	}

	/* For each cell, see if it's active and move it in it's direction */
	for (uint8_t cellIndex = 0; cellIndex < this->maxcells; cellIndex++)
	{
		if (otocells[cellIndex].isActive)
		{
			switch (otocells[cellIndex].direction)
			{
				case CELL_UP:
				
					otocells[cellIndex].row--;
					
					if (otocells[cellIndex].row == 0)
					{
						otocells[cellIndex].direction = CELL_DOWN;
						topevent = otocells[cellIndex];
					}
					
					break;
				
				case CELL_RIGHT:
				
					otocells[cellIndex].column++;
				
					if (otocells[cellIndex].column == this->columnCount - 1) 
					{
						otocells[cellIndex].direction = CELL_LEFT;
						rightevent = otocells[cellIndex];
					}
					
					break;
				
				case CELL_DOWN:
				
					otocells[cellIndex].row++;
				
					if (otocells[cellIndex].row == this->rowCount - 1) 
					{
						otocells[cellIndex].direction = CELL_UP;
						bottomevent = otocells[cellIndex];
					}
						
					break;
				
				case CELL_LEFT:

					otocells[cellIndex].column--;
			
					if (otocells[cellIndex].column == 0)
					{
						otocells[cellIndex].direction = CELL_RIGHT;
						leftevent = otocells[cellIndex];
					}
						
					break;
			}
			
			/* Update the matrix display to show the new positions */
			this->cells[0][otocells[cellIndex].column][otocells[cellIndex].row] = 1;
		}
	}
	
	/* For each cell, see if any are on top of each other and rotate them clockwise */
	for (uint8_t cellIndex = 0; cellIndex < this->maxcells; cellIndex++)
	{
		if (otocells[cellIndex].isActive)
		{
			for (uint8_t cellIndex2 = 0; cellIndex2 < this->maxcells; cellIndex2++)
			{
				if ((cellIndex2 != cellIndex) && (otocells[cellIndex2].isActive))
				{
					if ((otocells[cellIndex].column == otocells[cellIndex2].column) && (otocells[cellIndex].row == otocells[cellIndex2].row))
					{
						if (otocells[cellIndex].direction == CELL_UP) otocells[cellIndex].direction = CELL_RIGHT;
						else if (otocells[cellIndex].direction == CELL_RIGHT) otocells[cellIndex].direction = CELL_DOWN;
						else if (otocells[cellIndex].direction == CELL_DOWN) otocells[cellIndex].direction = CELL_LEFT;
						else if (otocells[cellIndex].direction == CELL_LEFT) otocells[cellIndex].direction = CELL_UP;
					}
				}
			}
		}
	}
	
	if (topevent.isActive)
	{
		if (this->notes[0] != NULL) this->outs[0]->outputCV(this->key->getNoteMillivolt(this->notes[0][topevent.column][0], this->notes[0][topevent.column][1]));
		gates[0]->reset();
	}	

	if (rightevent.isActive)
	{
		if (this->notes[1] != NULL) this->outs[1]->outputCV(this->key->getNoteMillivolt(this->notes[1][rightevent.row][0],this->notes[1][rightevent.row][1]));
		gates[1]->reset();
	}	

	if (bottomevent.isActive)
	{
		if (this->notes[2] != NULL) this->outs[2]->outputCV(this->key->getNoteMillivolt(this->notes[2][bottomevent.column][0],this->notes[2][bottomevent.column][1]));
		gates[2]->reset();
	}	

	if (leftevent.isActive)
	{
		if (this->notes[3] != NULL) this->outs[3]->outputCV(this->key->getNoteMillivolt(this->notes[3][leftevent.row][0],this->notes[3][leftevent.row][1]));
		gates[3]->reset();
	}	


	if (isReady()) this->refreshGrid();
}

void GridOto::buttonPressed(uint8_t column, uint8_t row)
{		
	/* If the button was exactly on one of the active cells, clear it */
	for (uint8_t cellid = 0; cellid < this->maxcells; cellid++)
	{
		if (this->otocells[cellid].isActive && this->otocells[cellid].row == row && this->otocells[cellid].column == column)
		{
			this->otocells[cellid].isActive = false;
			return;
		}
	}
	
	/* If not, create a new active cell at this position */	
	/* Find the first inactive cell and use that place */
	for (uint8_t cellid = 0; cellid < this->maxcells; cellid++)
	{
		if (!this->otocells[cellid].isActive)
		{
			this->otocells[cellid].isActive = true;
			this->otocells[cellid].column = column;
			this->otocells[cellid].row = row;
			
			this->cells[0][column][row] = 1;
			
			break;
		}
	}
	
	/* If there were no inactive spots, then just ignore it */
	
	if (isReady()) this->refreshGrid();
}

void GridOto::buttonReleased(uint8_t column, uint8_t row)
{
	/* This guy doesn't really care */
	return;
}


