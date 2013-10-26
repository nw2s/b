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

#include <iterator>
#include <vector>
#include <stdexcept>

#ifdef __AVR__
#include <pnew.cpp>
#endif

#include <Arduino.h>

#include "Key.h"

using namespace std;
using namespace nw2s;


/* Key CLASS IMPLEMENTATION */
/* Key CLASS IMPLEMENTATION */
/* Key CLASS IMPLEMENTATION */


/* PUBLIC METHODS AND OPERATORS */

Key::Key(ScaleType scaletype, NoteName rootnote)
{
	this->scaletype = scaletype;
	this->rootnote = rootnote;
	
	this->initScaleMeta(scaletype, rootnote);	
}

ScaleNote& Key::operator [] (const int index)
{
	return this->notes[index];
}

size_t Key::getNoteCount()
{
	return this->notes.size();
}

NoteName Key::getRoot()
{
	return this->rootnote;
}

ScaleNote Key::getNote(int octave, int degree)
{
	for (int i = 0; i < notes.size(); i++)
	{
		if (notes[i].octave == octave && notes[i].degree == degree) return notes[i];
	}
	
	return nw2s::NOTE_NOT_FOUND;
}

ScaleNote Key::getRandomNote()
{
	return this->notes[random(this->notes.size())];
}

/* PRIVATE METHODS */

void Key::initScaleMeta(ScaleType scaletype, NoteName rootnote)
{	
	switch (scaletype)
	{
		case MAJOR:
		{
			int noteindexes[] = { 0, 2, 4, 5, 7, 9, 10 };
			initScaleNotes(7, noteindexes);
			break;
		}
			
		case MINOR:
		{
			int noteindexes[] = { 0, 2, 3, 5, 7, 8, 10 };
			initScaleNotes(7, noteindexes);
			break;
		}
					
		case CHROMATIC:
		{
			int noteindexes[] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };
			initScaleNotes(11, noteindexes);
			break;
		}
	}
}

void Key::initScaleNotes(int notesperoctave, int noteindexes[])
{
	this->notes = nw2s::ScaleNotes();
		
	for (int octaveindex = -1; octaveindex < 6; octaveindex++)
	{
		for (int noteindex = 0; noteindex < notesperoctave; noteindex++)
		{
			int cvlocation = noteindexes[noteindex] + this->rootnote + (octaveindex * 12);
			
			/* Once we're past the bounds, we're done */
			if (cvlocation >= nw2s::NOTE_CV_SIZE) break;

			/* Skip the negative locations */
			if (cvlocation > -1)
			{	
				ScaleNote note = { noteindex, SCALE_NOTES[cvlocation].cv, SCALE_NOTES[cvlocation].cvin, octaveindex, noteindex + 1 };
				this->notes.push_back(note);
			}
		}
	}
}


