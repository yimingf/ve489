#ifndef __FILE_H__
#define __FILE_H__

#include 	<iostream>
#include 	<fstream>
#include 	<string>
#include 	<cstring>
#include 	<openssl/sha.h>

#include 	"fragment.h"

class File {
// the "file" class has no interference w/network tX.

protected:

	char 				name[64];
	// the name of the file.
	unsigned int 		length;
	// the length of the complete file.
	unsigned int 		numPieces;
	// number of pieces;
	bool 				isDone;
	// if the file is completely on the computer.
	uint8_t*			bitfield;
	// DYNAMIC

public:

	File(char* filename);
	// if complete, set bitfield to all-1.
	File(char* filename, int len);
	// if not complete, set the length and set bitfield to all-*-1*.
	~File();
	// the destructor.

	unsigned int 		getLength();
	unsigned int 		getNumPieces();
	bool 				isComplete();
	uint8_t*			getBitfield();
	// the getters.
	
	void 				setAllBitfield(int numPieces, int isComplete);
	void 				setBitfield(int numPiece, int isComplete);
	// the setters.

	struct fragment 	readFragment(int index);
	void				saveFragment(struct fragment fr);
	// save the received fragment somewhere.
	void 				arrange();
	// when bitfield set to all-1 (meaning almost complete),
	// modify the temp into a complete file and
	// set isComplete() to true.
};

#endif