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


PinAnalogOut nw2s::getAnalogOutputFromJSON(aJsonObject* data)
{
	aJsonObject* outputNode = aJson.getObjectItem(data, "analogOutput");
	
	if (outputNode == NULL)
	{
		static const char nodeError[] = "Missing analogOutput";
		Serial.println(String(nodeError));
		return ANALOG_OUT_NONE;
	}
	
	if (outputNode->valueint > 16 || outputNode->valueint < 1)
	{
		static const char nodeError[] = "Invalid analogOutput";
		Serial.println(String(nodeError));
		return ANALOG_OUT_NONE;
	}
	
	static const char info[] = "CV Output: Analog Out ";
	Serial.println(String(info) + String(outputNode->valueint));

	return INDEX_ANALOG_OUT[outputNode->valueint - 1];
	
}

ScaleType nw2s::getScaleFromJSON(aJsonObject* data)
{	
	aJsonObject* scaleNode = aJson.getObjectItem(data, "scale");

	if (scaleNode == NULL)
	{
		static const char nodeError[] = "Missing scale definition. Using chromatic.";
		Serial.println(String(nodeError));
		return CHROMATIC;
	}

	static const char info[] = "Scale: ";
	Serial.println(String(info) + String(scaleNode->valuestring));

	return scaleTypeFromName(scaleNode->valuestring);
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



