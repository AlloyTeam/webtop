//***********************************************************
//				     Self-Extractor Kernel      		   //
//                                                         //
//                     Author : Chris C                    //
//   			        PassportONE.com			           //
//			      http://www.PassportONE.com			   //
//				   Email:  ccthou@yahoo.com				   //
//               Copyright ?May 2004 Chris C              //
//                                                         //
//                                                         //
//                                                         //
//                Compression Library (Zlib)	           //
//                                                         //
//			          http://www.zlib.net			       //
//  Copyright ?1995-2003 Jean-loup Gailly and Mark Adler  //
//     Jean-loup Gailly        Mark Adler                  //
//     jloup@gzip.org          madler@alumni.caltech.edu   //
//                                                         //
//                                                         //
//***********************************************************

// WINDOWS
#include <windows.h>
#include <winbase.h>
#include <commctrl.h>
#include <shlobj.h.>

// COM/AUTOMATION
#include <oleauto.h>

// C
#include <stdio.h>

// CUSTOM
#include "inc\zlib10.h"

// RESOURCE
#include "resource.h"


//-----------------------------------------------------------
//
// USER DEFINE CONSTANT DEFINITION
//
//-----------------------------------------------------------

// Define main window basic constant
#define	 APP_TITLE				"Self-Extractor"




//-----------------------------------------------------------
//
// GLOBAL VARIABLE DEFINITION & DECLARATION
//
//-----------------------------------------------------------

// HINSTANCE
HINSTANCE	hInst	 = NULL;

// HWND
HWND		hWndMain = NULL;

// CHAR
char		szExtractPath[MAX_PATH] = {NULL};



//-----------------------------------------------------------
//
// SELF-EXTRACT DATA FILE STRUCTURE DEFINITION & DECLARATION
//
//-----------------------------------------------------------

typedef struct tagSETUPINFO
{
	DWORD		dwFileCount;
	char		szAutoExecFile[MAX_PATH];
} SETUPINFO, FAR * LPSETUPINFO;


typedef struct tagEXTRACTFILEINFO
{
	DWORD		dwIndex;
	FILETIME	CreateTime;
	FILETIME	LastAcessTime;
	FILETIME	LastWriteTime;
	DWORD		dwFileSizeHigh; 
	DWORD		dwFileSizeLow; 
	char		szBinFileName[MAX_PATH];
	bool		isDirectory;
	char		prefix[1000];
} EXTRACTFILEINFO, FAR * LPEXTRACTFILEINFO;



//-----------------------------------------------------------
//
// WINDOW & DIALOUGE PROCEDURE DEFINITION
//
//-----------------------------------------------------------

LRESULT CALLBACK	MainWndProc			(HWND, UINT, WPARAM, LPARAM);



//-----------------------------------------------------------
//
// INTERNAL FUNCTION PROTOTYPE DEFINITION
//
//-----------------------------------------------------------

int		GetOutputFolder		(HWND);



//-----------------------------------------------------------
//
// THREAD FUNCTION PROTOTYPE DEFINITION
//
//-----------------------------------------------------------

HANDLE	hThread		= NULL;
HANDLE	hEvent		= NULL;
DWORD	dwThreadId  = 0;

DWORD WINAPI ExtractBinaryFile (LPVOID);
