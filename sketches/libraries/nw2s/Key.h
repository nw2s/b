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

#ifndef Keys_h
#define Keys_h

#include <iterator>
#include <vector>

namespace nw2s
{
	enum ScaleType
	{
		MAJOR,
		MINOR,
		CHROMATIC,
	};	

	enum NoteName
	{
		C = 0,
		C_SHARP = 1,
		D_FLAT = 1,
		D = 2,
		D_SHARP = 3,
		E_FLAT = 3,
		E = 4,
		F = 5,
		F_SHARP = 6,
		G_FLAT = 6,
		G = 7,
		G_SHARP = 8,
		A_FLAT = 8,
		A = 9,
		A_SHARP = 10,
		B_FLAT = 10,
		B = 11,
	};

	struct ScaleNote 
	{
		int index;
		int cv8;
		int cv12;
		int cvin;
		int octave;
		int degree;
	};

	extern const int NOTE_CV_SIZE;
	extern const ScaleNote SCALE_NOTES[61];
	extern const ScaleNote NOTE_NOT_FOUND;

	typedef std::vector<ScaleNote> ScaleNotes; 

	class Key;
}

class nw2s::Key 
{
	public:
		Key(ScaleType scaletype, NoteName rootnote);
		
		ScaleNote& operator [] (const int index);
		
		size_t getNoteCount();
		NoteName getRoot();
		ScaleNote getNote(int octave, int degree);
		ScaleNote getRandomNote();
		ScaleNote getRandomNote(int min, int max);

	private:
		ScaleType scaletype;			
		NoteName rootnote;			 
		ScaleNotes notes;
		
		void initScaleMeta(ScaleType scaletype, NoteName rootnote);
		void initScaleNotes(int notesperoctave, int noteindexes[]);		
};

#endif
