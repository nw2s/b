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

#include <Arduino.h>
#include "Audiodevice.h"

void TC4_Handler()
{
	// We need to get the status to clear it and allow the interrupt to fire again
	TC_GetStatus(TC1, 1);

	nw2s::AudioDevice::device0->timer_handler();
}

void TC5_Handler()
{
	// We need to get the status to clear it and allow the interrupt to fire again
	TC_GetStatus(TC1, 2);

	nw2s::AudioDevice::device1->timer_handler();
}
