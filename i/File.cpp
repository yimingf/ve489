// File.cpp

#include 	"File.h"

// implementation of file.h.

using namespace std;

File::File (char* filename) {
// if the file is complete.
	streampos 	begin, end;
	// set pathname to the path.
	ifstream file (filename);
	// open the file.
	begin 		= file.tellg();
	file.seekg (0, ios::end);
	end 		= file.tellg();
	length 		= end - begin;
	// get the length of the file.
	file.close();
	// close the file.
	strcpy(name, filename);
	// name of the file.
	numPieces 	= length / BLOCKSIZE + 1;
	// get the number of pieces.
	isDone 	= 1;
	// complete now.
	// will set bitfields to all-1.
	bitfield = new uint8_t[numPieces] {};
	setAllBitfield(numPieces, 1);
	// set the bitfields.
}

File::File (char* filename, int len) {
// if the file is not complete.
	streampos 	begin, end;
	strcpy( name, filename);
	length		= len;
	// set the pathname, filename and the length.
	numPieces 	= length / BLOCKSIZE + 1;
	isDone 	= 0;
	bitfield = new uint8_t[numPieces] {};
	setAllBitfield(numPieces, -1);
	// set the number of pieces and the bitfield.

	ofstream 	file(name);
	for (int i = 0; i < numPieces; i++) {
		char* 		buffer;
		file.seekp(0, ios::end);
		if (i != numPieces-1) {
			buffer = new char[BLOCKSIZE] {};
			file.write(buffer, BLOCKSIZE);
		} else {
			buffer = new char[length-i*BLOCKSIZE] {};
			file.write(buffer, length-i*BLOCKSIZE);
		}
		delete[] 	buffer;	
	}
	file.seekp(0, ios::beg);
	file.close();
	// create a file w/empty content.
}

File::~File() {
	delete[] 	bitfield;
}

unsigned int File::getLength () {
	return length;
}

unsigned int File::getNumPieces() {
	return numPieces;
}

bool File::isComplete() {
	if ( ! isDone ){
		for ( int i = 0; i < numPieces; i++ ) {
			if ( bitfield[i] != 1){
				return isDone;
			}
		}
		arrange();
		return isDone;
	}
	else{
		return isDone;
	}
}

uint8_t* File::getBitfield() {
	return bitfield;
}

void File::setAllBitfield (int numPieces, int isComplete) {
	for (int i = 0; i < numPieces; i++) {
		bitfield[i] = (uint8_t) isComplete;
	}
}

void File::setBitfield(int numPiece, int isComplete) {
	bitfield[numPiece] = isComplete;
}

struct fragment File::readFragment (int blockNumber) {
// get the chunk according to the no. of block indicated.
	fragment frag;
	
	frag.numPieceSelf 	= blockNumber;
	// which piece am I in?

	ifstream file (name, std::ifstream::binary);
	// open the file.
	file.seekg (blockNumber * BLOCKSIZE, ios::beg);
	if (blockNumber == (numPieces-1)) {
		file.read (frag.bitstream, length - blockNumber * BLOCKSIZE);
	} else {
		file.read (frag.bitstream, BLOCKSIZE);
	}
	// read the content into the bitstream.
	file.close();
	// close the file.

	unsigned char* temp = new unsigned char[SHA_DIGEST_LENGTH] {};
	SHA1( (unsigned char*) frag.bitstream, BLOCKSIZE, temp );
	for (int i=0; i < SHA_DIGEST_LENGTH; i++) {
		sprintf((char*)&(frag.hashcode[i*2]), "%02x", temp[i]);
	}
	delete[] temp;
	return 	frag;

}

void File::saveFragment (struct fragment frag) {
// save the received fragment somewhere.
	fstream file (name, std::ios::in | std::ios::out | std::ios::binary);
	// open the file.
	file.seekp(0,ios::beg);
	file.seekp(frag.numPieceSelf * BLOCKSIZE, ios::cur);
	if (frag.numPieceSelf == (numPieces-1)) {
		file.write(frag.bitstream, length-frag.numPieceSelf*BLOCKSIZE);
	} else {
		file.write(frag.bitstream, BLOCKSIZE);
	}
	cout << "[file] successfully write on fragment " << frag.numPieceSelf << " on " << frag.numPieceSelf*BLOCKSIZE << endl;
	// write the content into the file.
	bitfield[frag.numPieceSelf] = 1;
	file.close();
}

void File::arrange () {
	isDone = true;
}