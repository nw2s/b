
#include "EventManager.h"
#include <Arduino.h>

using namespace std;
using namespace nw2s;

volatile unsigned long EventManager::t = 0UL;
vector<TimeBasedDevice *> EventManager::timedevices;

void EventManager::initialize()
{
	/* Let's seed the random number generator just to be sure */
	randomSeed(analogRead(0) + analogRead(1) + analogRead(2) + analogRead(3) + digitalRead(0) + digitalRead(1) + digitalRead(2) + digitalRead(3) + millis());
}

void EventManager::loop()
{
	/* This gets run every loop() call, but we only want to */
	/* fire events if the clock has changed. */
	unsigned long current_time = millis();
	
	if (t != current_time)
	{
		t = current_time;
				
		for (int i = 0; i < EventManager::timedevices.size(); i++)
		{
			TimeBasedDevice *device = EventManager::timedevices[i];
			EventManager::timedevices[i]->timer(EventManager::t);	
		}
	}
}

void EventManager::registerdevice(TimeBasedDevice *device)
{
	timedevices.push_back(device);
}

