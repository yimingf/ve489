#include "global.h"

using namespace std;

unsigned int TransCHAR(char c[]){
	unsigned int n=0;
	for (int i=0;i<4;i++){
		n+=(uint8_t)c[i] *pow(2,8*(3-i));
	}
	return n;
}
void TransINT(unsigned int i, char c[]){
	int j;
	for (j=3;j>=0;j--){
		c[j]=i%256;
		i=i/256;
	}
}