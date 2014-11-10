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
	class SlideRuleGrid;
}

class nw2s::SlideRuleGrid : public BeatDevice, public USBGridController
{
	public:
		
		static SlideRuleGrid* create(GridDevice deviceType, uint8_t columnCount, uint8_t rowCount, int clockDivision, PinDigitalOut outd0, PinAnalogOut outa0, PinDigitalOut outd1, PinAnalogOut outa1, PinDigitalOut outd2, PinAnalogOut outa2, PinDigitalOut outd3, PinAnalogOut outa3);
		static SlideRuleGrid* create(aJsonObject* data);

		virtual void timer(unsigned long t);
		virtual void reset();
		
		void setNextPageToggle(PinDigitalIn input);
		void setClockInput(PinDigitalIn input);	

	protected:
		
		virtual void buttonPressed(uint8_t column, uint8_t row);
		virtual void buttonReleased(uint8_t column, uint8_t row);
		
	private:
		
		SlideRuleGrid(GridDevice deviceType, uint8_t columnCount, uint8_t rowCount, int clockDivision, PinDigitalOut outd0, PinAnalogOut outa0, PinDigitalOut outd1, PinAnalogOut outa1, PinDigitalOut outd2, PinAnalogOut outa2, PinDigitalOut outd3, PinAnalogOut outa3);
		
		Gate* gates[4];	
		AnalogOut* outs[4];
		uint8_t divisions[4][4][14];
 		uint8_t offsets[4][4][14];

		unsigned long clockState = 0;
		unsigned long nextPageState = 0;
		
		PinDigitalIn clockInput = DIGITAL_IN_NONE;
		PinDigitalIn nextPageInput = DIGITAL_IN_NONE;
		PinDigitalIn probabilityMode = DIGITAL_IN_NONE;
		
};