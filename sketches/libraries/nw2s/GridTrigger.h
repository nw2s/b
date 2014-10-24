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


namespace nw2s
{
	class GridTriggerController;
}


class nw2s::GridTriggerController : public BeatDevice, public USBGridController
{
	public:
		
		static GridTriggerController* create(GridDevice deviceType, uint8_t columnCount, uint8_t rowCount, int clockDivision, PinDigitalOut out0, PinDigitalOut out1, PinDigitalOut out2, PinDigitalOut out3, PinDigitalOut out4, PinDigitalOut out5, PinDigitalOut out6);
		static GridTriggerController* create(aJsonObject* data);

		virtual void timer(unsigned long t);
		virtual void reset();
		
		void setProbabilityInput(PinAnalogIn input);
		void setShuffleToggle(PinDigitalIn input);
		void setNextPageToggle(PinDigitalIn input);
		void setResetPageToggle(PinDigitalIn input);	
		void setShuffleScopeInput(PinDigitalIn input);	
		void setClockInput(PinDigitalIn input);	

	protected:
		
		virtual void buttonPressed(uint8_t column, uint8_t row);
		virtual void buttonReleased(uint8_t column, uint8_t row);
		
	private:
		
		GridTriggerController(GridDevice deviceType, uint8_t columnCount, uint8_t rowCount, int clockDivision, PinDigitalOut out0, PinDigitalOut out1, PinDigitalOut out2, PinDigitalOut out3, PinDigitalOut out4, PinDigitalOut out5, PinDigitalOut out6);
		
		void shuffleRow(uint8_t rowIndex);

		Gate* gates[8];	
		bool shuffleState = false;
		bool resetPageState = false;
		unsigned long clockState = 0;
		unsigned long nextPageState = 0;
		
		PinAnalogIn probabilityInput = ANALOG_IN_NONE;
		PinDigitalIn shuffleInput = DIGITAL_IN_NONE;
		PinDigitalIn shuffleScopeInput = DIGITAL_IN_NONE;
		PinDigitalIn resetPageInput = DIGITAL_IN_NONE;
		PinDigitalIn nextPageInput = DIGITAL_IN_NONE;
		PinDigitalIn clockInput = DIGITAL_IN_NONE;
		
};