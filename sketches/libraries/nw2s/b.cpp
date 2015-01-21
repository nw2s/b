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

#include <SD.h>
#include "IO.h"
#include "b.h"
#include "../aJSON/aJSON.h"

using namespace nw2s;

/* GLOBAL SETTINGS */
bool b::debugMode = false;
bool b::cvGainMode = CV_GAIN_LOW;
bool b::rootInitialized = false;
DeviceModel b::model = NW2S_B_1_0_0;

bool b::softTune = false;
int32_t b::dimming = 50;
int16_t b::offset[16] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
int32_t b::scale[16] = { 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000 };




SdFile b::root;
Sd2Card b::card;
SdVolume b::volume;


SdFile b::getSDRoot()
{
	if (rootInitialized)
	{
		return b::root;
	} 
	else
	{
		if (!card.init(SPI_HALF_SPEED, SD_CS)) 
		{
		    Serial.println("Initialization failed. Is a card is inserted?");
		    return SdFile();
		} 
		
		if (!volume.init(card)) 
		{
			Serial.println("Could not find FAT16/FAT32 partition.\nMake sure you've formatted the card");
		    return SdFile();
		}

		if (!root.openRoot(volume))
		{
		    Serial.println("Error opening root folder. Is something wrong with the card?");
		    return SdFile();
		}
		
		rootInitialized = true;
		b::root = root;
		return root;
	}	
}

void b::configure()
{
	/* Loads the config JSON file from SD card (if it exists) and sets up the system variables */
	/* Loads the config JSON file from SD card (if it exists) and sets up the system variables */
	/* Loads the config JSON file from SD card (if it exists) and sets up the system variables */
	
	SdFile root;
	SdFile configDir;
	SdFile configFile;

	/* Open the file '/config/sys.b' */

	root = getSDRoot();
	
	if (!configDir.open(root, "CONFIG", O_READ))
	{
	    Serial.println("No config folder found. Using default configuration.");
	    return;
	}
	
	if (!configFile.open(configDir, "SYS.B", O_READ))
	{
	    Serial.println("Error opening '/config/sys.b'. Using default configuration.");
	    return;
	}

	/* Load and parse the config definition file */

	uint fileSize = configFile.fileSize();
	char configData[fileSize + 1];

	configFile.read(configData, fileSize);
	configData[fileSize] == '\0';
		
	aJsonObject* config = aJson.parse(configData);

    if (config == NULL) 
	{
		static const char error[] = "Configuration not parsed successfully. Check to see that it's properly formatted JSON."; 
        Serial.println(error);
		return;
	}
	else
	{
		static const char msg[] = "Configuration parsed successfully.";
        Serial.println(msg);
	}
	
	/* Get the root node and iterate over the properties, updating what's available. */
	
	aJsonObject* configNode = aJson.getObjectItem(config, "configuration"); 
	
	
	/* Module types */
	
	aJsonObject* moduleNode = aJson.getObjectItem(configNode, "module"); 
	
	if (strcmp(moduleNode->valuestring, "NW2S-B-1-0-0") == 0)
	{
		Serial.println("Model: nw2s::b 1.0.0");
		b::model = NW2S_B_1_0_0;
	}
	else
	{
		Serial.print("Unknown Model, skipping: ");
		Serial.println(moduleNode->valuestring);
	}
	
	/* Gain mode */

	aJsonObject* gainNode = aJson.getObjectItem(configNode, "gainmode"); 
	
	if (strcmp(gainNode->valuestring, "HIGH") == 0)
	{
		Serial.println("Gain: HIGH +/- 10V operation");
		b::cvGainMode = CV_GAIN_HIGH;
	}
	else if (strcmp(gainNode->valuestring, "LOW") == 0)
	{
		Serial.println("Gain: LOW +/- 5V operation");
		b::cvGainMode = CV_GAIN_LOW;
	}
	else
	{
		Serial.print("Unknown gain, skipping: ");
		Serial.println(gainNode->valuestring);
	}
	
	/* Brightness/Dimming */
	/* Values can range from 10 to 100. 10 = no dimming. 100 = 1/10 the brightness level */

	aJsonObject* dimNode = aJson.getObjectItem(configNode, "dimming"); 
	
	Serial.print("Dim: ");
	Serial.println(dimNode->valueint);
	b::dimming = dimNode->valueint;
	
	/* Output config */

	aJsonObject* outputNode = aJson.getObjectItem(configNode, "outputs"); 
	
	aJsonObject* outputTuneNode = aJson.getObjectItem(outputNode, "software-tune"); 
	
	if (outputTuneNode->valuebool)
	{
		b::softTune = true;
		
		aJsonObject* scaleNode = aJson.getObjectItem(outputNode, "scale"); 
		
		if (aJson.getArraySize(scaleNode) == 16)
		{
			Serial.print("Scale: { ");
			
			for (int i = 0; i < aJson.getArraySize(scaleNode); i++)
			{
				aJsonObject* valueNode = aJson.getArrayItem(scaleNode, i);
				
				b::scale[i] = valueNode->valueint;
				
				Serial.print(b::scale[i]);
				Serial.print(" ");
			}
			
			Serial.println("}");
		}
		else
		{
			Serial.println("Configuration requires 16 scale values, skipping.");
		}
		
		
		aJsonObject* offsetNode = aJson.getObjectItem(outputNode, "offset"); 
		
		if (aJson.getArraySize(offsetNode) == 16)
		{
			Serial.print("Offset: { ");
			
			for (int i = 0; i < aJson.getArraySize(offsetNode); i++)
			{
				aJsonObject* valueNode = aJson.getArrayItem(offsetNode, i);
				
				b::offset[i] = valueNode->valueint;

				Serial.print(b::offset[i]);
				Serial.print(" ");
			}
			
			Serial.println("}");
		}
		else
		{
			Serial.println("Configuration requires 16 offset values, skipping.");
		}
	}
	else
	{
		Serial.println("Using default output tuning");
	}
	
	
	
	
	
}


