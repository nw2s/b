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

	static const ScaleNote SCALE_NOTES[NOTE_CV_SIZE] = {
	
		{ 0, 0, 0, 0, 1 },
		{ 1, 4, 9, 0, 2 },
		{ 2, 8, 26, 0, 3 },
		{ 3, 12, 43, 0, 4 },
		{ 4, 16, 60, 0, 5 },
		{ 5, 20, 77, 0, 6 },
		{ 6, 24, 94, 0, 7 },
		{ 7, 28, 111, 0, 8 },
		{ 8, 32, 128, 0, 9 },
		{ 9, 36, 145, 0, 10 },
		{ 10, 40, 162, 0, 11 },
		{ 11, 44, 180, 0, 12 },
		{ 12, 48, 197, 1, 1 },
		{ 13, 52, 214, 1, 2 },
		{ 14, 56, 231, 1, 3 },
		{ 15, 60, 248, 1, 4 },
		{ 16, 64, 265, 1, 5 },
		{ 17, 68, 282, 1, 6 },
		{ 18, 72, 299, 1, 7 },
		{ 19, 76, 316, 1, 8 },
		{ 20, 80, 333, 1, 9 },
		{ 21, 84, 350, 1, 10 },
		{ 22, 88, 367, 1, 11 },
		{ 23, 92, 384, 1, 12 },
		{ 24, 96, 401, 2, 1 },
		{ 25, 100, 418, 2, 2 },
		{ 26, 104, 435, 2, 3 },
		{ 27, 108, 452, 2, 4 },
		{ 28, 112, 469, 2, 5 },
		{ 29, 116, 486, 2, 6 },
		{ 30, 120, 503, 2, 7 },
		{ 31, 124, 521, 2, 8 },
		{ 32, 128, 538, 2, 9 },
		{ 33, 132, 555, 2, 10 },
		{ 34, 136, 572, 2, 11 },
		{ 35, 140, 589, 2, 12 },
		{ 36, 144, 606, 3, 1 },
		{ 37, 148, 623, 3, 2 },
		{ 38, 152, 640, 3, 3 },
		{ 39, 156, 657, 3, 4 },
		{ 40, 160, 674, 3, 5 },
		{ 41, 164, 691, 3, 6 },
		{ 42, 168, 708, 3, 7 },
		{ 43, 172, 725, 3, 8 },
		{ 44, 176, 742, 3, 9 },
		{ 45, 180, 759, 3, 10 },
		{ 46, 184, 776, 3, 11 },
		{ 47, 188, 793, 3, 12 },
		{ 48, 192, 810, 4, 1 },
		{ 49, 196, 827, 4, 2 },
		{ 50, 200, 844, 4, 3 },
		{ 51, 204, 862, 4, 4 },
		{ 52, 208, 879, 4, 5 },
		{ 53, 212, 896, 4, 6 },
		{ 54, 216, 913, 4, 7 },
		{ 55, 220, 930, 4, 8 },
		{ 56, 224, 947, 4, 9 },
		{ 57, 228, 964, 4, 10 },
		{ 58, 232, 981, 4, 11 },
		{ 59, 236, 998, 4, 12 },
		{ 60, 240, 1015, 5, 1 },
	};


#else

	static const ScaleNote SCALE_NOTES[NOTE_CV_SIZE] = {
	
		{ 0, 0, 0, 0, 1 },
		{ 1, 67, 9, 0, 2 },
		{ 2, 133, 26, 0, 3 },
		{ 3, 200, 43, 0, 4 },
		{ 4, 267, 60, 0, 5 },
		{ 5, 333, 77, 0, 6 },
		{ 6, 400, 94, 0, 7 },
		{ 7, 467, 111, 0, 8 },
		{ 8, 533, 128, 0, 9 },
		{ 9, 600, 145, 0, 10 },
		{ 10, 667, 162, 0, 11 },
		{ 11, 733, 180, 0, 12 },
		{ 12, 800, 197, 1, 1 },
		{ 13, 867, 214, 1, 2 },
		{ 14, 933, 231, 1, 3 },
		{ 15, 1000, 248, 1, 4 },
		{ 16, 1067, 265, 1, 5 },
		{ 17, 1133, 282, 1, 6 },
		{ 18, 1200, 299, 1, 7 },
		{ 19, 1267, 316, 1, 8 },
		{ 20, 1333, 333, 1, 9 },
		{ 21, 1400, 350, 1, 10 },
		{ 22, 1467, 367, 1, 11 },
		{ 23, 1533, 384, 1, 12 },
		{ 24, 1600, 401, 2, 1 },
		{ 25, 1667, 418, 2, 2 },
		{ 26, 1733, 435, 2, 3 },
		{ 27, 1800, 452, 2, 4 },
		{ 28, 1867, 469, 2, 5 },
		{ 29, 1933, 486, 2, 6 },
		{ 30, 2000, 503, 2, 7 },
		{ 31, 2067, 521, 2, 8 },
		{ 32, 2133, 538, 2, 9 },
		{ 33, 2200, 555, 2, 10 },
		{ 34, 2267, 572, 2, 11 },
		{ 35, 2333, 589, 2, 12 },
		{ 36, 2400, 606, 3, 1 },
		{ 37, 2467, 623, 3, 2 },
		{ 38, 2533, 640, 3, 3 },
		{ 39, 2600, 657, 3, 4 },
		{ 40, 2667, 674, 3, 5 },
		{ 41, 2733, 691, 3, 6 },
		{ 42, 2800, 708, 3, 7 },
		{ 43, 2867, 725, 3, 8 },
		{ 44, 2933, 742, 3, 9 },
		{ 45, 3000, 759, 3, 10 },
		{ 46, 3067, 776, 3, 11 },
		{ 47, 3133, 793, 3, 12 },
		{ 48, 3200, 810, 4, 1 },
		{ 49, 3267, 827, 4, 2 },
		{ 50, 3333, 844, 4, 3 },
		{ 51, 3400, 862, 4, 4 },
		{ 52, 3467, 879, 4, 5 },
		{ 53, 3533, 896, 4, 6 },
		{ 54, 3600, 913, 4, 7 },
		{ 55, 3667, 930, 4, 8 },
		{ 56, 3733, 947, 4, 9 },
		{ 57, 3800, 964, 4, 10 },
		{ 58, 3867, 981, 4, 11 },
		{ 59, 3933, 998, 4, 12 },
		{ 60, 4000, 1015, 5, 1 },
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
