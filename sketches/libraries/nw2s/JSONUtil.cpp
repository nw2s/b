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

#include "JSONUtil.h"

using namespace nw2s;


int nw2s::getIntFromJSON(aJsonObject* data, const char* nodeName, int defaultVal, int min, int max)
{
	aJsonObject* node = aJson.getObjectItem(data, nodeName);
	
	if (node == NULL)
	{
		static const char nodeError[] = "Missing ";
		Serial.println(String(nodeError) + String(nodeName));
		return defaultVal;
	}
	
	if (node->valueint > max || node->valueint < min)
	{
		static const char nodeError[] = "Invalid ";
		Serial.println(String(nodeError) + String(nodeName));
		return defaultVal;
	}
	
	static const char info[] = ": ";
	Serial.println(String(nodeName) + String(info) + String(node->valueint));

	return node->valueint;
}

bool nw2s::getBoolFromJSON(aJsonObject* data, const char* nodeName, bool defaultVal)
{
	aJsonObject* node = aJson.getObjectItem(data, nodeName);
	
	if (node == NULL)
	{
		static const char nodeError[] = "Missing ";
		Serial.println(String(nodeError) + String(nodeName));
		return defaultVal;
	}
		
	static const char info[] = ": ";
	Serial.println(String(nodeName) + String(info) + (node->valuebool ? "true" : "false"));

	return node->valuebool;
}

PinAnalogOut nw2s::getAnalogOutputFromJSON(aJsonObject* data)
{
	static const char nodeName[] = "analogOutput";
	int val = getIntFromJSON(data, nodeName, 0, 1, 16);

	return INDEX_ANALOG_OUT[val];
	
}

PinAnalogIn nw2s::getAnalogInputFromJSON(aJsonObject* data)
{
	static const char nodeName[] = "analogInput";

	return getAnalogInputFromJSON(data, nodeName);	
}

PinAnalogIn nw2s::getAnalogInputFromJSON(aJsonObject* data, const char* nodeName)
{
	int val = getIntFromJSON(data, nodeName, 0, 1, 12);

	return INDEX_ANALOG_IN[val];	
}

PinDigitalIn nw2s::getDigitalInputFromJSON(aJsonObject* data, const char* nodeName)
{
	int val = getIntFromJSON(data, nodeName, 0, 1, 8);

	return INDEX_DIGITAL_IN[val];	
}

PinDigitalOut nw2s::getDigitalOutputFromJSON(aJsonObject* data, const char* nodeName)
{
	int val = getIntFromJSON(data, nodeName, 0, 1, 16);

	return INDEX_DIGITAL_OUT[val];	
}

PinAudioOut nw2s::getAudioOutputFromJSON(aJsonObject* data)
{
	static const char nodeName[] = "dacOutput";
	int val = getIntFromJSON(data, nodeName, 1, 1, 2);

	return val == 1 ? DUE_DAC0 : DUE_DAC1;
}


Scale nw2s::getScaleFromJSON(aJsonObject* data)
{	
	aJsonObject* scaleNode = aJson.getObjectItem(data, "scale");

	if (scaleNode == NULL)
	{
		static const char nodeError[] = "Missing scale definition. Using chromatic.";
		Serial.println(String(nodeError));
		return Key::SCALE_CHROMATIC;
	}

	static const char info[] = "Scale: ";
	Serial.println(String(info) + String(scaleNode->valuestring));

	return scaleFromName(scaleNode->valuestring);
}

NoteName nw2s::getRootFromJSON(aJsonObject* data)
{
	aJsonObject* rootNode = aJson.getObjectItem(data, "root");

	if (rootNode == NULL)
	{
		static const char nodeError[] = "Missing root definition. Using C";
		Serial.println(String(nodeError));
		return C;
	}

	static const char info[] = "Scale Root: ";
	Serial.println(String(info) + String(rootNode->valuestring));

	return noteFromName(rootNode->valuestring);
}

int nw2s::getDivisionFromJSON(aJsonObject* data)
{
	aJsonObject* divisionNode = aJson.getObjectItem(data, "division");

	if (divisionNode == NULL)
	{
		static const char nodeError[] = "Missing division definition. Assume quarter note";
		Serial.println(String(nodeError));
		return DIV_QUARTER;
	}

	static const char info[] = "Clock Division: ";
	Serial.println(String(info) + String(divisionNode->valuestring));

	return clockDivisionFromName(divisionNode->valuestring);
}

NoteSequenceData* nw2s::getNotesFromJSON(aJsonObject* data)
{
	aJsonObject* notesNode = aJson.getObjectItem(data, "notes");
	
	if (notesNode == NULL)
	{
		static const char nodeError[] = "The NoteSequencer node is missing a notes definition.";
		Serial.println(String(nodeError));
		return NULL;
	}

	NoteSequenceData* notes = noteSequenceFromJSON(notesNode);	
}

std::vector<int>* nw2s::getIntCollectionFromJSON(aJsonObject* data, const char* nodeName)
{
	aJsonObject* valuesNode = aJson.getObjectItem(data, nodeName);
	
	if (valuesNode == NULL) return NULL;

	/* Allocate enough space to store our vector */
	std::vector<int>* values = new std::vector<int>(aJson.getArraySize(valuesNode));
	
	/* Iterate over the set of values in the sequence */
	for (int i = 0; i < aJson.getArraySize(valuesNode); i++)
	{
		aJsonObject* valueNode = aJson.getArrayItem(valuesNode, i);
				
		(*values)[i] = valueNode->valueint;
	}
				
	return values;
}

