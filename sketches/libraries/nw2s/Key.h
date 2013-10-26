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

#ifdef __AVR__
#define CVTYPE unsigned char
#else
#define CVTYPE int
#endif


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

	struct ScaleCV
	{
		CVTYPE cv;
		int cvin;
	};

	struct ScaleNote 
	{
		char index;
		CVTYPE cv;
		int cvin;
		char octave;
		char degree;
	};

	typedef std::vector<ScaleNote> ScaleNotes; 

	static const int NOTE_CV_SIZE = 61;
	static const ScaleNote NOTE_NOT_FOUND = { -1, -1, -1, -1 };;

#ifdef __AVR__

	static const ScaleCV SCALE_NOTES[NOTE_CV_SIZE] = {
	
		{ 0, 0 },
		{ 4, 9 },
		{ 8, 26 },
		{ 12, 43 },
		{ 16, 60 },
		{ 20, 77 },
		{ 24, 94 },
		{ 28, 111 },
		{ 32, 128 },
		{ 36, 145 },
		{ 40, 162 },
		{ 44, 180 },
		{ 48, 197 },
		{ 52, 214 },
		{ 56, 231 },
		{ 60, 248 },
		{ 64, 265 },
		{ 68, 282 },
		{ 72, 299 },
		{ 76, 316 },
		{ 80, 333 },
		{ 84, 350 },
		{ 88, 367 },
		{ 92, 384 },
		{ 96, 401 },
		{ 100, 418 },
		{ 104, 435 },
		{ 108, 452 },
		{ 112, 469 },
		{ 116, 486 },
		{ 120, 503 },
		{ 124, 521 },
		{ 128, 538 },
		{ 132, 555 },
		{ 136, 572 },
		{ 140, 589 },
		{ 144, 606 },
		{ 148, 623 },
		{ 152, 640 },
		{ 156, 657 },
		{ 160, 674 },
		{ 164, 691 },
		{ 168, 708 },
		{ 172, 725 },
		{ 176, 742 },
		{ 180, 759 },
		{ 184, 776 },
		{ 188, 793 },
		{ 192, 810 },
		{ 196, 827 },
		{ 200, 844 },
		{ 204, 862 },
		{ 208, 879 },
		{ 212, 896 },
		{ 216, 913 },
		{ 220, 930 },
		{ 224, 947 },
		{ 228, 964 },
		{ 232, 981 },
		{ 236, 998 },
		{ 240, 1015 },
	};


#else

	static const ScaleCV SCALE_NOTES[NOTE_CV_SIZE] = {
	
		{ 0, 0 },
		{ 67, 9 },
		{ 133, 26 },
		{ 200, 43 },
		{ 267, 60 },
		{ 333, 77 },
		{ 400, 94 },
		{ 467, 111 },
		{ 533, 128 },
		{ 600, 145 },
		{ 667, 162 },
		{ 733, 180 },
		{ 800, 197 },
		{ 867, 214 },
		{ 933, 231 },
		{ 1000, 248 },
		{ 1067, 265 },
		{ 1133, 282 },
		{ 1200, 299 },
		{ 1267, 316 },
		{ 1333, 333 },
		{ 1400, 350 },
		{ 1467, 367 },
		{ 1533, 384 },
		{ 1600, 401 },
		{ 1667, 418 },
		{ 1733, 435 },
		{ 1800, 452 },
		{ 1867, 469 },
		{ 1933, 486 },
		{ 2000, 503 },
		{ 2067, 521 },
		{ 2133, 538 },
		{ 2200, 555 },
		{ 2267, 572 },
		{ 2333, 589 },
		{ 2400, 606 },
		{ 2467, 623 },
		{ 2533, 640 },
		{ 2600, 657 },
		{ 2667, 674 },
		{ 2733, 691 },
		{ 2800, 708 },
		{ 2867, 725 },
		{ 2933, 742 },
		{ 3000, 759 },
		{ 3067, 776 },
		{ 3133, 793 },
		{ 3200, 810 },
		{ 3267, 827 },
		{ 3333, 844 },
		{ 3400, 862 },
		{ 3467, 879 },
		{ 3533, 896 },
		{ 3600, 913 },
		{ 3667, 930 },
		{ 3733, 947 },
		{ 3800, 964 },
		{ 3867, 981 },
		{ 3933, 998 },
		{ 4000, 1015 },
	};


#endif

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
