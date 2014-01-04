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

#include "ShiftRegister.h"
#include "Entropy.h"


//using namepsace nw2s;

nw2s::RandomLoopingShiftRegister* RandomLoopingShiftRegister::create(int size, PinAnalogIn control, int clockdivision)
{
	return new RandomLoopingShiftRegister(size, control, clockdivision);
}

RandomLoopingShiftRegister::RandomLoopingShiftRegister(int size, PinAnalogIn control, int clockdivision)
{
	this->nextCV = 0;
	this->controlpin = control;
	this->clock_division = clockdivision;
	this->next_or_gate = false;
	this->next_and_gate = false;
	this->next_or_trigger = false;
	this->next_and_trigger = false;
	this->next_sequencercv = 0;
	
	/* Fill up the shiftregister with random bits */
	for (int i = 0; i < size; i++)
	{
		shiftregister.push_back(Entropy::getBit());
	}
	
	this->cvout = NULL;
	this->noteout = NULL;
	this->key = NULL;
	this->sequencercvout = NULL;
	this->sequencernoteout = NULL;
	this->sequencerscale = DUE_IN_A_NONE;
	
	this->or_gate = NULL;
	this->and_gate = NULL;
	this->or_trigger = NULL;
	this->and_trigger = NULL;
	
	this->or_gate_terms.clear();
	this->and_gate_terms.clear();
	this->or_trigger_terms.clear();
	this->and_trigger_terms.clear();
	
	for (int i = 0; i < 8; i++) 
	{
		this->trigger[i] = NULL;
		this->gate[i] = NULL;
		this->sequencerinput[i] = DUE_IN_A_NONE;
	}
}

void RandomLoopingShiftRegister::setCVOut(PinAnalogOut pinout)
{
	this->cvout = AnalogOut::create(pinout);
}

void RandomLoopingShiftRegister::setNoteOut(PinAnalogOut pinout)
{
	this->noteout = AnalogOut::create(pinout);
}

void RandomLoopingShiftRegister::setSequencerCVOut(PinAnalogOut pinout)
{
	this->sequencercvout = AnalogOut::create(pinout);
}

void RandomLoopingShiftRegister::setSequencerNoteOut(PinAnalogOut pinout)
{
	this->sequencernoteout = AnalogOut::create(pinout);
}

void RandomLoopingShiftRegister::setSequencerInputs(PinAnalogIn p1, PinAnalogIn p2, PinAnalogIn p3, PinAnalogIn p4, PinAnalogIn p5, PinAnalogIn p6, PinAnalogIn p7, PinAnalogIn p8)
{
	this->sequencerinput[0] = p1;
	this->sequencerinput[1] = p2;
	this->sequencerinput[2] = p3;
	this->sequencerinput[3] = p4;
	this->sequencerinput[4] = p5;
	this->sequencerinput[5] = p6;
	this->sequencerinput[6] = p7;
	this->sequencerinput[7] = p8;
}

void RandomLoopingShiftRegister::setSequencerScaleInput(PinAnalogIn pin)
{
	this->sequencerscale = pin;
}

void RandomLoopingShiftRegister::setDelayedCVOut(PinAnalogOut pinout, int ticks)
{
	this->delayedcvout = AnalogOut::create(pinout);
	
	for (int i = 0; i <= ticks; i++) this->cvdelayline.push_back(0);
}

void RandomLoopingShiftRegister::setDelayedNoteOut(PinAnalogOut pinout, int ticks)
{
	this->delayednoteout = AnalogOut::create(pinout);
	
	for (int i = 0; i <= ticks; i++) this->notedelayline.push_back(this->key->getNote(0, 1));
}

void RandomLoopingShiftRegister::setTriggerOut(int position, PinDigitalOut pinout)
{
	this->trigger[position - 1] = Trigger::create(pinout, 0);
}

void RandomLoopingShiftRegister::setGateOut(int position, PinDigitalOut pinout, int duration)
{
	this->gate[position - 1] = Gate::create(pinout, duration);
}

void RandomLoopingShiftRegister::setKey(NoteName root, ScaleType scale)
{
	this->key = new Key(scale, root);
}

void RandomLoopingShiftRegister::setLogicalOrTrigger(PinDigitalOut pinout, int p1, int p2, int p3, int p4)
{
	this->or_trigger = Trigger::create(pinout, 0);
	this->or_trigger_terms.push_back(p1);
	this->or_trigger_terms.push_back(p2);
	if (p3 != -1) this->or_trigger_terms.push_back(p3);
	if (p4 != -1) this->or_trigger_terms.push_back(p4);
}

void RandomLoopingShiftRegister::setLogicalAndTrigger(PinDigitalOut pinout, int p1, int p2, int p3, int p4)
{
	this->and_trigger = Trigger::create(pinout, 0);
	this->and_trigger_terms.push_back(p1);
	this->and_trigger_terms.push_back(p2);
	if (p3 != -1) this->and_trigger_terms.push_back(p3);
	if (p4 != -1) this->and_trigger_terms.push_back(p4);
}

void RandomLoopingShiftRegister::setLogicalOrGate(PinDigitalOut pinout, int duration, int p1, int p2, int p3, int p4)
{
	this->or_gate = Gate::create(pinout, duration);
	this->or_gate_terms.push_back(p1);
	this->or_gate_terms.push_back(p2);
	if (p3 != -1) this->or_gate_terms.push_back(p3);
	if (p4 != -1) this->or_gate_terms.push_back(p4);
}

void RandomLoopingShiftRegister::setLogicalAndGate(PinDigitalOut pinout, int duration, int p1, int p2, int p3, int p4)
{
	this->and_gate = Gate::create(pinout, duration);
	this->and_gate_terms.push_back(p1);
	this->and_gate_terms.push_back(p2);
	if (p3 != -1) this->and_gate_terms.push_back(p3);
	if (p4 != -1) this->and_gate_terms.push_back(p4);
}

void RandomLoopingShiftRegister::timer(unsigned long t)
{
	//TODO: Set up slews

	for (int i = 0; i < 8; i++)
	{
		if (trigger[i] != NULL) trigger[i]->timer(t);
		if (gate[i] != NULL) gate[i]->timer(t);
	}
	
	if (this->or_trigger != NULL) this->or_trigger->timer(t);
	if (this->and_trigger != NULL) this->and_trigger->timer(t);
	if (this->or_gate != NULL) this->or_gate->timer(t);
	if (this->and_gate != NULL) this->and_gate->timer(t);
}

void RandomLoopingShiftRegister::calculate()
{
	/* Shift a bit from the back to the front, randomly negating it */
	int rawval = analogRead(this->controlpin);
	int factor = 50;
	
	/* Scale the factor based on 0-50 and 50-100 */
	if ((rawval < 1700) || (rawval > 1800))
	{
		factor = (rawval * 50) / 1700;
	}
	
	/* Decide if we flip the bit as we rotate it, and do the rotation */
	bool negate = (factor == 0) ? false : (factor == 100) ? true : (Entropy::getValue(100) <= factor) ? true : false;
	bool newfront = this->shiftregister.back() ^ negate;
	
	/* Shift all of the bits back by one */
	for (int i = this->shiftregister.size() - 1; i > 0; i--)
	{
		this->shiftregister[i] = this->shiftregister[i - 1];
	}
	
	/* And put the new one in front */
	this->shiftregister[0] = newfront;
	
	/* Calculate the CV of the current state */
	this->nextCV = getCVfromShiftRegister();

	/* CV Delay Line */
	if (this->cvdelayline.size() > 0)
	{
		for (int i = this->cvdelayline.size() - 1; i > 0; i--)
		{
			this->cvdelayline[i] = this->cvdelayline[i - 1];
		}
		
		this->cvdelayline[0] = this->nextCV;		
	}
	
	/* Note Delay Line */
	if (this->notedelayline.size() > 0)
	{
		for (int i = this->notedelayline.size() - 1; i > 0; i--)
		{
			this->notedelayline[i] = this->notedelayline[i - 1];
		}
		
		this->notedelayline[0] = key->quantizeOutput(this->nextCV);
	}

	/* Logical Or Trigger */
	if (this->or_trigger_terms.size() > 1)
	{
		bool val = false;
		
		for (int i = 0; i < this->or_trigger_terms.size(); i++) val = val || this->shiftregister[this->or_trigger_terms[i]];
		
		this->next_or_trigger = val;
	}
	
	/* Logical And Trigger */
	if (this->and_trigger_terms.size() > 1)
	{
		bool val = true;
		
		for (int i = 0; i < this->and_trigger_terms.size(); i++) val = val && this->shiftregister[this->and_trigger_terms[i]];
		
		this->next_and_trigger = val;
	}
	
	/* Logical Or Gate */
	if (this->or_gate_terms.size() > 1)
	{
		bool val = false;
		
		for (int i = 0; i < this->or_gate_terms.size(); i++) val = val || this->shiftregister[this->or_gate_terms[i]];

		this->next_or_gate = val;
	}
	
	/* Logical And Trigger */
	if (this->and_gate_terms.size() > 1)
	{
		bool val = true;
		
		for (int i = 0; i < this->and_gate_terms.size(); i++) val = val && this->shiftregister[this->and_gate_terms[i]];

		this->next_and_gate = val;
	}	

	/* Sequencer */
	if (this->sequencerinput[0] != DUE_IN_A_NONE)
	{
		int val = 0;
		
		/* Add up the values that are true */
		for (int i = 0; i < 8; i++)
		{
			if (this->shiftregister[i]) val += this->sequencerinput[i];
		}
		
		/* Scale by the max analog input (about 3500) and the scale input */
		val = (val * 4096UL) / 3500;
		if (this->sequencerscale != DUE_IN_A_NONE) val = (val * analogRead(this->sequencerscale)) / 3500;

		this->next_sequencercv = val;
	}
}

void RandomLoopingShiftRegister::reset()
{
	/* CV Output */
	if (this->cvout != NULL) this->cvout->outputCV(this->nextCV);
	if (this->noteout != NULL) this->noteout->outputNoteCV(key->quantizeOutput(this->nextCV));
	
	for (int i = 0; i < 8; i++)
	{
		if ((this->trigger[i] != NULL) && this->shiftregister[i]) this->trigger[i]->reset();
		if ((this->gate[i] != NULL) && this->shiftregister[i]) this->gate[i]->reset();
	}
	
	/* CV Delay Line */
	if (this->cvdelayline.size() > 0)
	{
		this->delayedcvout->outputCV(this->cvdelayline.back());
	}
	
	/* Note Delay Line */
	if (this->notedelayline.size() > 0)
	{
		this->delayednoteout->outputNoteCV(this->notedelayline.back());
	}
			
	/* Logical Or Trigger */
	if (this->or_trigger != NULL)
	{
		if (this->next_or_trigger) this->or_trigger->reset();
	}
	
	/* Logical And Trigger */
	if (this->and_trigger != NULL)
	{ 
		if (this->next_and_trigger) this->and_trigger->reset();
	}
	
	/* Logical Or Gate */
	if (this->or_gate != NULL)
	{
		if (this->next_or_gate) this->or_gate->reset();
	}
	
	/* Logical And Trigger */
	if (this->and_gate != NULL)
	{
		if (this->next_and_gate) this->and_gate->reset();
	}
	
	/* Sequencer */
	if (this->sequencerinput[0] != DUE_IN_A_NONE)
	{
		if (this->sequencercvout != NULL) this->sequencercvout->outputCV(this->next_sequencercv);
		if (this->sequencernoteout != NULL) this->sequencernoteout->outputNoteCV(this->key->quantizeOutput(this->next_sequencercv));		
	}
}

int RandomLoopingShiftRegister::getCVfromShiftRegister()
{
	int val = 0;
	
	for (int i = 0; i < 12; i++)
	{
		if (i < this->shiftregister.size())
		{
			val = val << 1;
			val |= this->shiftregister[i];
		}
		else
		{
			val = val << 1;
		}
	}
	
	return val;
}

