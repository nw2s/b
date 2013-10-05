
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

	extern const int NOTE_CV_SIZE;
	extern const int NOTE_CV[61];

	struct ScaleNote 
	{
		int cv;
		int cv8;
		int cv12;
		int randomweight;
		int octave;
		int degree;
	};

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
		int randomweight;
		

		void initScaleMeta(ScaleType scaletype, NoteName rootnote);
		void initScaleNotes(int notesperoctave, int noteindexes[], int randomweights[]);		
};

#endif
