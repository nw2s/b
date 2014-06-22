#include <Key.h>
#include <EventManager.h>
#include <IO.h>
#include <Trigger.h>
#include <Clock.h>
#include <Slew.h>
#include <Sequence.h>
#include <SPI.h>
#include <SD.h>
#include <Wire.h>
#include <aJSON.h>

using namespace nw2s;

const PinAnalogIn INPUT_SETTING = DUE_IN_A01;

/* Keep a static reference to the sequencer so we can modify it outside of setup() */
NoteSequencer* sequencer;

void setup() 
{

	Serial.begin(19200);
	Serial.print("Starting...\n");

	EventManager::initialize();

	/* Fixed clock running at 75BPM on an Ardcore D0 */
	// FixedClock* fixedclock = FixedClock::create(120, 16);
	Clock* democlock = VariableClock::create(25, 525, DUE_IN_A00, 16);


	/* Note that in the key of E, this sequence will hit the 5V ceiling somewhere around {4,5}, so expect some of those last notes to not be in tune */
	/* Set up the note data for the sequence */
	SequenceNote notelist[34] = { {1,1}, {1,3}, {1,5}, {1,1}, {1,3}, {1,5}, {1,1}, {1,5}, 
								  {2,1}, {2,3}, {2,5}, {2,1}, {2,3}, {2,5}, {2,1}, {2,5}, 
								  {3,1}, {3,3}, {3,5}, {3,1}, {3,3}, {3,5}, {3,1}, {3,5}, 
								  {4,1}, {4,3}, {4,5}, {4,1}, {4,3}, {4,5}, {4,1}, {5,1} };
								
	/* Add the raw array to a vector for easier transport */
	NoteSequenceData* notes = new NoteSequenceData(notelist, notelist + 34);

	/* Build our note-based seuqnce */
	sequencer = NoteSequencer::create(notes, C, Key::SCALE_MAJOR, DIV_EIGHTH, DUE_SPI_4822_15);

	sequencer->setgate(Gate::create(DUE_OUT_D00, 75));

	democlock->registerDevice(sequencer);
	
	EventManager::registerDevice(democlock);	
}

void loop() 
{
	if (EventManager::getT() % 10 == 0)
	{
		/* AnalogReads are relatively expensive and slow, so only do them every 10ms */
		int setting = analogReadmV(INPUT_SETTING);
		
		if (setting > 2000) 
		{
			sequencer->setKey(E);
		}
		else
		{
			sequencer->setKey(C);
		}
	}
	
	EventManager::loop();	
}