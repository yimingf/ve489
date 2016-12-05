#ifndef __FRAGMENT_H__
#define __FRAGMENT_H__

#include <openssl/sha.h>
#include "global.h"

struct fragment {

	int 		numPieceSelf;
	// which piece am I in?
	char	bitstream[BLOCKSIZE];
	// the content.
	char 	hashcode[SHA_DIGEST_LENGTH * 2];
	// the hashcode of the content.
};

#endif