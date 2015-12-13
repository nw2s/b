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


/*

	Stack of currently pressed keys.

	Currently pressed keys are stored as a linked list. The linked list is used
	as a LIFO stack to allow monosynth-like behaviour. An example of such
	behaviour is:
	player presses and holds C4-> C4 is played.
	player presses and holds C5 (while holding C4) -> C5 is played.
	player presses and holds G4 (while holding C4&C5)-> G4 is played.
	player releases C5 -> G4 is played.
	player releases G4 -> C4 is played.

	The nodes used in the linked list are pre-allocated from a pool of 16
	nodes, so the "pointers" (to the root element for example) are not actual
	pointers, but indices of an element in the pool.

	Additionally, an array of pointers is stored to allow random access to the
	n-th note, sorted by ascending order of pitch (for arpeggiation).

*/

#ifndef MIDIPAL_NOTE_STACK_H_
#define MIDIPAL_NOTE_STACK_H_

#include <list>
#include <stdint.h>

namespace nw2s
{
	struct NoteListEntry
	{
		uint32_t note;
		uint32_t velocity;
		bool latchRelease;
	};

	typedef std::list<NoteListEntry> NoteList;

	class NoteStack;
}

class nw2s::NoteStack
{
	public:

		NoteStack() { }
		void init() { clear(); }

		void noteOn(uint32_t note, uint32_t velocity);
		void noteOn(uint32_t note, uint32_t velocity, bool latched);
		void noteOff(uint32_t note);
		void noteLatchRelease(uint32_t note);
		void clear();
		void clearLatched();
		void sort();
		
		uint32_t getSize() { return this->pool.size(); }
		NoteListEntry mostRecentNote() { return this->pool.back(); }
		NoteListEntry leastRecentNote() { return this->pool.front(); }
		NoteListEntry getNote(uint32_t n); 

	private:

		NoteList pool;
};

#endif
