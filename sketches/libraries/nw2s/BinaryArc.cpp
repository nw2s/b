/*
	BinaryArc - a binary sequencer for nw2s::b and monome arc
	copyright (c) 2015 scanner darkly (fuzzybeacon@gmail.com)

	This code is developed for the the nw2s::b framework 
	Copyright (C) 2013 Scott Wilson (thomas.scott.wilson@gmail.com)

	Parts of it are also based on USB Host library 
	https://github.com/felis/USB_Host_Shield_2.0
	and the monome
	https://github.com/monome

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

#include "BinaryArc.h"

#define TRIGGER_LENGTH 40 // must be over 20 for the triggers out to be used reliably as triggers in with 20ms jitter protection threshold
#define DIVISOR_DELTA 40
#define LEVEL_DELTA 25
#define CONFIG_FOLDER "CONFIG"
#define CONFIG_FILE_NAME "binarc.cfg"

using namespace nw2s;

BinaryArc* BinaryArc::create(uint8_t encoderCount, bool pushButton)
{
	return new BinaryArc(encoderCount, pushButton);
}

BinaryArc* BinaryArc::create(aJsonObject* data)
{
	static const char encoderCountNodeName[] = "encoderCount";
	static const char pushButtonNodeName[] = "pushButton";
	static const char clockNodeName[] = "externalClock";

	uint8_t encoderCount = getIntFromJSON(data, encoderCountNodeName, 4, 1, 4);
	bool pushButton = getBoolFromJSON(data, pushButtonNodeName, false);
	PinDigitalIn clockPin = getDigitalInputFromJSON(data, clockNodeName);
	
	BinaryArc* binaryArc = new BinaryArc(encoderCount, pushButton);
	
	if (clockPin != DIGITAL_IN_NONE)
	{
		binaryArc->setClockInput(clockPin);
	}
			
	return binaryArc;
}

BinaryArc::BinaryArc(uint8_t encoderCount, bool pushButton) : USBArcController(encoderCount, pushButton)
{
	this->encoderCount = encoderCount;
	this->pushButton = pushButton;

	this->beat = 0;
	this->clock_division = DIV_SIXTEENTH;

	mainCvOut = AnalogOut::create(DUE_SPI_4822_15);
	for (int i = 0; i < ARC_MAX_ENCODERS; i++)
	{	
		transposeCvIn[i] = INDEX_ANALOG_IN[i+1];
		phaseCvIn[i] = INDEX_ANALOG_IN[i+1+ARC_MAX_ENCODERS];
		divisorCvIn[i] = INDEX_ANALOG_IN[i+1+ARC_MAX_ENCODERS*2];
		pitchCvOut[i] = AnalogOut::create(INDEX_ANALOG_OUT[i*2+1]);
		cvOut[i] = AnalogOut::create(INDEX_ANALOG_OUT[i*2+2]);
		gateOutput[i] = INDEX_DIGITAL_OUT[i*2+1];
		triggerOutput[i] = INDEX_DIGITAL_OUT[i*2+2];
		triggerState[i] = 0;
	}
	
	readConfig();
	
	for (int i = 0; i < ARC_MAX_ENCODERS; i++)
	{	
		updateRing(i);
	}

	delay(100);	
	IOUtils::displayBeat(scale, this);
	refreshArc();
}

void BinaryArc::readConfig()
{
	for (int i = 0; i < ARC_MAX_ENCODERS; i++)
	{	
		transposeCv[i] = aRead(transposeCvIn[i]);
		delay(10); // give the ADC time to recover
		phaseCv[i] = aRead(phaseCvIn[i]);
		phase[i] = phaseCv[i] * ARC_MAX_LEDS / 2048;
		delay(10);
		divisorCv[i] = aRead(divisorCvIn[i]) * MAX_DIVISORS / 2048;
		delay(10);
	}

	scale = 0;
	for (int i = 0; i < ARC_MAX_ENCODERS; i++)
	{	
		divisor[i] = i % MAX_DIVISORS;
		level[i] = 0;
	}

	SdFile root = b::getSDRoot(); 
	SdFile configFolder;
	SdFile configFile;

	if (!configFolder.open(root, CONFIG_FOLDER, O_READ))
	{
	    Serial.println("Could not find config folder");
	    return;
	}
	
	if (!configFile.open(configFolder, CONFIG_FILE_NAME, O_READ))
	{
	    Serial.print("Could not read config, error opening config file ");
		Serial.println(CONFIG_FILE_NAME);
	    return;
	}
	
	uint fileSize = configFile.fileSize();
	char configData[fileSize + 1];
	configFile.read(configData, fileSize);
	configData[fileSize] == '\0';
		
	aJsonObject* sdConfig = aJson.parse(configData);

    if (sdConfig == NULL) 
	{
        Serial.println("Config file not parsed successfully. Check to see that it's properly formatted JSON.");
		return;
	}
	else
	{
        Serial.println("Config file parsed successfully.");
	}
	
	scale = getIntFromJSON(sdConfig, "scale", 0, 0, MAX_SCALES);
	
	aJsonObject* jsonArray;
	
	jsonArray = aJson.getObjectItem(sdConfig, "divisors");
	if (jsonArray == NULL)
	{
		Serial.println("Could not find divisors in the config data");
		return;
	}
	for (int i = 0; i < aJson.getArraySize(jsonArray); i++)
	{
		if (i >= ARC_MAX_ENCODERS) break;
		aJsonObject* item = aJson.getArrayItem(jsonArray, i);
		divisor[i] = item->valueint;
	}
	jsonArray = aJson.getObjectItem(sdConfig, "levels");
	if (jsonArray == NULL)
	{
		Serial.println("Could not find levels in the config data");
		return;
	}
	for (int i = 0; i < aJson.getArraySize(jsonArray); i++)
	{
		if (i >= ARC_MAX_ENCODERS) break;
		aJsonObject* item = aJson.getArrayItem(jsonArray, i);
		level[i] = item->valueint;
	}

	aJson.deleteItem(sdConfig);
	Serial.println("Config data loaded successfully");
}

void BinaryArc::saveConfig()
{
	aJsonObject *root;
	root = aJson.createObject();
	
	aJson.addNumberToObject(root, "scale", scale);
	
	aJsonObject* jsonArray;

	jsonArray = aJson.createArray();
	for (int i = 0; i < ARC_MAX_ENCODERS; i++)
	{
		aJsonObject* item = aJson.createItem(divisor[i]);
		aJson.addItemToArray(jsonArray, item);
	}
	aJson.addItemToObject(root, "divisors", jsonArray);	

	jsonArray = aJson.createArray();
	for (int i = 0; i < ARC_MAX_ENCODERS; i++)
	{
		aJsonObject* item = aJson.createItem(level[i]);
		aJson.addItemToArray(jsonArray, item);
	}
	aJson.addItemToObject(root, "levels", jsonArray);	
	
	aJsonStringStream stringStream(NULL, jsonBuffer, 512);
	aJson.print(root, &stringStream);

	Serial.println("--- config JSON created ----------------------------");
	Serial.println(jsonBuffer);
	Serial.println("----------------------------------------------------");
	
	SdFile rootFile = b::getSDRoot(); 
	SdFile configFolder;
	if (!configFolder.open(rootFile, CONFIG_FOLDER, O_READ))
	{
	    if (configFolder.makeDir(rootFile, CONFIG_FOLDER))
		{
			Serial.println("Config folder not found, created");
		}
		else
		{
			Serial.println("Config folder not found and could not create one");
			return;
		}
		if (!configFolder.open(rootFile, CONFIG_FOLDER, O_READ))
		{
			Serial.println("Could not open config folder after creating one");
		}
	}
	
	SdFile configFile;
	if (configFile.open(configFolder, CONFIG_FILE_NAME, O_CREAT | O_WRITE | O_TRUNC))
	{
		configFile.println(jsonBuffer);
		configFile.close();
		Serial.println("Config saved!");
	}
	else
	{
	    Serial.print("Could not write config, error opening config file for writing ");
		Serial.println(CONFIG_FILE_NAME);
	}
	
	aJson.deleteItem(root);
}

void BinaryArc::setClockInput(PinDigitalIn input)
{
	this->clockInput = input;
}

void BinaryArc::timer(unsigned long t)
{
	currentTime = t;
	if (readCvClockState < t)
	{
		uint8_t cvIndex = readCvCounter % ARC_MAX_ENCODERS;
		if (readCvCounter < ARC_MAX_ENCODERS)
		{
			transposeCv[cvIndex] = aRead(transposeCvIn[cvIndex]);
		}
		else if (readCvCounter < ARC_MAX_ENCODERS * 2)
		{
			int newPhaseCv = aRead(phaseCvIn[cvIndex]);
			if (abs(newPhaseCv - phaseCv[cvIndex]) > 35)
			{
				phaseCv[cvIndex] = newPhaseCv;
				phase[cvIndex] = constrain(phaseCv[cvIndex] * ARC_MAX_LEDS / 2048, 0, 63);
				updateRing(cvIndex);
			}
		}
		else
		{
			int newDivisor = aRead(divisorCvIn[cvIndex]) * MAX_DIVISORS / 2048;
			if (abs(newDivisor - divisorCv[cvIndex]) > 0)
			{
				divisorCv[cvIndex] = newDivisor;
				updateRing(cvIndex);
			}
		}
		
		readCvCounter = (readCvCounter + 1) % (ARC_MAX_ENCODERS * 3);
		readCvClockState = t + 10; // stagger reading CVs
	}

	for (int ring = 0; ring < ARC_MAX_ENCODERS; ring++)
	{
		if (triggerState[ring] && triggerState[ring] < t)
		{
			digitalWrite(triggerOutput[ring], LOW);
			triggerState[ring] = 0;
		}
	}

	if (!resetState && digitalRead(resetInput))
	{
		resetState = t + 20;
		counter = 0;
		for (int ring = 0; ring < ARC_MAX_ENCODERS; ring++)
			updateRing(ring);
	} 
	else if (resetState && (resetState < t) && !digitalRead(resetInput))
	{
		resetState = 0;
	}

	if (!scaleState && digitalRead(nextScaleInput))
	{
		scaleState = t + 20;
		scale = (scale + 1) % MAX_SCALES;
		IOUtils::displayBeat(scale, this);
	}
	else if (scaleState && (scaleState < t) && !digitalRead(nextScaleInput))
	{
		scaleState = 0;
	}

	if (!saveConfigState && digitalRead(saveConfigInput))
	{
		saveConfigState = t + 20;
		saveConfig();
	}
	else if (saveConfigState && (saveConfigState < t) && !digitalRead(saveConfigInput))
	{
		saveConfigState = 0;
	}

	bool counterChanged = false;
	if (clockInput != DIGITAL_IN_NONE && !clockState && digitalRead(clockInput))
	{
		clockState = t + 20;
		counterChanged = true;
		reset();
	}
	else if (clockInput != DIGITAL_IN_NONE && clockState && (clockState < t) && !digitalRead(clockInput))
	{
		clockState = 0;
	}
	
	if (refresh)
	{
		if (isReady()) refreshArc();
		for (int ring = 0; ring < ARC_MAX_ENCODERS; ring++)
		{
			digitalWrite(gateOutput[ring], prevValue[ring] ? HIGH : LOW);
			if (counterChanged && prevValue[ring] != values[0][ring][(counter + ARC_MAX_LEDS - 1) % ARC_MAX_LEDS])
			{
				digitalWrite(triggerOutput[ring], HIGH);
				triggerState[ring] = currentTime + TRIGGER_LENGTH;
			}
			updateOutputCvs(ring);
		}
		updateMainCv();
		refresh = false;
	}
}

void BinaryArc::reset()
{
	for (int ring = 0; ring < ARC_MAX_ENCODERS; ring++)
	{
		values[0][ring][counter] = prevValue[ring];
	}
	counter = (counter + 1) % ARC_MAX_LEDS;
	for (int ring = 0; ring < ARC_MAX_ENCODERS; ring++)
	{
		prevValue[ring] = values[0][ring][counter];
		values[0][ring][counter] = 15;
	}
	refresh = true;
}

void BinaryArc::updateRing(uint8_t ring)
{
	for(int led = 0; led < ARC_MAX_LEDS; led++)
	{
		values[0][ring][led] = ((led + phase[ring])/ getDivisor(ring)) % 2 ? 0 : level[ring];
		if (led == counter)
		{
			prevValue[ring] = values[0][ring][led];
			values[0][ring][led] = 15;
		}
	}
	refresh = true;
}

void BinaryArc::updateOutputCvs(uint8_t ring)
{
	pitchCvOut[ring]->outputCV(getNoteCv(transposeCv[ring], prevValue[ring]));
	cvOut[ring]->outputCV(prevValue[ring] * 273);
}

void BinaryArc::updateMainCv()
{
	int totalTranspose, totalLevel;
	totalTranspose = totalLevel = 0;
	for (int ring = 0; ring < ARC_MAX_ENCODERS; ring++)
		if (digitalRead(sumInput[ring]))
		{
			totalTranspose += transposeCv[ring];
			totalLevel += prevValue[ring];
		}
	mainCvOut->outputCV(getNoteCv(totalTranspose, totalLevel));
}

int BinaryArc::aRead(PinAnalogIn analogIn)
{
	return constrain(analogRead(analogIn) - 2048 - 20, 0, 2047);
}

int BinaryArc::getNoteCv(int transpose, uint8_t level)
{
	int realLevel = transpose * SCALES[scale].length / 2048 + level;
	return constrain(realLevel/SCALES[scale].length*1000, 0, 2000) + SEMITONE_MV[SCALES[scale].semis[realLevel%SCALES[scale].length]];
}

uint8_t BinaryArc::getDivisor(uint8_t ring)
{
	return divisors[(divisor[ring] + divisorCv[ring]) % MAX_DIVISORS];
}

void BinaryArc::encoderPositionChanged(uint8_t ring, int8_t delta)
{
	if (delta < 0)
	{
		deltaDivState += abs(delta);
		if (deltaDivState > DIVISOR_DELTA)
		{
			deltaDivState = 0;
			divisor[ring] = (divisor[ring] + 1) % MAX_DIVISORS;
			updateRing(ring);
		}
	}
	else
	{
		deltaLevelState += delta;
		if (deltaLevelState > LEVEL_DELTA)
		{
			deltaLevelState = 0;
			level[ring] = (level[ring] + 1) % 16;
			updateRing(ring);
		}
	}
}

void BinaryArc::buttonPressed(uint8_t encoder) {}
void BinaryArc::buttonReleased(uint8_t encoder) {}
