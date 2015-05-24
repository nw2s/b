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


	This was borrowed from the midipal source code for use in the nw2s-b project.
	It's been modified to run on a 32-bit Cortex machine and use the std::list as 
	its linked list implementation.

	Original Copyright 2009 Olivier Gillet.
	Author: Olivier Gillet (ol.gillet@gmail.com)
*/


#include "NoteStack.h"

using namespace nw2s;

void NoteStack::noteOn(uint32_t note, uint32_t velocity)
{
	/* See if this note is already in the list */
	for (NoteList::iterator i = this->pool.begin(); i != this->pool.end(); ++i)
	{
		if ((*i).note == note)
		{
			/* Remove it so we can re-add at the end */
			this->pool.erase(i);
		}
	}
	
	NoteListEntry entry = { note, velocity };
		
	this->pool.push_back(entry);
	
	if (this->pool.size() > 16)
	{
		this->pool.pop_front();
	}
}


void NoteStack::noteOff(uint32_t note) 
{
	for (NoteList::iterator i = this->pool.begin(); i != this->pool.end(); i++)
	{
		if ((*i).note == note)
		{
			this->pool.erase(i);
			return;
		}
	}
}
	
NoteListEntry NoteStack::getNote(uint32_t n) 
{ 
	if (n >= this->pool.size())
	{
		n = this->pool.size() - 1;
	}
	
	NoteList::iterator iterator = this->pool.begin();
	
	for (uint32_t i = 0; i < (n - 1); i++)
	{
		iterator++;
	}

	return *iterator;
}
	

void NoteStack::clear()
{
	this->pool.clear();
}
