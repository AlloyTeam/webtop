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

#include "inc\FileInfo.h"


//  PURPOSE:  
//		- Add the child node with component information
//  PARAMETERS:
//		- lpszFilename	:: Dll filename
//		- lpszBuffer	:: String buffer
//		- MaxBuffer		:: Maximum buffer size
//  OPERATION:
//		- ...
//  RETURN VALUE:
//      - NIL

void GetFileVersion(LPCTSTR lpszFilename, LPTSTR lpszOutput, int nMaxCount)
{
	DWORD				dwFileVerSize=0;
	DWORD				dwHandle=0;
	BYTE				*lpData=NULL;
	LPVOID				lpBuffer=NULL;
	UINT				iLen=0;
	//
	VS_FIXEDFILEINFO	*lpFixedFileInfo=NULL;
	//
	TCHAR				szVersion[260];	
	TCHAR				szPrivate[260];
	TCHAR				szSpecial[260];

	TCHAR				*lpszFile=NULL;

	LPLANGANDCODEPAGE	lpTranslate=NULL;


	__try
	{
		// Allocate memory
		lpszFile = new TCHAR[260];
		memset(lpszFile, TEXT('\0'), 260);
		_tcscpy(lpszFile, lpszFilename);

		// Get file version block size
		dwFileVerSize = GetFileVersionInfoSize(lpszFile, &dwHandle);
		// Checking
		if ((long)dwFileVerSize != 0)
		{
			// Allocate memory to hold the file version information
			lpData = new BYTE[(long)dwFileVerSize];
			// Reset the memory block to NULL
			memset(lpData, TEXT('\0'), (long)dwFileVerSize);
			// Get file information
			GetFileVersionInfo(lpszFile, dwHandle, dwFileVerSize, lpData);

			// NOTE:
			//      The file version information is always in Unicode format. 

			// Query the file version information from the retrieve memory block
			if (VerQueryValue(lpData, TEXT("\\"), &lpBuffer, &iLen))
			{
				// Map the memory block pointer to VS_FIXEDFILEINFO variable.
				lpFixedFileInfo = (VS_FIXEDFILEINFO*)lpBuffer;

				// Reset memory block to NULL
				memset(szVersion, TEXT('\0'), 260);
				// Compose the file version
				wsprintf(szVersion,
						TEXT("%ld.%ld.%ld.%ld"), 
						HIWORD(lpFixedFileInfo->dwFileVersionMS), LOWORD(lpFixedFileInfo->dwFileVersionMS),
						HIWORD(lpFixedFileInfo->dwFileVersionLS), LOWORD(lpFixedFileInfo->dwFileVersionLS));

				// Get the current resource language & code page value
				if (VerQueryValue(lpData, 
							      TEXT("\\VarFileInfo\\Translation"),
							      (LPVOID*)&lpTranslate,
							      &iLen) == 0)
				{
					// Fail to read the lanugage & code page,
					// Use default, English (United States)
					lpTranslate[0].wLanguage = 0x409;
					lpTranslate[0].wCodePage = 0x4B0;
				}

				// # Retrieve Private Build from StringTable 
				memset(szPrivate, TEXT('\0'), 260);
				// Compose query value string
				wsprintf(szPrivate, TEXT("\\StringFileInfo\\%04x%04x\\PrivateBuild"), lpTranslate[0].wLanguage, lpTranslate[0].wCodePage);
				// Get company name
				if (VerQueryValue(lpData, szPrivate, &lpBuffer, &iLen))
				{
					// Reset string buffer
					memset(szPrivate, TEXT('\0'), 260);
					// Compose
					wsprintf(szPrivate, TEXT("%s"), lpBuffer);
				}
				else
					// Compose
					_tcscpy(szPrivate, TEXT("-"));

				// # Retrieve Special Build from StringTable 
				memset(szSpecial, TEXT('\0'), 260);
				// Compose query value string
				sprintf(szSpecial, TEXT("\\StringFileInfo\\%04x%04x\\SpecialBuild"), lpTranslate[0].wLanguage, lpTranslate[0].wCodePage);
				// Get company name
				if (VerQueryValue(lpData, szSpecial, &lpBuffer, &iLen))
				{
					// Reset string buffer
					memset(szSpecial, TEXT('\0'), 260);
					// Compose
					wsprintf(szSpecial, TEXT("%s"), lpBuffer);
				}
				else
					// Compose
					_tcscpy(szSpecial, TEXT("-"));
			}
			// Release the used memory
			delete [] lpData;
		}
		else
		{
			// Initialize...
			memset(szVersion, TEXT('\0'), 260);
			memset(szPrivate, TEXT('\0'), 260);
			memset(szSpecial, TEXT('\0'), 260);
			// Update...
			_tcscpy(szVersion, TEXT("?"));
			_tcscpy(szPrivate, TEXT("?"));
			_tcscpy(szSpecial, TEXT("?"));
		}

		// Build the return string...
		memset(lpszOutput, TEXT('\0'), nMaxCount);
		wsprintf(lpszOutput,
				 TEXT("Version:\t%s (Build %s)\r\nPlatform:\t%s"),
				 szVersion, szPrivate, szSpecial);

	}
	__except(EXCEPTION_EXECUTE_HANDLER)
	{
		// DO NOTHING
	}
	
	// Clean Exit:
	// Release used memory
	if (NULL != lpszFile)
		delete [] lpszFile;
	
}


//  PURPOSE:  
//		- Add the dependacies into the listview control
//  PARAMETERS:
//		- hWndList	:: Listview control handle
//		- hModule	:: Dependacies module instance
//  OPERATION:
//		- ...
//  RETURN VALUE:
//      - NIL

void GetComponentVer(HWND hWndList, LPCTSTR lpszModuleName, HINSTANCE hModule)
{
	DWORD				dwFileVerSize	= 0;
	DWORD				dwHandle		= 0;
	BYTE				*lpData			= NULL;
	LPVOID				lpBuffer		= NULL;
	UINT				iLen			= 0;
	//
	VS_FIXEDFILEINFO	*lpFixedFileInfo= NULL;
	//
	TCHAR				*wBuff			= NULL;
	TCHAR				*pszText		= new TCHAR[260];

	LPTSTR				pszFilePath		= NULL;

	// LISTVIEW CONTROL USED ONLY
	LVITEM				pitem;
	long				lColumn			= 0;
	long				CurrentRowIndex	= 0;

	LPLANGANDCODEPAGE	lpTranslate=NULL;

	__try
	{
		// Allocate string buffer
		wBuff = new TCHAR [260];

		// Add module name (Item) into the dependacies listview control
		pitem.mask = LVIF_TEXT;
		pitem.iItem = SendMessage(hWndList, LVM_GETITEMCOUNT, 0, 0);
		pitem.pszText = (LPTSTR)lpszModuleName;
		pitem.iSubItem = 0;
		// Insert item
		CurrentRowIndex = SendMessage(hWndList, LVM_INSERTITEM, 0, (LPARAM)&pitem);

		// Validation
		if (NULL != hModule)
		{
			// Allocate string buffer
			pszFilePath = (LPTSTR)LocalAlloc(LPTR, 260);

			// Get the module full path
			if ( 0 != GetModuleFileName(hModule, pszFilePath, 260) )
			{
				// Get file version block size
				dwFileVerSize = GetFileVersionInfoSize(pszFilePath, &dwHandle);
				// Checking
				if ((long)dwFileVerSize != 0)
				{
					// Allocate memory to hold the file version information
					lpData = new BYTE[(long)dwFileVerSize];
					// Reset the memory block to NULL
					memset(lpData, TEXT('\0'), (long)dwFileVerSize);
					// Get file information
					GetFileVersionInfo(pszFilePath, dwHandle, dwFileVerSize, lpData);

					// NOTE:
					//      The file version information is always in Unicode format. 
					// Query the file version information from the retrieve memory block
					if (VerQueryValue(lpData, TEXT("\\"), &lpBuffer, &iLen))
					{
						// Map the memory block pointer to VS_FIXEDFILEINFO variable.
						lpFixedFileInfo = (VS_FIXEDFILEINFO*)lpBuffer;
						// Reset memory block to NULL
						memset(wBuff, TEXT('\0'), 260);
						// Compose the file version
						wsprintf(wBuff,
								TEXT("%ld.%ld.%ld.%ld"), 
								HIWORD(lpFixedFileInfo->dwFileVersionMS), LOWORD(lpFixedFileInfo->dwFileVersionMS),
								HIWORD(lpFixedFileInfo->dwFileVersionLS), LOWORD(lpFixedFileInfo->dwFileVersionLS));
						// Add component data to listview
						AddSubItem(hWndList, CurrentRowIndex, 1, wBuff);

						// Get the current resource language & code page value
						if (VerQueryValue(lpData, 
										  TEXT("\\VarFileInfo\\Translation"),
										  (LPVOID*)&lpTranslate,
										  &iLen) == 0)
						{
							// Fail to read the lanugage & code page,
							// Use default, English (United States)
							lpTranslate[0].wLanguage = 0x409;
							lpTranslate[0].wCodePage = 0x4B0;
						}

						// # Retrieve Private Build from StringTable 
						memset(wBuff, TEXT('\0'), 260);
						// Compose query value string
						wsprintf(wBuff, TEXT("\\StringFileInfo\\%04x%04x\\PrivateBuild"), lpTranslate[0].wLanguage, lpTranslate[0].wCodePage);
						// Get company name
						if (VerQueryValue(lpData, wBuff, &lpBuffer, &iLen))
						{
							// Reset string buffer
							memset(wBuff, TEXT('\0'), 260);
							// Compose
							wsprintf(wBuff, TEXT("%s"), lpBuffer);
							// Add component data to listview
							AddSubItem(hWndList, CurrentRowIndex, 2, wBuff);
						}
						else
							// Add component data to listview
							AddSubItem(hWndList, CurrentRowIndex, 2, TEXT("-"));

						// # Retrieve Special Build from StringTable 
						memset(wBuff, TEXT('\0'), 260);
						// Compose query value string
						wsprintf(wBuff, TEXT("\\StringFileInfo\\%04x%04x\\SpecialBuild"), lpTranslate[0].wLanguage, lpTranslate[0].wCodePage);
						// Get company name
						if (VerQueryValue(lpData, wBuff, &lpBuffer, &iLen))
						{
							// Reset string buffer
							memset(wBuff, TEXT('\0'), 260);
							// Compose
							wsprintf(wBuff, TEXT("%s"), lpBuffer);
							// Add component data to listview
							AddSubItem(hWndList, CurrentRowIndex, 3, wBuff);				
						}
						else
							// Add component data to listview
							AddSubItem(hWndList, CurrentRowIndex, 3, TEXT("-"));

						// Add component data to listview
						AddSubItem(hWndList, CurrentRowIndex, 4, pszFilePath);
					}
					else
					{
						// Walk through each columns... and set the default value...
						for (lColumn=1; lColumn<4; lColumn++)
						{
							pitem.mask = LVIF_TEXT;
							pitem.iItem = CurrentRowIndex;
							pitem.pszText = TEXT("-");
							pitem.iSubItem = lColumn;
							// Insert subitem
							SendMessage(hWndList, LVM_SETITEM, 0, (LPARAM)&pitem);			
						}

						// Add component data to listview
						AddSubItem(hWndList, CurrentRowIndex, 4, pszFilePath);
					}

					// Release the used memory
					delete [] lpData;
				}
				else
				{
					// Walk through each columns... and set the default value...
					for (lColumn=1; lColumn<4; lColumn++)
					{
						pitem.mask = LVIF_TEXT;
						pitem.iItem = CurrentRowIndex;
						pitem.pszText = TEXT("-");
						pitem.iSubItem = lColumn;
						// Insert subitem
						SendMessage(hWndList, LVM_SETITEM, 0, (LPARAM)&pitem);			
					}
					// Add component data to listview
					AddSubItem(hWndList, CurrentRowIndex, 4, pszFilePath);
				}
			}
		}
		else
		{
			// Walk through each columns... and set the default value...
			for (lColumn=1; lColumn<4; lColumn++)
			{
				pitem.mask = LVIF_TEXT;
				pitem.iItem = CurrentRowIndex;
				pitem.pszText = TEXT("-");
				pitem.iSubItem = lColumn;
				// Insert subitem
				SendMessage(hWndList, LVM_SETITEM, 0, (LPARAM)&pitem);			
			}
			// Add component data to listview
			AddSubItem(hWndList, CurrentRowIndex, 4, TEXT("missing dll!"));
		}
	}
	__except(EXCEPTION_EXECUTE_HANDLER)
	{
		// DO NOTHING
	}

	// Free the string buffer
	if (NULL != pszFilePath)
		LocalFree(pszFilePath);
	if (NULL != wBuff)
		delete [] wBuff;
}


//  PURPOSE:  
//		- Add the subitem into the dependacies listview control
//  PARAMETERS:
//		- hWndList		:: Listview control window handle.
//		- ItemIndex		:: Current list item index.
//		- SubItemIndex	:: Current adding subitem index.
//		- lpszItemText	:: Sub item display text.
//  OPERATION:
//		- ...
//  RETURN VALUE:
//      - NIL

void AddSubItem(HWND hWndList, long ItemIndex, long SubItemIndex, LPCTSTR lpszItemText)
{
	LVITEM	pitem;

	__try
	{
		pitem.mask = LVIF_TEXT;
		pitem.iItem = ItemIndex;
		pitem.pszText = (LPTSTR)lpszItemText;
		pitem.iSubItem = SubItemIndex;
		// Insert subitem
		SendMessage(hWndList, LVM_SETITEM, 0, (LPARAM)&pitem);
	}
	__except(EXCEPTION_EXECUTE_HANDLER)
	{
		// DO NOTHING
	}
}