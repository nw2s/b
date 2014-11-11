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

#include <Key.h>
#include <EventManager.h>
#include <IO.h>
#include <Trigger.h>
#include <Oscillator.h>
#include <Clock.h>
#include <Slew.h>
#include <Sequence.h>
#include <SPI.h>
#include <SD.h>
#include <Wire.h>
#include <aJSON.h>
#include <Gate.h>
#include <Grid.h>
#include <GameOfLife.h>
#include <Usb.h>

using namespace nw2s;

GameOfLife* grid;

void setup() 
{
    Serial.begin(19200);
  
    EventManager::initialize();
  
    grid = GameOfLife::create(DEVICE_GRIDS, 16, 8, true);
  
    /* Setup a variable clock */
    // Clock* vclock = VariableClock::create(10, 240, DUE_IN_A00, 16);
    // vclock->registerDevice(grid);
    // EventManager::registerDevice(vclock);

    /* This tells the device that you want to clock it externally */
    grid->setClockInput(DUE_IN_D0);
    EventManager::registerDevice(grid);

    /* This allows the event manager to manage USB events */
    EventManager::registerUsbDevice(grid);
}

void loop()
{
    EventManager::loop();
    delay(1);
}





