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

#include "GameOfLife.h"

#define TRIGGER_LENGTH 40 // must be over 20 for the triggers out to be used reliably as triggers in with 20ms jitter protection threshold
#define CONFIG_FOLDER "configs"
#define GAME_OF_LIFE_CONFIG_FILE_NAME "gamelife.cfg"

using namespace nw2s;

GameOfLife* GameOfLife::create(GridDevice deviceType, uint8_t columnCount, uint8_t rowCount, bool varibright)
{
	return new GameOfLife(deviceType, columnCount, rowCount, varibright);
}

GameOfLife* GameOfLife::create(aJsonObject* data)
{
	static const char rowsNodeName[] = "rows";
	static const char columnsNodeName[] = "columns";
	static const char clockNodeName[] = "externalClock";
	static const char varibrightNodeName[] = "varibright";

	GridDevice device = getGridDeviceFromJSON(data);
	int rows = getIntFromJSON(data, rowsNodeName, 8, 8, 16);
	int columns = getIntFromJSON(data, columnsNodeName, 8, 8, 16);
	bool varibright = getBoolFromJSON(data, varibrightNodeName, false);

	PinDigitalIn clockPin = getDigitalInputFromJSON(data, clockNodeName);
	
	GameOfLife* grid = new GameOfLife(device, columns, rows, varibright);
	
	if (clockPin != DIGITAL_IN_NONE)
	{
		grid->setClockInput(clockPin);
	}
			
	return grid;
}

GameOfLife::GameOfLife(GridDevice deviceType, uint8_t columnCount, uint8_t rowCount, bool varibright) : USBGridController(deviceType, columnCount, rowCount)
{
	this->varibright = varibright;

	this->beat = 0;
	this->clock_division = DIV_SIXTEENTH;

	// page 0 is for the game itself, page 1 is the control page, page 2 is used to track buttons pressed
	for (uint8_t page = 0; page < 16; page++)
	{
		for (uint8_t column = 0; column < 16; column++)
		{
			for (uint8_t row = 0; row < 16; row++)
			{
				this->cells[page][column][row] = 0;
			}
		}
	}

	readConfig();
	renderControlPage();
	
	memoryInitialized = true;
	
	// assign analog outs
	for (int i = 0; i < 16; i++)
		cvout[i] = AnalogOut::create(INDEX_ANALOG_OUT[i+1]);

	populationThreshold = columnCount * rowCount / 16; // this makes it 8 cells for grid128, seems like a good threshold
		
	/* Give it a moment... */
	delay(200);		
}

void GameOfLife::setClockInput(PinDigitalIn input)
{
	this->clockInput = input;
}

void GameOfLife::readConfig()
{
	SdFile root = b::getSDRoot(); 
	SdFile configFolder;
	SdFile configFile;

	if (!configFolder.open(root, CONFIG_FOLDER, O_READ))
	{
	    Serial.println("Could not find config folder");
	    return;
	}
	
	if (!configFile.open(configFolder, GAME_OF_LIFE_CONFIG_FILE_NAME, O_READ))
	{
	    Serial.print("Could not read config, error opening config file ");
		Serial.println(GAME_OF_LIFE_CONFIG_FILE_NAME);
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
	
	aJsonObject* jsonArray;
	
	jsonArray = aJson.getObjectItem(sdConfig, "gateMode");
	if (jsonArray == NULL)
	{
		Serial.println("Could not find gateMode in the config data");
		return;
	}
	for (int i = 0; i < aJson.getArraySize(jsonArray); i++)
	{
		if (i > 13) break;
		aJsonObject* item = aJson.getArrayItem(jsonArray, i);
		config.gateMode[i] = item->valueint;
	}
	
	jsonArray = aJson.getObjectItem(sdConfig, "cvMode");
	if (jsonArray == NULL)
	{
		Serial.println("Could not find cvMode in the config data");
		return;
	}
	for (int i = 0; i < aJson.getArraySize(jsonArray); i++)
	{
		if (i > 15) break;
		aJsonObject* item = aJson.getArrayItem(jsonArray, i);
		config.cvMode[i] = item->valueint;
	}
	
	jsonArray = aJson.getObjectItem(sdConfig, "cvRangeMin");
	if (jsonArray == NULL)
	{
		Serial.println("Could not find cvRangeMin in the config data");
		return;
	}
	for (int i = 0; i < aJson.getArraySize(jsonArray); i++)
	{
		if (i > 15) break;
		aJsonObject* item = aJson.getArrayItem(jsonArray, i);
		config.cvRangeMin[i] = item->valueint;
	}
	
	jsonArray = aJson.getObjectItem(sdConfig, "cvRangeMax");
	if (jsonArray == NULL)
	{
		Serial.println("Could not find cvRangeMax in the config data");
		return;
	}
	for (int i = 0; i < aJson.getArraySize(jsonArray); i++)
	{
		if (i > 15) break;
		aJsonObject* item = aJson.getArrayItem(jsonArray, i);
		config.cvRangeMax[i] = item->valueint;
	}
	
	jsonArray = aJson.getObjectItem(sdConfig, "noteScale");
	if (jsonArray == NULL)
	{
		Serial.println("Could not find noteScale in the config data");
		return;
	}
	for (int i = 0; i < aJson.getArraySize(jsonArray); i++)
	{
		if (i > 15) break;
		aJsonObject* item = aJson.getArrayItem(jsonArray, i);
		config.noteScale[i] = item->valueint;
	}
	
	aJson.deleteItem(sdConfig);
	Serial.println("Config data loaded successfully");
}

void GameOfLife::saveConfig()
{
	aJsonObject *root;
	root = aJson.createObject();
	
	aJsonObject* jsonArray;

	jsonArray = aJson.createArray();
	for (int i = 0; i < 14; i++)
	{
		aJsonObject* item = aJson.createItem(config.gateMode[i]);
		aJson.addItemToArray(jsonArray, item);
	}
	aJson.addItemToObject(root, "gateMode", jsonArray);	

	jsonArray = aJson.createArray();
	for (int i = 0; i < 16; i++)
	{
		aJsonObject* item = aJson.createItem(config.cvMode[i]);
		aJson.addItemToArray(jsonArray, item);
	}
	aJson.addItemToObject(root, "cvMode", jsonArray);	

	jsonArray = aJson.createArray();
	for (int i = 0; i < 16; i++)
	{
		aJsonObject* item = aJson.createItem(config.cvRangeMin[i]);
		aJson.addItemToArray(jsonArray, item);
	}
	aJson.addItemToObject(root, "cvRangeMin", jsonArray);	

	jsonArray = aJson.createArray();
	for (int i = 0; i < 16; i++)
	{
		aJsonObject* item = aJson.createItem(config.cvRangeMax[i]);
		aJson.addItemToArray(jsonArray, item);
	}
	aJson.addItemToObject(root, "cvRangeMax", jsonArray);	
	
	jsonArray = aJson.createArray();
	for (int i = 0; i < 16; i++)
	{
		aJsonObject* item = aJson.createItem(config.noteScale[i]);
		aJson.addItemToArray(jsonArray, item);
	}
	aJson.addItemToObject(root, "noteScale", jsonArray);
	
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
	if (configFile.open(configFolder, GAME_OF_LIFE_CONFIG_FILE_NAME, O_CREAT | O_WRITE | O_TRUNC))
	{
		configFile.println(jsonBuffer);
		configFile.close();
		Serial.println("Config saved!");
	}
	else
	{
	    Serial.print("Could not write config, error opening config file for writing ");
		Serial.println(GAME_OF_LIFE_CONFIG_FILE_NAME);
	}
	
	aJson.deleteItem(root);
}

void GameOfLife::timer(unsigned long t)
{
	currentTime = t;
	bool refresh = false;
	
	if (!newCellTriggerState && digitalRead(newCellTrigger))
	{
		newCellTriggerState = t;

		int x = constrain(aRead(newCellXCV) * columnCount / 4096, 0, columnCount - 1) - 1;
		int y = constrain(aRead(newCellYCV) * rowCount / 4096, 0, rowCount - 1) - 1;
		int shape = constrain(aRead(newCellShapeCV) * newShapesCount / 4096, 0, newShapesCount - 1);
		
		for (int i = 0; i < 3; i++)
			for (int j = 0; j < 3; j++)
				if (newShapes[shape][i][j])
				{
					int _x = wrapX(x+i);
					int _y = wrapY(y+j);
					this->cells[0][_x][_y] = 15;
					lifecells[generation][_x][_y] = 15;
				}
				
		refresh = true;
	}
	if (newCellTriggerState && ((newCellTriggerState + 20) < t) && !digitalRead(newCellTrigger))
	{
		newCellTriggerState = 0;
	}
	
	if (!shiftLeftTriggerState && digitalRead(shiftLeftTrigger))
	{
		shiftLeftTriggerState = t;
		shiftCells(-1, 0);
		refresh = true;
	}
	if (shiftLeftTriggerState && ((shiftLeftTriggerState + 20) < t) && !digitalRead(shiftLeftTrigger))
	{
		shiftLeftTriggerState = 0;
	}
	
	if (!shiftRightTriggerState && digitalRead(shiftRightTrigger))
	{
		shiftRightTriggerState = t;
		shiftCells(1, 0);
		refresh = true;
	}
	if (shiftRightTriggerState && ((shiftRightTriggerState + 20) < t) && !digitalRead(shiftRightTrigger))
	{
		shiftRightTriggerState = 0;
	}
	
	if (!shiftDownTriggerState && digitalRead(shiftDownTrigger))
	{
		shiftDownTriggerState = t;
		shiftCells(0, -1);
		refresh = true;
	}
	if (shiftDownTriggerState && ((shiftDownTriggerState + 20) < t) && !digitalRead(shiftDownTrigger))
	{
		shiftDownTriggerState = 0;
	}

	if (!shiftUpTriggerState && digitalRead(shiftUpTrigger))
	{
		shiftUpTriggerState = t;
		shiftCells(0, 1);
		refresh = true;
	}
	if (shiftUpTriggerState && ((shiftUpTriggerState + 20) < t) && !digitalRead(shiftUpTrigger))
	{
		shiftUpTriggerState = 0;
	}

	if (triggerStart + TRIGGER_LENGTH > t)
	{
		for (int i = 2; i < columnCount; i++) // first two digital outs are reserved for "too full / too empty"
		{
			if (!config.gateMode[i - 2])
				digitalWrite(INDEX_DIGITAL_OUT[i + 1], LOW);
		}
		triggerStart = 0;
	}
	
	/* Clock is rising */
	if (this->clockInput != DIGITAL_IN_NONE && !this->clockState && digitalRead(this->clockInput))
	{
		this->clockState = t;

		/* Simulate a clock pulse */
		this->reset();
	}
	else if (refresh && isReady())
	{
		this->refreshGrid();
	}

	/* Clock is falling and at least 20ms after rising */
	if (this->clockInput != DIGITAL_IN_NONE && this->clockState && ((this->clockState + 20) < t) && !digitalRead(this->clockInput))
	{
		this->clockState = 0;
	}
}

void GameOfLife::reset()
{
	beat = (beat + 1) % columnCount;
	nextGeneration();

	if (isReady()) this->refreshGrid();
}

void GameOfLife::renderCVControlColumn(int column)
{
	int minRow = rowCount - 2 - constrain(config.cvRangeMax[column] * (rowCount - 3) / 4096, 0, rowCount - 3);
	int maxRow = rowCount - 2 - constrain(config.cvRangeMin[column] * (rowCount - 3) / 4096, 0, rowCount - 3);
	
	for (int row = 1; row < rowCount - 1; row++)
		this->cells[1][column][row] = (row >= minRow && row <= maxRow) ? 8 : 0;
}

void GameOfLife::renderGateModeRow()
{
	for (int i = 0; i < 14; i++)
		this->cells[1][i+2][0] = config.gateMode[i] ? 15 : 0;
}

void GameOfLife::renderCVModeRow()
{
	for (int i = 0; i < 16; i++)
		this->cells[1][i][rowCount - 1] = 0; // reserved for switching between CV mode and note mode
}

void GameOfLife::renderControlPage()
{
	this->cells[1][0][0] = this->cells[1][1][0] = 15;

	renderGateModeRow();
	renderCVModeRow();
	
	for (int i = 0; i < 16; i++)
		renderCVControlColumn(i);
}

int GameOfLife::wrapX(int x)
{
	return (x + columnCount) % columnCount;
}

int GameOfLife::wrapY(int y)
{
	return (y + rowCount) % rowCount;
}

int GameOfLife::calculateNeighbours(int gen, int j, int  k)
{
	return 
		(lifecells[gen][wrapX(j-1)][wrapY(k-1)] ? 1 : 0) + 
		(lifecells[gen][wrapX(j-1)][wrapY(k)] ? 1 : 0) + 
		(lifecells[gen][wrapX(j-1)][wrapY(k+1)] ? 1 : 0) + 
		(lifecells[gen][wrapX(j)][wrapY(k-1)] ? 1 : 0) + 
		(lifecells[gen][wrapX(j)][wrapY(k+1)] ? 1 : 0) + 
		(lifecells[gen][wrapX(j+1)][wrapY(k-1)] ? 1 : 0) + 
		(lifecells[gen][wrapX(j+1)][wrapY(k)] ? 1 : 0) + 
		(lifecells[gen][wrapX(j+1)][wrapY(k+1)] ? 1 : 0);
}

int GameOfLife::countColumn(int gen, int column)
{
	int cv = 0;
	for (int j = 0; j < rowCount; j++)
		cv += lifecells[gen][column][j] ? 1 : 0;
	return cv;
}

void GameOfLife::copyCellsToGrid(int gen)
{
	for (uint8_t column = 0; column < columnCount; column++)
	{
		for (uint8_t row = 0; row < rowCount; row++)
		{
			this->cells[0][column][row] = lifecells[gen][column][row];
		}
	}
}

void GameOfLife::shiftCells(int deltaX, int deltaY)
{
	int nextGen = (generation + 1) % 2;
	for (uint8_t column = 0; column < columnCount; column++)
	{
		for (uint8_t row = 0; row < rowCount; row++)
		{
			lifecells[nextGen][wrapX(column + deltaX)][wrapY(row + deltaY)] = lifecells[generation][column][row];
		}
	}
	copyCellsToGrid(nextGen);
	generation = nextGen;
}

int GameOfLife::aRead(PinAnalogIn analogIn)
{
	return constrain((analogRead(analogIn) - 2048) * 2, 0, 4095); // TODO not sure yet how to fix this for using bipolar CVs into the analog ins
}

void GameOfLife::nextGeneration()
{
	int nextGen = (generation + 1) % 2;
	bool isRandom = digitalRead(generateRandomTrigger);

	for (int column = 0; column < columnCount; column++)
	{
		for (int row = 0; row < rowCount; row++)
		{
			if (isRandom)
			{
				lifecells[nextGen][column][row] = random(0, 4096) < aRead(randomDensityCV) ? 15 : 0;
			}
			else 
			{
				int minNew = 3, maxNew = 3, minSurvive = 2, maxSurvive = 3;
				if (digitalRead(cvRulesOnSwitch))
				{
					minNew = aRead(minNewCV) * 9 / 4096;
					maxNew = aRead(maxNewCV) * 9 / 4096;
					minSurvive = aRead(minSurviveCV) * 9 / 4096;
					maxSurvive = aRead(maxSurviveCV) * 9 / 4096;
				}
				int neighbours = calculateNeighbours(generation, column, row);
				if (debug)
				{
					Serial.print(neighbours);
				}
				lifecells[nextGen][column][row] = 0;
				if ((neighbours >= minSurvive && neighbours <= maxSurvive) && lifecells[generation][column][row])
					lifecells[nextGen][column][row] = constrain(lifecells[generation][column][row] - 1, 4, 15);
				else if ((neighbours >= minNew && neighbours <= maxNew) && !lifecells[generation][column][row])
					lifecells[nextGen][column][row] = 15;
			}
		}
		if (debug)
		{
			Serial.println();
		}
	}

	this->copyCellsToGrid(nextGen);
	
	int totalPopulation = 0;
	for (int i = 0; i < columnCount; i++)
	{
		for (int j = 0; j < rowCount; j++)
		{
			totalPopulation += (lifecells[nextGen][i][j] ? 1 : 0);
		}
	}
	digitalWrite(INDEX_DIGITAL_OUT[1], totalPopulation + populationThreshold > (columnCount * rowCount) ? HIGH : LOW);
	digitalWrite(INDEX_DIGITAL_OUT[2], totalPopulation < populationThreshold ? HIGH : LOW);
	
	for (int i = 2; i < columnCount; i++)
	{
		digitalWrite(INDEX_DIGITAL_OUT[i + 1], !lifecells[generation][i][0] && lifecells[nextGen][i][0] ? HIGH : LOW);
	}
	triggerStart = currentTime;
	
	if (debug)
	{
		Serial.println("-- generation --");
		for (int i = 0; i < columnCount; i++)
		{
			for (int j = 0; j < rowCount; j++)
			{
				Serial.print(lifecells[generation][i][j]);
			}
			Serial.println();
		}
		Serial.println("-- neighbours --");
		for (int i = 0; i < columnCount; i++)
		{
			for (int j = 0; j < rowCount; j++)
			{
				Serial.print(calculateNeighbours(generation, i, j));
			}
			Serial.println();
		}
		Serial.println("-- next generation --");
		for (int i = 0; i < columnCount; i++)
		{
			for (int j = 0; j < rowCount; j++)
			{
				Serial.print(lifecells[nextGen][i][j]);
			}
			Serial.println();
		}
		debug = 0;
	}

	for (int i = 0; i < columnCount; i++)
		{
			cvout[i]->outputCV(constrain((config.cvRangeMax[i] - config.cvRangeMin[i]) / rowCount * countColumn(nextGen, i) + config.cvRangeMin[i], 0, 4096));
		}
	
	generation = nextGen;
}

void GameOfLife::buttonPressed(uint8_t column, uint8_t row)
{	
	this->cells[2][column][row] = 1;

	if (column == 0 && row == 0)
	{
		// debug = 1; 
		controlButton1 = true;
	}
	else if (column == 1 && row == 0)
	{
		controlButton2 = true;
	}
	
	if (controlButton1 && controlButton2)
	{
		if (isControl)
		{
			saveConfig();
			this->currentPage = 0;
		}
		else
		{
			this->currentPage = 1;
		}
		isControl = !isControl;
		this->refreshGrid();
		return;
	}
		
	if (isControl)
	{
		if (column >= 2 && row == 0)
		{
			config.gateMode[column - 2] = config.gateMode[column - 2] ? 0 : 1;
			renderGateModeRow();
		}
		else if (row == rowCount - 1)
		{
			// reserved for switching between CV mode and note mode
		}
		else
		{
			int topRowPressed = 0;
			for (int i = 1; i < rowCount - 1; i++)
			{
				if (cells[2][column][i])
				{
					topRowPressed = i;
					break;
				}
			}
			int bottomRowPressed = 0;
			for (int i = rowCount - 2; i > 0; i--)
			{
				if (cells[2][column][i])
				{
					bottomRowPressed = i;
					break;
				}
			}
			if (topRowPressed != bottomRowPressed)
			{
				config.cvRangeMin[column] = (rowCount - 2 - bottomRowPressed) * 4096 / (rowCount - 3) + 1;
				config.cvRangeMax[column] = (rowCount - 2 - topRowPressed) * 4096 / (rowCount - 3) + 1;
				renderCVControlColumn(column);
			}
		}
	}
	else
	{
		if (lifecells[generation][column][row])
		{
			this->cells[0][column][row] = 0;
			lifecells[generation][column][row] = 0;
			digitalWrite(INDEX_DIGITAL_OUT[column + 1], !lifecells[generation][column][0] && lifecells[(generation + 1) % 2][column][0] ? HIGH : LOW);
		}
		else
		{
			this->cells[0][column][row] = 15;
			lifecells[generation][column][row] = 15;
			digitalWrite(INDEX_DIGITAL_OUT[column + 1], LOW);
		}
		cvout[column]->outputCV(constrain((config.cvRangeMax[column] - config.cvRangeMin[column]) / rowCount * countColumn(generation, column) + config.cvRangeMin[column], 0, 4096));
	}
		
	if (isReady()) this->refreshGrid();
	delay(5);
	// Serial.println("\npressed");
}

void GameOfLife::buttonReleased(uint8_t column, uint8_t row)
{
	this->cells[2][column][row] = 0;

	if (column == 0 && row == 0)
	{
		controlButton1 = false;
	}
	else if (column == 1 && row == 0)
	{
		controlButton2 = false;
	}
	
	//Serial.println("");
}


