//***********************************************************
//			    Self-Extract Data File Builder      	   //
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
#include <commdlg.h>
#include <shlobj.h.>

// COM/AUTOMATION
#include <oleauto.h>

// C
#include <stdio.h>

// CUSTOM
#include "inc\FileInfo.h"
#include "inc\zlib10.h"

// RESOURCE
#include "resource.h"


//-----------------------------------------------------------
//
// USER DEFINE CONSTANT DEFINITION
//
//-----------------------------------------------------------

// Define main window basic constant
#define APP_TITLE	TEXT("Self-Extract Data File Builder")



//-----------------------------------------------------------
//
// GLOBAL VARIABLE DEFINITION & DECLARATION
//
//-----------------------------------------------------------

// HINSTANCE
HINSTANCE	hInst	 = NULL;

// HMENU
HMENU		hMenu	 = NULL;

// HWND
HWND		hWndMain = NULL;

// Char
char		szWinTmpPath[MAX_PATH]		= {NULL};
char		szTmpBinFile1[MAX_PATH]		= {NULL};
char		szTmpBinFile2[MAX_PATH]		= {NULL};
char		szTmpBinFile3[MAX_PATH]		= {NULL};
char		szTmpBinFile4[MAX_PATH]		= {NULL};
char		szSelfExtractFile[MAX_PATH]	= {NULL};


// DWORD
DWORD		dwFileCount = 0;
DWORD		dwTotalFile	= 0;



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
// LISTVIEW CONTROL COLUMN HEADER DEFINITION
//
//-----------------------------------------------------------
typedef struct tagCOLUMNHEADERINFO
{
	int		ColFormat;		// Column format
	int		ColWidth;		// Column width
	TCHAR	ColName[32];	// Column header caption
} COLUMNHEADERINFO;


COLUMNHEADERINFO chi[] = 
{
	{LVCFMT_LEFT,	 45,  TEXT("Index")},
	{LVCFMT_CENTER,	100,  TEXT("File Name")},
	{LVCFMT_CENTER,	 80,  TEXT("File Size")},
	{LVCFMT_CENTER,	130,  TEXT("Create Time")},
	{LVCFMT_CENTER,	130,  TEXT("Last Access Time")},
	{LVCFMT_CENTER,	130,  TEXT("Last Write Time")}
};


COLUMNHEADERINFO ColHeaderInfo[] =
{
	{LVCFMT_LEFT,	 80,  TEXT("Filename")},
	{LVCFMT_CENTER,	 80,  TEXT("Version")},
	{LVCFMT_CENTER,	 80,  TEXT("Build")},
	{LVCFMT_CENTER,	 80,  TEXT("Platform")},
	{LVCFMT_LEFT,   140, TEXT("Path")}
};



//-----------------------------------------------------------
//
// WINDOW & DIALOUGE PROCEDURE DEFINITION
//
//-----------------------------------------------------------

LRESULT CALLBACK	MainWndProc		(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK	AboutBoxProc	(HWND, UINT, WPARAM, LPARAM);



//-----------------------------------------------------------
//
// INTERNAL FUNCTION PROTOTYPE DEFINITION
//
//-----------------------------------------------------------

void	WriteSelfExtractHeader		(HWND, LPWIN32_FIND_DATA, char* prefix, bool isDirectory=false);
void	WriteSelfExtractBinData		(HWND, LPWIN32_FIND_DATA, char*);
BOOL	MergeSelfExtractData		(HWND);

void	RefreshListviewContent		(HWND, LPWIN32_FIND_DATA);
void	GetSelfExtractFileName		(HWND, WPARAM);
void	GetOutputFolder				(HWND, WPARAM);
void	GetDistributeFileList		(HWND);
void	IsInfoComplete				(HWND);


//-----------------------------------------------------------
//
// INTERNAL SYBCHRONIZATION THREAD PROTOTYPE DEFINITION
//
//-----------------------------------------------------------

HANDLE	hThread1	= NULL;
HANDLE	hThread2	= NULL;
HANDLE  hEvent		= NULL;

DWORD	dwThreadId1	= 0;
DWORD	dwThreadId2	= 0;

DWORD WINAPI	CreateSelfExtractFile	(LPVOID);
DWORD WINAPI	BrowseSelfExtractFile	(LPVOID);