#include <EventManager.h>
#include <Entropy.h>
#include <Key.h>
#include <Trigger.h>
#include <Clock.h>
#include <Oscillator.h>
#include <Sequence.h>
#include <IO.h>
#include <SPI.h>
#include <Wire.h>
#include <SD.h>
#include <ShiftRegister.h>
#include <aJSON.h>
#include <SDFirmware.h>

using namespace nw2s;

// drums [z][y][x]
int const noOfDrumPrograms = 2;
int const noOfDrumSteps = 6;
int const noOfDrumOutputs = 6;
int drums[noOfDrumPrograms][noOfDrumOutputs][16] = {

	{
	  // 1  2  3  4  5  6  7  8   1  2  3  4  5  6  7  8
		{1, 0, 0, 0, 1, 0, 0, 0,  1, 0, 0, 0, 1, 0, 0, 0},
		{0, 0, 1, 0, 0, 0, 1, 0,  0, 0, 1, 0, 0, 0, 1, 0},
		{0, 1, 0, 1, 0, 1, 0, 0,  0, 1, 0, 1, 0, 1, 0, 1},
		{1, 0, 0, 0, 0, 0, 0, 0,  0, 0, 0, 0, 1, 0, 1, 0},
		{1, 0, 0, 1, 0, 0, 1, 0,  0, 1, 0, 0, 1, 0, 0, 1},
	},
	
	{
	  // 1  2  3  4  5  6  7  8   1  2  3  4  5  6  7  8
		{1, 0, 0, 0, 1, 0, 0, 0,  1, 0, 0, 0, 1, 0, 0, 0},
		{0, 0, 1, 0, 0, 0, 1, 0,  0, 0, 1, 0, 0, 0, 1, 0},
		{0, 1, 0, 1, 0, 1, 0, 0,  0, 1, 0, 1, 0, 1, 0, 1},
		{1, 0, 0, 0, 0, 0, 0, 0,  0, 0, 0, 0, 1, 0, 1, 0},
		{1, 0, 0, 1, 0, 0, 1, 0,  0, 1, 0, 0, 1, 0, 0, 1},
	}
	
};

/* Lets define our inputs up here since we use them in various places below */
const PinAnalogIn INPUT_TEMPO = DUE_IN_A00;
const PinAnalogIn INPUT_PROGRAM = DUE_IN_A01;
const PinAnalogIn INPUT_SUBTRACT = DUE_IN_A02;
const PinAnalogIn INPUT_ADD = DUE_IN_A03;

/* Ardcore sketch used integers, so we'll use an array instead */
PinDigitalOut outputs[noOfDrumOutputs] = 
{ 
	DUE_OUT_D00, 
	DUE_OUT_D01, 
	DUE_OUT_D02, 
	DUE_OUT_D03, 
	DUE_OUT_D04, 
	DUE_OUT_D05 
};


/* Use an array of 'gates' cause the off-on-off is too fast to see */
/* Gates are devices that will turn off after a duration and are managed by the EventManager */
/* We'll set up a really short gate to make it a trigger */
Gate* gates[noOfDrumOutputs];

/* randomization/probability based on 0-5V measurements in mV */
int randValueSubtract = 5000;
int randValueAdd = 5000;

int isThisATrigger[noOfDrumSteps] = { 1, 1, 1, 1, 1, 1 };
int anslagEveryOther[noOfDrumSteps] = { 1, 1, 1, 1, 1, 1 };

/* Manage state for the transitions */
int nexttime = 0; 
int column = 0;

void setup() 
{
	Serial.begin(19200);
	
	EventManager::initialize();
	
	/* Intialize our six gates */
	for (int i = 0; i < noOfDrumOutputs; i++)
	{
		/* Set up a gate for each output with a duration of 20ms */
		gates[i] = Gate::create(outputs[i], 10);
		EventManager::registerDevice(gates[i]);
	}
}

void loop() 
{
	if (EventManager::getT() >= nexttime) 
  	{
		/* Crude tempo based on an analog input */
		/* 333ms = 60BPM (slowest) to 100ms = 200BPM (fastest) */
    	nexttime += 100 + ((5000 - analogReadmV(INPUT_TEMPO, 0, 5000)) / 20);		
						
		/* Program Selection. Here it's once per beat */					
     	int drumProgram = (noOfDrumPrograms > 1) ? (analogReadmV(INPUT_PROGRAM, 0, 4900) / (5000 / noOfDrumPrograms)) : 0; 


		/* Instead of looping, just keep the current column as a state */
		column = (column + 1) % 16;

		/* temporal. start with first beat point... */
  	  	for (int row = 0; row < noOfDrumSteps; row++) 
  	  	{ 
			/* vertical, outputs. start with output 0... */

    		/* will the program run for this column? */
    		randValueSubtract = Entropy::getValue(0, 5000);

    		if (randValueSubtract > analogReadmV(INPUT_SUBTRACT, 0, 5000)) 
			{            
				/* the hit */
				if (drums[drumProgram][row][column] == 1) 
				{
					digitalWrite(outputs[row], HIGH);
				}

      		  	/* the 1 or 0 from the pattern is added to isThisATrigger */
				/* give isThisATrigger a 1 or 0 depending on hit or not */
      		  	isThisATrigger[row] = drums[drumProgram][row][column]; 

      		  	/* a 1 is added to isThisATrigger anyway... maybe */
      		  	randValueAdd = Entropy::getValue(0, 5000);

      		  	if (randValueAdd < analogReadmV(INPUT_ADD, 0, 5000))
	  		  	{
					digitalWrite(outputs[row], HIGH);
					isThisATrigger[row] = 1;
      		  	}
      
				/* check if gate should be turned off or kept open */
      		  	if (isThisATrigger[row] == 1) 
	  		  	{ 
		  			/* if this is a hit */
          		  	if (anslagEveryOther[row] == 1)
					{ 
						/* if the indicator variable shows 1 */
						gates[row]->reset();
          			  	
						/* indicator value set to 0 to indicate that the last hit turned gate off... 
						   so next one should keep it on and not go through this loop */							
						anslagEveryOther[row] = 0; 
        			}
        			else 
					{
						/* so that next time there will be a turning off */
          				anslagEveryOther[row] = 1;  
        			}
      			}         
		  	}
  		}
	}

	EventManager::loop();
}