//***********************************************************
//	     		File Version Information Module			   //
//                                                         //
//                     Author : Chris C                    //
//   			        PassportONE.com			           //
//			      http://www.PassportONE.com			   //
//				   Email:  ccthou@yahoo.com				   //
//               Copyright ?Sept 2002 Chris C             //
//                                                         //
//***********************************************************

#include <windows.h>
#include <stdio.h>
#include <tchar.h>
#include <commctrl.h>


//-----------------------------------------------------------
//
// DEPENDACIES LANGAND & CODEPAGE STRUCTURE DEFINITION
//
//-----------------------------------------------------------

typedef struct tagLANGANDCODEPAGE
{
	WORD wLanguage;
	WORD wCodePage;
} LANGANDCODEPAGE, FAR * LPLANGANDCODEPAGE;


//-----------------------------------------------------------
//
// INTERNAL FUNCTION DEFINITION
//
//-----------------------------------------------------------

void GetFileVersion		(LPCTSTR, LPTSTR, int);
void GetComponentVer	(HWND, LPCTSTR, HINSTANCE);
void AddSubItem			(HWND, long, long, LPCTSTR);
