#include <windows.h>
#include <stdio.h>
#include <limits.h>

#include "zlibengn.h"
#include "zlib10.h"


int Compress (LPSTR lpszSource, LPSTR lpszOutput)
{
	//  PURPOSE:  
	//		- Compress the source file.
	//  PARAMETERS:
	//		- lpszSource	:: Source filename
	//		- lpszOutput	:: Output filename
	//  OPERATION:
	//		- Compress the source file into output file
	//  RETURN VALUE:
	//      - 0/1

	ZlibEngine *c_stream = new ZlibEngine;
	int result = c_stream->compress( lpszSource, lpszOutput );
	delete c_stream;
	// Set the return value
	return result;

}

int Uncompress (LPSTR lpszSource, LPSTR lpszOutput)
{
	//  PURPOSE:  
	//		- Uncompress the source file.
	//  PARAMETERS:
	//		- lpszSource	:: Source filename
	//		- lpszOutput	:: Output filename
	//  OPERATION:
	//		- Uncompress the source file into output file
	//  RETURN VALUE:
	//      - 0/1

	ZlibEngine *c_stream = new ZlibEngine;
	int result = c_stream->decompress( lpszSource, lpszOutput );
	delete c_stream;
	// Set the return value
	return result;

}
