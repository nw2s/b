
#include <iterator>
#include <vector>
#include <stdexcept>
#include <pnew.cpp>

#include <Arduino.h>

#include "Key.h"

using namespace std;
using namespace nw2s;

const int nw2s::NOTE_CV_SIZE = 61;
const int nw2s::NOTE_CV[nw2s::NOTE_CV_SIZE] = {
  0,   9,   26,  43,  60,  77,  94,  111, 128, 145, 162, 180, 
  197, 214, 231, 248, 265, 282, 299, 316, 333, 350, 367, 384, 
  401, 418, 435, 452, 469, 486, 503, 521, 538, 555, 572, 589, 
  606, 623, 640, 657, 674, 691, 708, 725, 742, 759, 776, 793, 
  810, 827, 844, 862, 879, 896, 913, 930, 947, 964, 981, 998, 
  1015};

const ScaleNote nw2s::NOTE_NOT_FOUND = { -1, -1, -1, -1 };


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
	long randomval = random(this->randomweight);
	int randomsum = 0;
	
	for (int i = 0; i < this->notes.size(); i++)
	{
		randomsum += this->notes[i].randomweight;
		
		if (randomsum > randomval) return this->notes[i];
	}
	
	return this->notes[this->notes.size() - 1];
}

ScaleNote Key::getRandomNote(int min, int max)
{
	//TODO: normalize inputs on available notes
	
	int rangeweight = 0;
	int randomsum = 0;

	for (int i = min; i < max && i < this->notes.size(); i++)
	{
		rangeweight += this->notes[i].randomweight;
	}
	
	long randomval = random(rangeweight);

	for (int i = min; i < max && i < this->notes.size(); i++)
	{
		randomsum += this->notes[i].randomweight;
		
		if (randomsum > randomval) return this->notes[i];
	}
	
	return this->notes[this->notes.size() - 1];
}



/* PRIVATE METHODS */

void Key::initScaleMeta(ScaleType scaletype, NoteName rootnote)
{	
	switch (scaletype)
	{
		case MAJOR:
		{
			int noteindexes[] = { 0, 2, 4, 5, 7, 9, 11 };
			int randomweights[] = { 1, 1, 1, 1, 1, 1, 1 };
			initScaleNotes(7, noteindexes, randomweights);
			break;
		}
			
		case MINOR:
		{
			int noteindexes[] = { 0, 2, 3, 5, 7, 8, 10 };
			int randomweights[] = { 1, 1, 1, 1, 1, 1, 1 };
			initScaleNotes(7, noteindexes, randomweights);
			break;
		}
					
		case CHROMATIC:
		{
			int noteindexes[] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11 };
			int randomweights[] = { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 };
			initScaleNotes(11, noteindexes, randomweights);
			break;
		}
	}
}

void Key::initScaleNotes(int notesperoctave, int noteindexes[], int randomweights[])
{
	this->notes = nw2s::ScaleNotes();
	this->randomweight = 0;
		
	for (int octaveindex = -1; octaveindex < 6; octaveindex++)
	{
		for (int noteindex = 0; noteindex < notesperoctave; noteindex++)
		{
			int cvlocation = noteindex + this->rootnote + (octaveindex * 11);
			
			/* Once we're past the bounds, we're done */
			if (cvlocation >= nw2s::NOTE_CV_SIZE) break;

			/* Skip the negative locations */
			if (cvlocation > -1)
			{										
				ScaleNote note = { NOTE_CV[cvlocation], cvlocation << 2, 0, randomweights[noteindex], octaveindex, noteindex + 1 };
			
				this->notes.push_back(note);
				this->randomweight += note.randomweight;
			}
		}
	}
}


