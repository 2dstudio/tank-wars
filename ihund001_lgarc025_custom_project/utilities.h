#ifndef UTIL_H
#define UTIL_H

unsigned char SetBit(unsigned char x, unsigned char k, unsigned char b){
	return (b ? x | (0x01 << k) : x & ~(0x01 << k));
}

unsigned char GetBit(unsigned char x, unsigned char k){
	return ((x >> k) & 0x01);
}

#endif