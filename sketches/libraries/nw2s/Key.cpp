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
#include <Arduino.h>
#include "Key.h"

using namespace std;
using namespace nw2s;

Scale nw2s::Key::SCALE_MAJOR = { 7, { 0, 2, 4, 5, 7, 9, 10 } };
Scale nw2s::Key::SCALE_MINOR = { 7, { 0, 2, 3, 5, 7, 8, 10 } };
Scale nw2s::Key::SCALE_CHROMATIC = { 12, { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11 } };
Scale nw2s::Key::SCALE_MAJOR_PENTATONIC = { 5, { 0, 2, 4, 7, 9 } };

Scale nw2s::scaleFromName(char* name)
{
	if (strcmp("major", name) == 0)
	{
		return Key::SCALE_MAJOR;
	}
	if (strcmp("minor", name) == 0)
	{
		return Key::SCALE_MINOR;
	}
	if (strcmp("chromatic", name) == 0)
	{
		return Key::SCALE_CHROMATIC;
	}
	if (strcmp("major pentatonic", name) == 0)
	{
		return Key::SCALE_MAJOR_PENTATONIC;
	}

	/* For now, just default to chromatic if we don't find the scale */
	Serial.println("Unknown scale name " + String(name) + ". Just using chromatic instead.");
	return Key::SCALE_CHROMATIC; 
}

NoteName nw2s::noteFromName(char* name)
{
	if (strcmp("C", name) == 0)
	{
		return C;
	}
	if (strcmp("C#", name) == 0)
	{
		return C_SHARP;
	}
	if (strcmp("Db", name) == 0)
	{
		return D_FLAT;
	}
	if (strcmp("D", name) == 0)
	{
		return D;
	}
	if (strcmp("D#", name) == 0)
	{
		return D_SHARP;
	}
	if (strcmp("Eb", name) == 0)
	{
		return E_FLAT;
	}
	if (strcmp("E", name) == 0)
	{
		return E;
	}
	if (strcmp("E#", name) == 0)
	{
		return E_SHARP;
	}
	if (strcmp("F", name) == 0)
	{
		return F;
	}
	if (strcmp("F#", name) == 0)
	{
		return F_SHARP;
	}
	if (strcmp("Gb", name) == 0)
	{
		return G_FLAT;
	}
	if (strcmp("G", name) == 0)
	{
		return G;
	}
	if (strcmp("G#", name) == 0)
	{
		return G_SHARP;
	}
	if (strcmp("Ab", name) == 0)
	{
		return A_FLAT;
	}
	if (strcmp("A", name) == 0)
	{
		return A;
	}
	if (strcmp("A#", name) == 0)
	{
		return A_SHARP;
	}
	if (strcmp("Bb", name) == 0)
	{
		return B_FLAT;
	}
	if (strcmp("B", name) == 0)
	{
		return B;
	}
	if (strcmp("B#", name) == 0)
	{
		return B_SHARP;
	}

	/* For now, just default to C if we don't find the note */
	Serial.println("Unknown note name " + String(name) + ". Just using C instead.");
	return C; 
}

Key::Key(Scale scale, NoteName rootnote)
{
	this->scale = scale;
	this->rootnote = rootnote;
}

int Key::getNoteMillivolt(int octave, int degree)
{
	int rootmV = (octave * 1000) + SEMITONE_MV[this->rootnote];
	int degreemV = SEMITONE_MV[(degree - 1) % this->scale.length];
	
	return rootmV + degreemV;
}

int Key::quantizeOutput(int cv)
{
	return 0;
	// /* Output quantizing is done when we're going from 0-4096 */
	// for (int i = this->notes.size() - 1; i >= 0; i--)
	// {
	// 	//TODO: giving it a few bits of slop - good idea?
	// 	if (cv >= this->notes[i].cv - 10) return this->notes[i];
	// }
	// 
	// return this->notes[0];
}

