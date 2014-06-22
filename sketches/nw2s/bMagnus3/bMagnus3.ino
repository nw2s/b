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
NoteSequencer* sequencer1;
NoteSequencer* sequencer2;
NoteSequencer* sequencer3;

void setup() 
{

	Serial.begin(19200);
	Serial.print("Starting...\n");

	EventManager::initialize();

	/* Set up the note data for the sequence */
	SequenceNote notelist1[4] = { {1,1}, {1,4}, {1,1}, {1,5} };
	SequenceNote notelist2[4] = { {1,5}, {2,1}, {1,5}, {2,2} };
	SequenceNote notelist3[5] = { {3,1}, {3,3}, {3,2}, {3,7}, {3,5} };
								
	/* Add the raw array to a vector for easier transport */
	NoteSequenceData* notes1 = new NoteSequenceData(notelist1, notelist1 + 4);
	NoteSequenceData* notes2 = new NoteSequenceData(notelist2, notelist2 + 4);
	NoteSequenceData* notes3 = new NoteSequenceData(notelist3, notelist3 + 5);

	/* Build our note-based seuqnce */
	sequencer1 = TriggerNoteSequencer::create(notes1, C, Key::SCALE_MAJOR, DUE_IN_D0, DUE_SPI_4822_15);
	sequencer2 = TriggerNoteSequencer::create(notes2, C, Key::SCALE_MAJOR, DUE_IN_D0, DUE_SPI_4822_14);
	sequencer3 = TriggerNoteSequencer::create(notes3, C, Key::SCALE_MAJOR, DUE_IN_D0, DUE_SPI_4822_13, true);

	sequencer1->setgate(Gate::create(DUE_OUT_D00, 75));

	EventManager::registerDevice(sequencer1);
	EventManager::registerDevice(sequencer2);
	EventManager::registerDevice(sequencer3);
}

void loop() 
{
	if (EventManager::getT() % 10 == 0)
	{
		/* AnalogReads are relatively expensive and slow, so only do them every 10ms */
		int setting = analogReadmV(INPUT_SETTING);
		
		if (setting > 2000) 
		{
			sequencer1->setKey(E);
			sequencer2->setKey(E);
			sequencer3->setKey(E);
		}
		else
		{
			sequencer1->setKey(C);
			sequencer2->setKey(C);
			sequencer3->setKey(C);
		}
	}
	
	EventManager::loop();	
}