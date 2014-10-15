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

#include "GridNoteSequencer.h"

#define GATE_DURATION 50

using namespace nw2s;


GridNoteSequencer* GridNoteSequencer::create(GridDevice deviceType, uint8_t columnCount, uint8_t rowCount, int clockDivision, NoteName key, Scale scale, PinDigitalOut outd0, PinAnalogOut outa0, int notes0[][2], PinDigitalOut outd1, PinAnalogOut outa1, int notes1[][2], PinDigitalOut outd2, PinAnalogOut outa2, int notes2[][2], PinDigitalOut outd3, PinAnalogOut outa3, int notes3[][2])
{
	return new GridNoteSequencer(deviceType, columnCount, rowCount, clockDivision, key, scale, outd0, outa0, notes0, outd1, outa1, notes1, outd2, outa2, notes2, outd3, outa3, notes3);
}

GridNoteSequencer::GridNoteSequencer(GridDevice deviceType, uint8_t columnCount, uint8_t rowCount, int clockDivision, NoteName key, Scale scale, PinDigitalOut outd0, PinAnalogOut outa0, int notes0[][2], PinDigitalOut outd1, PinAnalogOut outa1, int notes1[][2], PinDigitalOut outd2, PinAnalogOut outa2, int notes2[][2], PinDigitalOut outd3, PinAnalogOut outa3, int notes3[][2]) : USBGridController(deviceType, columnCount, rowCount)
{
	this->key = new Key(scale, key);
	this->beat = 0;
	this->clock_division = clockDivision;

	if (outd0 != DIGITAL_IN_NONE) gates[0] = Gate::create(outd0, GATE_DURATION);
	if (outd1 != DIGITAL_IN_NONE) gates[1] = Gate::create(outd1, GATE_DURATION);
	if (outd2 != DIGITAL_IN_NONE) gates[2] = Gate::create(outd2, GATE_DURATION);
	if (outd3 != DIGITAL_IN_NONE) gates[3] = Gate::create(outd3, GATE_DURATION);

	if (outa0 != ANALOG_IN_NONE) outs[0] = AnalogOut::create(outa0);
	if (outa1 != ANALOG_IN_NONE) outs[1] = AnalogOut::create(outa1);
	if (outa2 != ANALOG_IN_NONE) outs[2] = AnalogOut::create(outa2);
	if (outa3 != ANALOG_IN_NONE) outs[3] = AnalogOut::create(outa3);
	
	/* Copy the notes to their final resting place */
	if (outa0 != ANALOG_OUT_NONE)
	{
		for (uint8_t row = 0; row < this->rowCount; row++)
		{
			this->notes[0][row][0] = notes0[row][0];
			this->notes[0][row][1] = notes0[row][1];
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
		for (uint8_t row = 0; row < this->rowCount; row++)
		{
			this->notes[2][row][0] = notes2[row][0];
			this->notes[2][row][1] = notes2[row][1];
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


void GridNoteSequencer::timer(unsigned long t)
{
	// for (uint8_t i = 1; i < 8; i++)
	// {
	// 	this->gates[i]->timer(t);
	// }
	//
	// /* Clock is rising */
	// if (this->clockInput != DIGITAL_IN_NONE && !this->clockState && digitalRead(this->clockInput))
	// {
	// 	this->clockState = t;
	//
	// 	/* Simulate a clock pulse */
	// 	this->reset();
	// }
	//
	// /* Clock is falling and at least 20ms after rising */
	// if (this->clockInput != DIGITAL_IN_NONE && (this->clockState != 0) && ((this->clockState + 20) < t) && !digitalRead(this->clockInput))
	// {
	// 	this->clockState = 0;
	// }
	//
	// /* Shuffle input signal rising */
	// if (this->shuffleInput != DIGITAL_IN_NONE && !this->shuffleState && digitalRead(this->shuffleInput))
	// {
	// 	this->shuffleState = true;
	//
	// 	if (!digitalRead(this->shuffleScopeInput))
	// 	{
	// 		if (this->lastpress[1] != 0)
	// 		{
	// 			this->shuffleRow(this->lastpress[1]);
	// 		}
	// 	}
	// 	else
	// 	{
	// 		for (uint8_t i = 1; i < rowCount; i++)
	// 		{
	// 			this->shuffleRow(i);
	// 		}
	// 	}
	//
	// 	this->refreshGrid();
	//
	// 	this->setLED(this->currentPage, this->currentPage, 0, 1);
	// }
	//
	// /* Next page trigger is rising */
	// if (this->nextPageInput != DIGITAL_IN_NONE && !this->nextPageState && digitalRead(this->nextPageInput))
	// {
	// 	this->nextPageState = t;
	// 	this->currentPage = (this->currentPage + 1) % this->columnCount;
	//
	// 	this->refreshGrid();
	//
	// 	this->setLED(this->currentPage, this->currentPage, 0, 1);
	// }
	//
	// /* Next page trigger is falling and at least 20ms after rising */
	// if (this->nextPageInput != DIGITAL_IN_NONE && (this->nextPageState != 0) && ((this->nextPageState + 20) < t) && !digitalRead(this->nextPageInput))
	// {
	// 	this->nextPageState = 0;
	// }	
}

void GridNoteSequencer::reset()
{
	if (isReady() && this->currentPage != this->beat) this->clearLED(this->currentPage, beat, 0);

	beat = (beat + 1) % columnCount;

	if (isReady()) this->setLED(this->currentPage, beat, 0, 1);

	// for (uint8_t i = 1; i < 8; i++)
	// {
	// 	if (getValue(this->currentPage, beat, i))
	// 	{
	// 		this->gates[i]->reset();
	// 	}
	// }
	//
	// if (!digitalRead(this->shuffleInput))
	// {
	// 	this->shuffleState = false;
	// }
}

void GridNoteSequencer::buttonPressed(uint8_t column, uint8_t row)
{		
	/* Top row is reserved for the clock - pressing sets the current page */
	if (row == 0) 
	{		
		this->switchPage(column);		
	}	
	else
	{
		for (uint8_t rowIterator = 0; rowIterator < rowCount; rowIterator++)
		{
			if (rowIterator == row)
			{
				this->cells[this->currentPage][column][row] = 1;
			}
			else
			{
				this->cells[this->currentPage][column][row] = 0;
			}
		}
		
		if (isReady()) this->refreshGrid();
	}
}

void GridNoteSequencer::buttonReleased(uint8_t column, uint8_t row)
{
	/* This guy doesn't really care */
	return;
}


