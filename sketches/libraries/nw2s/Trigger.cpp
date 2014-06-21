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


#include "Trigger.h"
#include "IO.h"
#include "aJson.h"

using namespace nw2s;

Trigger* Trigger::create(PinDigitalOut output, int clock_division)
{	
	return new Trigger(output, clock_division);
}

Trigger* Trigger::create(aJsonObject* data)
{
	return NULL;
	// static const char randomizeNodeName[] = "randomize";
	// static const char gateNodeName[] = "gateOutput";
	// static const char durationNodeName[] = "gateLength";
	//
	// bool randomize = getBoolFromJSON(data, randomizeNodeName, false);
	// NoteSequenceData* notes = getNotesFromJSON(data);
	// Scale scale = getScaleFromJSON(data);
	// NoteName root = getRootFromJSON(data);
	// int clockdivision = getDivisionFromJSON(data);
	// PinAnalogOut output = getAnalogOutputFromJSON(data);
	// PinDigitalOut gatePin = getDigitalOutputFromJSON(data, gateNodeName);
	// int gateDuration = getIntFromJSON(data, durationNodeName, 20, 1, 1000);
	//
	// NoteSequencer* seq = new NoteSequencer(notes, root, scale, clockdivision, output, randomize);
	//
	// if (gatePin != DIGITAL_OUT_NONE) seq->setgate(Gate::create(gatePin, gateDuration));
	//
	// return seq;
}

Trigger::Trigger(PinDigitalOut output, int clock_division)
{
	this->clock_division = clock_division;
	
	/* Make sure the pin is low */
	pinMode(output, OUTPUT);
	this->output = output;
	this->state = LOW;
	this->t_start = 0;
	digitalWrite(this->output, LOW);		
}

void Trigger::reset()
{
	/* Reset turns the trigger on */
	this->t_start = 0;
	this->state = HIGH;
	digitalWrite(this->output, HIGH);
}

void Trigger::timer(unsigned long t)
{	
	/* If the state is low, nothing else to do */
	if (this->state == LOW) return;
	
	if (this->t_start == 0)
	{
		this->t_start = t;
	}
	else
	{
		if ((this->state == HIGH) && (t - this->t_start > TRIGGER_TIME))
		{
			this->state = LOW;
			digitalWrite(this->output, LOW);
		}	
	}	
}



