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

const int nw2s::NOTE_CV_SIZE = 61;

const ScaleNote nw2s::NOTE_NOT_FOUND = { -1, -1, -1, -1 };

const ScaleNote nw2s::SCALE_NOTES[nw2s::NOTE_CV_SIZE] = {
	
	{ 0, 0, 0, 0, 0, 1 },
	{ 1, 4, 67, 9, 0, 2 },
	{ 2, 8, 133, 26, 0, 3 },
	{ 3, 12, 200, 43, 0, 4 },
	{ 4, 16, 267, 60, 0, 5 },
	{ 5, 20, 333, 77, 0, 6 },
	{ 6, 24, 400, 94, 0, 7 },
	{ 7, 28, 467, 111, 0, 8 },
	{ 8, 32, 533, 128, 0, 9 },
	{ 9, 36, 600, 145, 0, 10 },
	{ 10, 40, 667, 162, 0, 11 },
	{ 11, 44, 733, 180, 0, 12 },
	{ 12, 48, 800, 197, 1, 1 },
	{ 13, 52, 867, 214, 1, 2 },
	{ 14, 56, 933, 231, 1, 3 },
	{ 15, 60, 1000, 248, 1, 4 },
	{ 16, 64, 1067, 265, 1, 5 },
	{ 17, 68, 1133, 282, 1, 6 },
	{ 18, 72, 1200, 299, 1, 7 },
	{ 19, 76, 1267, 316, 1, 8 },
	{ 20, 80, 1333, 333, 1, 9 },
	{ 21, 84, 1400, 350, 1, 10 },
	{ 22, 88, 1467, 367, 1, 11 },
	{ 23, 92, 1533, 384, 1, 12 },
	{ 24, 96, 1600, 401, 2, 1 },
	{ 25, 100, 1667, 418, 2, 2 },
	{ 26, 104, 1733, 435, 2, 3 },
	{ 27, 108, 1800, 452, 2, 4 },
	{ 28, 112, 1867, 469, 2, 5 },
	{ 29, 116, 1933, 486, 2, 6 },
	{ 30, 120, 2000, 503, 2, 7 },
	{ 31, 124, 2067, 521, 2, 8 },
	{ 32, 128, 2133, 538, 2, 9 },
	{ 33, 132, 2200, 555, 2, 10 },
	{ 34, 136, 2267, 572, 2, 11 },
	{ 35, 140, 2333, 589, 2, 12 },
	{ 36, 144, 2400, 606, 3, 1 },
	{ 37, 148, 2467, 623, 3, 2 },
	{ 38, 152, 2533, 640, 3, 3 },
	{ 39, 156, 2600, 657, 3, 4 },
	{ 40, 160, 2667, 674, 3, 5 },
	{ 41, 164, 2733, 691, 3, 6 },
	{ 42, 168, 2800, 708, 3, 7 },
	{ 43, 172, 2867, 725, 3, 8 },
	{ 44, 176, 2933, 742, 3, 9 },
	{ 45, 180, 3000, 759, 3, 10 },
	{ 46, 184, 3067, 776, 3, 11 },
	{ 47, 188, 3133, 793, 3, 12 },
	{ 48, 192, 3200, 810, 4, 1 },
	{ 49, 196, 3267, 827, 4, 2 },
	{ 50, 200, 3333, 844, 4, 3 },
	{ 51, 204, 3400, 862, 4, 4 },
	{ 52, 208, 3467, 879, 4, 5 },
	{ 53, 212, 3533, 896, 4, 6 },
	{ 54, 216, 3600, 913, 4, 7 },
	{ 55, 220, 3667, 930, 4, 8 },
	{ 56, 224, 3733, 947, 4, 9 },
	{ 57, 228, 3800, 964, 4, 10 },
	{ 58, 232, 3867, 981, 4, 11 },
	{ 59, 236, 3933, 998, 4, 12 },
	{ 60, 240, 4000, 1015, 5, 1 },
};


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
			int cvlocation = noteindexes[noteindex] + this->rootnote + (octaveindex * 11);
			
			/* Once we're past the bounds, we're done */
			if (cvlocation >= nw2s::NOTE_CV_SIZE) break;

			/* Skip the negative locations */
			if (cvlocation > -1)
			{										
				ScaleNote note = { SCALE_NOTES[cvlocation].index, SCALE_NOTES[cvlocation].cv8, SCALE_NOTES[cvlocation].cv12, SCALE_NOTES[cvlocation].cvin, octaveindex, noteindex + 1 };
			
				Serial.print("\nPushing scale note:");
				Serial.print(" loc: " + String(cvlocation));
				Serial.print(" index: " + String(SCALE_NOTES[cvlocation].index));
				Serial.print(" cv8: " + String(SCALE_NOTES[cvlocation].cv8));
				Serial.print(" cv12: " + String(SCALE_NOTES[cvlocation].cv12));
				Serial.print(" cvin: " + String(SCALE_NOTES[cvlocation].cvin));
				Serial.print(" octave: " + String(octaveindex));
				Serial.print(" degree: " + String(noteindex + 1));
			
				this->notes.push_back(note);
			}
		}
	}
}


