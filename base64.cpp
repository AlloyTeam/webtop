#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>



//#include <string>
#include <fstream>
#include <iostream>

using namespace std;



static const char *codes =
"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
char ords[128];

#define PAD -1

void init_ords()
{
	ords['A'] = 0; ords['B'] = 1; ords['C'] = 2; ords['D'] = 3; ords['E'] = 4; 
	ords['F'] = 5; ords['G'] = 6; ords['H'] = 7; ords['I'] = 8; ords['J'] = 9; 
	ords['K'] = 10; ords['L'] = 11; ords['M'] = 12; ords['N'] = 13; ords['O'] = 14; 
	ords['P'] = 15; ords['Q'] = 16; ords['R'] = 17; ords['S'] = 18; ords['T'] = 19; 
	ords['U'] = 20; ords['V'] = 21; ords['W'] = 22; ords['X'] = 23; ords['Y'] = 24; 
	ords['Z'] = 25; ords['a'] = 26; ords['b'] = 27; ords['c'] = 28; ords['d'] = 29; 
	ords['e'] = 30; ords['f'] = 31; ords['g'] = 32; ords['h'] = 33; ords['i'] = 34; 
	ords['j'] = 35; ords['k'] = 36; ords['l'] = 37; ords['m'] = 38; ords['n'] = 39; 
	ords['o'] = 40; ords['p'] = 41; ords['q'] = 42; ords['r'] = 43; ords['s'] = 44; 
	ords['t'] = 45; ords['u'] = 46; ords['v'] = 47; ords['w'] = 48; ords['x'] = 49; 
	ords['y'] = 50; ords['z'] = 51; ords['0'] = 52; ords['1'] = 53; ords['2'] = 54; 
	ords['3'] = 55; ords['4'] = 56; ords['5'] = 57; ords['6'] = 58; ords['7'] = 59; 
	ords['8'] = 60; ords['9'] = 61; ords['+'] = 62; ords['/'] = 63;
	ords['='] = PAD;
}

void base64_encode(char *in, const int in_len, char *out, int out_len)
{
	init_ords();
	int base64_len = 4 * ((in_len+2)/3);
	assert(out_len >= base64_len);
	char *p = out;
	int times = in_len / 3;

	for(int i=0; i<times; ++i) {
		*p++ = codes[(in[0] >> 2) & 0x3f];
		*p++ = codes[((in[0] << 4) & 0x30) + ((in[1] >> 4) & 0xf)];
		*p++ = codes[((in[1] << 2) & 0x3c) + ((in[2] >> 6) & 0x3)];
		*p++ = codes[in[2] & 0x3f];
		in += 3;
	}

	// pad .. 
	if(times * 3 + 1 == in_len) {
		*p++ = codes[(in[0] >> 2) & 0x3f];
		*p++ = codes[((in[0] << 4) & 0x30) + ((in[1] >> 4) & 0xf)];
		*p++ = '=';
		*p++ = '=';
	}
	if(times * 3 + 2 == in_len) {
		*p++ = codes[(in[0] >> 2) & 0x3f];
		*p++ = codes[((in[0] << 4) & 0x30) + ((in[1] >> 4) & 0xf)];
		*p++ = codes[((in[1] << 2) & 0x3c)];
		*p++ = '=';
	}
	*p = 0;
}

void base64_decode(const char *in, const int in_len, char *out, int *out_len)
{
	init_ords();
	int decode_len = in_len * 3 / 4;
	printf("out_len=%d, in_len=%d, decode_len=%d\n", out_len, in_len, decode_len);
	assert(*out_len > decode_len);
	char *tmp=new char[in_len];  // for ords[]
	const char *p = in;
	for(int i=0; i<in_len; ++i,++p) {
		tmp[i] = ords[*p];
	}

	char *q = out;
	p = tmp;
	*out_len = 0;
	for(int i=0; i<in_len-4; i+=4) {
		*q++ = (p[0] << 2) + (p[1] >> 4);
		*q++ = (p[1] << 4) + (p[2] >> 2);
		*q++ = (p[2] << 6) + p[3];
		p += 4;
		*out_len += 3;
	}

	// deal with pad
	if(p[3] != PAD) {   // no pad
		*q++ = (p[0] << 2) + (p[1] >> 4);
		*q++ = (p[1] << 4) + (p[2] >> 2);
		*q++ = (p[2] << 6) + p[3];
		*out_len += 3;
	} else if(p[2] != PAD) {  // one pad
		*q++ = (p[0] << 2) + (p[1] >> 4);
		*q++ = (p[1] << 4) + (p[2] >> 2);
		*out_len += 2;
	} else if(p[1] != PAD) {    // two pads
		*q++ = (p[0] << 2) + (p[1] >> 4);
		*q++ = (p[1] << 4);
		*out_len += 2;
	}
	*q++ = 0;
	delete []tmp;
}
