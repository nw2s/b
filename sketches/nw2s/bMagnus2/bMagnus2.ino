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


// create a static declaration at the top of the page
// AnalogOut* out1;

using namespace nw2s;

////////////////////////////////////////////////////////////////////////

CVNoteSequencer* sequencer;

////////////////////////////////////////////////////////////////////////

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
    {1, 1, 0, 0, 1, 1, 0, 0,  1, 1, 0, 0, 1, 0, 0, 0},
    {0, 0, 1, 0, 0, 0, 1, 0,  0, 0, 1, 0, 0, 0, 1, 0},
    {0, 1, 0, 1, 0, 1, 0, 0,  0, 1, 0, 1, 0, 1, 0, 1},
    {1, 0, 0, 0, 0, 0, 0, 0,  0, 0, 0, 0, 1, 0, 1, 0},
    {1, 0, 0, 1, 0, 0, 1, 0,  0, 1, 0, 0, 1, 0, 0, 1},
  }
};

////////////////////////////////////////////////////////////////////////

/* Lets define our inputs up here since we use them in various places below */
const PinAnalogIn INPUT_TEMPO = DUE_IN_A00;
const PinAnalogIn INPUT_PROGRAM = DUE_IN_A01;
const PinAnalogIn INPUT_SUBTRACT = DUE_IN_A02;
const PinAnalogIn INPUT_ADD = DUE_IN_A03;
const PinAnalogIn INPUT_NOTE = DUE_IN_A04;

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

////////////////////////////////////////////////////////////////////////

void setup() 
{
  Serial.begin(19200);
  EventManager::initialize();

  // out1 = AnalogOut::create(DUE_SPI_4822_00);

  Clock* democlock = VariableClock::create(25, 525, DUE_IN_A11, 16);

  SequenceNote notelist[34] = { {1,1}, {1,3}, {1,5}, {1,1}, {1,3}, {1,5}, {1,1}, {1,5}, 
                  {2,1}, {2,3}, {2,5}, {2,1}, {2,3}, {2,5}, {2,1}, {2,5}, 
                  {3,1}, {3,3}, {3,5}, {3,1}, {3,3}, {3,5}, {3,1}, {3,5}, 
                  {4,1}, {4,3}, {4,5}, {4,1}, {4,3}, {4,5}, {4,1}, {5,1} };

  NoteSequenceData* notes = new NoteSequenceData(notelist, notelist + 34);

  // CVNoteSequencer(notes, key, scale, output, input, randomize);
  sequencer = CVNoteSequencer::create(notes, C, Key::SCALE_MINOR, DUE_SPI_4822_02, DUE_IN_A10);


  sequencer->setgate(Gate::create(DUE_OUT_D15, 75));
 
  democlock->registerDevice(sequencer);
  
  EventManager::registerDevice(democlock);  
  
  
  /* Intialize our six gates */
  for (int i = 0; i < noOfDrumOutputs; i++)
  {
    /* Set up a gate for each output with a duration of 20ms */
    gates[i] = Gate::create(outputs[i], 10);
    EventManager::registerDevice(gates[i]);
  }
}

////////////////////////////////////////////////////////////////////////

void loop() {

  if (EventManager::getT() % 10 == 0)
  {
    /* AnalogReads are relatively expensive and slow, so only do them every 10ms */
    int setting = analogReadmV(INPUT_NOTE);
    
    if (setting > 2000) 
    {
      sequencer->setKey(E);
    }
    else
    {
      sequencer->setKey(C);
    }
  }
  
  // out1->outputCV(100 * noteOut);

  if (EventManager::getT() >= nexttime) {
    nexttime += 100 + ((5000 - analogReadmV(INPUT_TEMPO, 0, 5000)) / 20);   
            
    int drumProgram = (noOfDrumPrograms > 1) ? (analogReadmV(INPUT_PROGRAM, 0, 4900) / (5000 / noOfDrumPrograms)) : 0;

    column = (column + 1) % 16;

    for (int row = 0; row < noOfDrumSteps; row++) { 

      randValueSubtract = Entropy::getValue(0, 5000);
      if (randValueSubtract > analogReadmV(INPUT_SUBTRACT, 0, 5000)) {

        if (drums[drumProgram][row][column] == 1) {
          digitalWrite(outputs[row], HIGH);
        }

        isThisATrigger[row] = drums[drumProgram][row][column]; 

        randValueAdd = Entropy::getValue(0, 5000);

        if (randValueAdd < analogReadmV(INPUT_ADD, 0, 5000)) {
          digitalWrite(outputs[row], HIGH);
          isThisATrigger[row] = 1;
        }
      
        if (isThisATrigger[row] == 1) { 
          if (anslagEveryOther[row] == 1) { 
            gates[row]->reset();
            anslagEveryOther[row] = 0; 
          }
          else {
            anslagEveryOther[row] = 1;  
          }
        } // gate check
      } // subtract
    } // row
  } // column
  EventManager::loop();
}