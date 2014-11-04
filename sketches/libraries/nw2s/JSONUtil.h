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

#ifndef JSONUtil_h
#define JSONUtil_h

#include "Grid.h"
#include "IO.h"
#include "Key.h"
#include "Sequence.h"
#include "Loop.h"
#include "../aJSON/aJSON.h"

namespace nw2s
{	
	/* 
		These are some utility functions to encapsulate navigating the 'b JSON format. 
	   	The goal is to put functions here to avoid having multiple copies of the same
		getters and error messages throughout the code. 
	*/
	
	PinAudioOut getAudioOutputFromJSON(aJsonObject* data);
	PinAnalogOut getAnalogOutputFromJSON(aJsonObject* data, const char* nodeName);
	PinAnalogOut getAnalogOutputFromJSON(aJsonObject* data);
	PinAnalogIn getAnalogInputFromJSON(aJsonObject* data);
	PinAnalogIn getAnalogInputFromJSON(aJsonObject* data, const char* nodeName);
	PinDigitalOut getDigitalOutputFromJSON(aJsonObject* data, const char* nodeName);
	PinDigitalIn getDigitalInputFromJSON(aJsonObject* data, const char* nodeName);
	GridDevice getGridDeviceFromJSON(aJsonObject* data);
	NoteName getRootFromJSON(aJsonObject* data);
	Scale getScaleFromJSON(aJsonObject* data);
	int getDivisionFromJSON(aJsonObject* data);
	NoteSequenceData* getNotesFromJSON(aJsonObject* data);
	int getTempoFromJSON(aJsonObject* data);
	int getBeatsFromJSON(aJsonObject* data);
	char* getNameFromJSON(aJsonObject* data);
	std::vector<int>* getIntCollectionFromJSON(aJsonObject* data, const char* nodeName);
	SampleRateInterrupt getSampleRateFromJSON(aJsonObject* data);

	aJsonObject* getClockFromJSON(aJsonObject* data);
	aJsonObject* getProgramFromJSON(aJsonObject* data);
	aJsonObject* getDevicesFromJSON(aJsonObject* data);	
	
	int getIntFromJSON(aJsonObject* data, const char* nodeName, int defaultVal, int min, int max);
	bool getBoolFromJSON(aJsonObject* data, const char* nodeName, bool defaultVal);
	char* getStringFromJSON(aJsonObject* data, const char* nodeName);
}

#endif