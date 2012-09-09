#ifndef __BASE64_H__
#define __BASE64_H__

#include <stdio.h>
#include <string.h>
#include <malloc.h>

void base64_encode(char *in, const int in_len, char *out, int out_len);
void base64_decode(const char *in, const int in_len, char *out, int *out_len);

#endif // __BASE64_H__