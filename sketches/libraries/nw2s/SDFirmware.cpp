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

#include <EventManager.h>
#include <Key.h>
#include <Trigger.h>
#include <Clock.h>
#include <Slew.h>
#include <Oscillator.h>
#include <Sequence.h>
#include <IO.h>
#include <SPI.h>
#include <Wire.h>
#include <SD.h>
#include <ShiftRegister.h>
#include <Loop.h>
#include <aJSON.h>
#include "b.h"
#include "../aJSON/aJSON.h"
#include "SDFirmware.h"

using namespace nw2s;

void nw2s::initializeFirmware()
{
	SdFile root;
	SdFile programsDir;
	SdFile programFile;


	/* Ensure the proper structure and locate the program definition file */
	/* Ensure the proper structure and locate the program definition file */
	/* Ensure the proper structure and locate the program definition file */

	root = b::getSDRoot();
	
	if (!programsDir.open(root, "PROGRAMS", O_READ))
	{
	    Serial.println("Error opening programs folder. Are you sure it's there?");
	    return;
	}
	
	if (!programFile.open(programsDir, "DEFAULT.B", O_READ))
	{
	    Serial.println("Error opening default program. Are you sure it's there?");
	    return;
	}

	/* Load and parse the program definition file */
	/* Load and parse the program definition file */
	/* Load and parse the program definition file */

	uint fileSize = programFile.fileSize();
	char programData[fileSize + 1];

	programFile.read(programData, fileSize);
	programData[fileSize] == '\0';
		
	aJsonObject* program = aJson.parse(programData);

    if (program == NULL) 
	{
		static const char error[] = "Program not parsed successfully. Check to see that it's properly formatted JSON."; 
        Serial.println(error);
		return;
	}
	else
	{
		static const char msg[] = "Program parsed successfully.";
        Serial.println(msg);
	}

	aJsonObject* programNode = aJson.getObjectItem(program, "program"); 

	if (programNode == NULL)
	{
		static const char msg[] = "Program not parsed successfully. 'program' node not found.";
        Serial.println(msg);
		return;
	}

	/* If there is a clock, keep track of it for beatdevices */
	/* If there is a clock, keep track of it for beatdevices */
	/* If there is a clock, keep track of it for beatdevices */

	Clock* clockDevice = NULL;
	aJsonObject* clockNode = aJson.getObjectItem(programNode, "clock");
	
	if (clockNode == NULL)
	{
		Serial.println("No clock defined. If you have any beat devices, they won't have a clock to run on");
	}
	else
	{
		aJsonObject* clockTypeNode = aJson.getObjectItem(clockNode, "type");
		Serial.println("Clock: " + String(clockTypeNode->valuestring));
		
		if (strcmp(clockTypeNode->valuestring, "FixedClock") == 0)
		{
			clockDevice = FixedClock::create(clockNode);
			EventManager::registerDevice(clockDevice);
		}		
		else if (strcmp(clockTypeNode->valuestring, "VariableClock") == 0)
		{
			clockDevice = VariableClock::create(clockNode);
			EventManager::registerDevice(clockDevice);
		}		
		else if (strcmp(clockTypeNode->valuestring, "RandomTempoClock") == 0)
		{
			clockDevice = RandomTempoClock::create(clockNode);
			EventManager::registerDevice(clockDevice);
		}		
	}
	

	/* Iterate over the devices and initialize them */
	/* Iterate over the devices and initialize them */
	/* Iterate over the devices and initialize them */

	aJsonObject* deviceNodes = aJson.getObjectItem(programNode, "devices");
	
	for (int i = 0; i < aJson.getArraySize(deviceNodes); i++)
	{
		aJsonObject* deviceNode = aJson.getArrayItem(deviceNodes, i);
		aJsonObject* typeNode = aJson.getObjectItem(deviceNode, "type");
		Serial.println("Device " + String(i + 1) + ": " + typeNode->valuestring);
		
		if (strcmp(typeNode->valuestring, "DiscreteNoise") == 0)
		{
			EventManager::registerDevice(DiscreteNoise::create(deviceNode));
		}
		else if (strcmp(typeNode->valuestring, "ByteBeat") == 0)
		{
			EventManager::registerDevice(ByteBeat::create(deviceNode));
		}
		else if (strcmp(typeNode->valuestring, "CVNoteSequencer") == 0)
		{
			EventManager::registerDevice(CVNoteSequencer::create(deviceNode));
		}
		else if (strcmp(typeNode->valuestring, "CVSequencer") == 0)
		{
			if (clockDevice != NULL)
			{
				clockDevice->registerDevice(CVSequencer::create(deviceNode));
			}
			else
			{
				static const char nodeError[] = "CVSequencer defined with no clock, skipping.";
				Serial.println(String(nodeError));
			}
		}
		else if (strcmp(typeNode->valuestring, "DrumTriggerSequencer") == 0)
		{
			if (clockDevice != NULL)
			{
				clockDevice->registerDevice(DrumTriggerSequencer::create(deviceNode));
			}
			else
			{
				static const char nodeError[] = "DrumTriggerSequencer defined with no clock, skipping.";
				Serial.println(String(nodeError));
			}
		}
		else if (strcmp(typeNode->valuestring, "EFLooper") == 0)
		{
			EventManager::registerDevice(EFLooper::create(deviceNode));
		}
		else if (strcmp(typeNode->valuestring, "Looper") == 0)
		{
			EventManager::registerDevice(Looper::create(deviceNode));
		}
		else if (strcmp(typeNode->valuestring, "MorphingNoteSequencer") == 0)
		{
			if (clockDevice != NULL)
			{
				clockDevice->registerDevice(MorphingNoteSequencer::create(deviceNode));
			}
			else
			{
				static const char nodeError[] = "MorphingNoteSequencer defined with no clock, skipping.";
				Serial.println(String(nodeError));
			}
		}
		else if (strcmp(typeNode->valuestring, "NoteSequencer") == 0)
		{
			if (clockDevice != NULL)
			{
				clockDevice->registerDevice(NoteSequencer::create(deviceNode));
			}
			else
			{
				static const char nodeError[] = "NoteSequencer defined with no clock, skipping.";
				Serial.println(String(nodeError));
			}
		}
		else if (strcmp(typeNode->valuestring, "ProbabilityDrumTriggerSequencer") == 0)
		{
			if (clockDevice != NULL)
			{
				clockDevice->registerDevice(ProbabilityDrumTriggerSequencer::create(deviceNode));
			}
			else
			{
				static const char nodeError[] = "ProbabilityDrumTriggerSequencer defined with no clock, skipping.";
				Serial.println(String(nodeError));
			}
		}
		else if (strcmp(typeNode->valuestring, "ProbabilityTriggerSequencer") == 0)
		{
			if (clockDevice != NULL)
			{
				clockDevice->registerDevice(ProbabilityTriggerSequencer::create(deviceNode));
			}
			else
			{
				static const char nodeError[] = "ProbabilityTriggerSequencer defined with no clock, skipping.";
				Serial.println(String(nodeError));
			}
		}
		else if (strcmp(typeNode->valuestring, "TriggeredNoteSequencer") == 0)
		{
			EventManager::registerDevice(TriggeredNoteSequencer::create(deviceNode));
		}
		else if (strcmp(typeNode->valuestring, "TriggerSequencer") == 0)
		{
			if (clockDevice != NULL)
			{
				clockDevice->registerDevice(TriggerSequencer::create(deviceNode));
			}
			else
			{
				static const char nodeError[] = "TriggerSequencer defined with no clock, skipping.";
				Serial.println(String(nodeError));
			}
		}
		else if (strcmp(typeNode->valuestring, "Trigger") == 0)
		{
			if (clockDevice != NULL)
			{
				clockDevice->registerDevice(Trigger::create(deviceNode));
			}
			else
			{
				static const char nodeError[] = "Trigger defined with no clock, skipping.";
				Serial.println(String(nodeError));
			}
		}
		else if (strcmp(typeNode->valuestring, "VCSamplingFrequencyOscillator") == 0)
		{
			EventManager::registerDevice(VCSamplingFrequencyOscillator::create(deviceNode));
		}
	}	
}