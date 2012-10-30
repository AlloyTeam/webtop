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

#include "inc\SetupEx.h"


//  PURPOSE:  
//		Main application message dispatch routing.
//		Check for previous instance.
//  PARAMETERS:
//		- HINSTANCE
//		- hPreviousInstance
//		- pCmdLine
//		- nCmdShow
//  OPERATION:
//		- Dispatch the received windows message
//  RETURN VALUE:
//      0/1

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine, int nCmdShow)
{
	INITCOMMONCONTROLSEX	icex;
	MSG msg;

	// Update global variable
	hInst = hInstance;

	// Setup the INITCOMMONCONTROLSEX structure
	icex.dwSize = sizeof(INITCOMMONCONTROLSEX);
	icex.dwICC  = ICC_PROGRESS_CLASS;
	// Initialize the Windows Common Control Library
	InitCommonControlsEx (&icex);

	// Create main form
	hWndMain = CreateDialog(hInst, MAKEINTRESOURCE(IDD_MAINWND), NULL, (DLGPROC)MainWndProc);

	if (hWndMain == NULL)
	{
		// Notified your about the failure
		MessageBox(NULL, TEXT("Fail to start self-extract program!"), APP_TITLE, MB_OK | MB_ICONEXCLAMATION);
		// Set the return value
		return false;
	}
	else
		// Display the dialouge
		ShowWindow(hWndMain, nCmdShow);

	// Main message loop:
	while (GetMessage(&msg, NULL, 0, 0)) 
	{
		if (!IsDialogMessage(hWndMain, &msg)) 
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	return msg.wParam;
}


//  PURPOSE:  
//		- Main window dialouge CALLBACK procedure.
//  PARAMETERS:
//		- Standard Window Procedure call back function arguement
//  OPERATION:
//		- Any windows related function/application functioncalling.
//  RETURN VALUE:
//      0/1
//
LRESULT CALLBACK MainWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	static HICON	hIcon1 = NULL;

	DWORD	dwTime		= 0;
	DWORD	dwExitCode	= 0;


	switch (message) 
	{
		case WM_INITDIALOG:
			// Update Window Caption
			SetWindowText(hWnd, APP_TITLE);

			// Set the dialog ICON
			hIcon1 = LoadIcon(hInst, (LPCTSTR)IDI_ICON1);
			SendMessage(hWnd, WM_SETICON, ICON_BIG, (LPARAM)hIcon1);
			
			// Get the user define output folder
			if (0 < GetOutputFolder (hWnd))
			{
				// Proceed to extract the data files
				
				// Create thread control event
				hEvent = CreateEvent(NULL,
									 TRUE,
									 FALSE,
									 NULL);
				// Create extract data file thread
				hThread = CreateThread(NULL,
									   0,
									   &ExtractBinaryFile,
									   hWnd,
									   0,
									   &dwThreadId);
			}
			//
			return TRUE;

		case WM_CLOSE:
		case WM_DESTROY:
			// Delete the loaded ICON
			if (NULL != hIcon1) {DestroyIcon(hIcon1);}
			hIcon1 = NULL;

			// Get the current thread exit code state
			if (NULL != hThread) {GetExitCodeThread(hThread, &dwExitCode);}
			// Check both the thread state
			if (STILL_ACTIVE == dwExitCode)
			{
				// Set the "ExtractBinaryFile" control event
				SetEvent(hEvent);

				dwTime = GetTickCount();
				//
				do
				{
					// DO NOTHING, AS THIS IS A DELAY LOOP					
					// THE REASON, "SLEEP": API WAS NOT CALL
					// HERE. BECAUSE, SLEEP WILL CAUSE THE
					// APPLICATION STOP RECEIVED ANY MESSAGE
					// FROM SYSTEM MESSAGE QUEUE.
				} while (GetTickCount() - dwTime < 500);
			}

			// Close the event handle
			if (NULL != hEvent) {CloseHandle(hEvent);}
			hEvent = NULL;


			// Send the end program command
			PostQuitMessage(WM_QUIT);
			//
			return TRUE;

		default:
			break;
   }
   return 0;
}

//  PURPOSE:  
//		- Get the user define source & output folder
//  PARAMETERS:
//		- hWnd		:: Parent window handle
//		- wParam	:: Standard window procedure WPARAM
//  OPERATION:
//		- ...
//  RETURN VALUE:
//      - NIL
//
int GetOutputFolder (HWND hWnd)
{
	BROWSEINFO		bi = {NULL};
	LPITEMIDLIST	lpIDList = NULL;

	__try
	{
		// Reset the gloabl variable
		ZeroMemory(szExtractPath, sizeof(szExtractPath));

		// Setup BROWSEINFO structure
		bi.hwndOwner 		= hWnd;
		bi.pidlRoot			= NULL;
		bi.lpszTitle		= TEXT("Extract to...");
		bi.ulFlags			= BIF_RETURNONLYFSDIRS;
		bi.lpfn				= NULL;
		bi.pszDisplayName	= szExtractPath;

		// Load the Browse folder dialog
		lpIDList = SHBrowseForFolder(&bi);
		// Get the selected path from IDList
		SHGetPathFromIDList(lpIDList, szExtractPath);
		// Free the resources
		CoTaskMemFree(lpIDList);
		
		// Check the selected folder
		if (strlen(szExtractPath) > 0)
		{
			// Check is the last character is "\", if not, an extra "\" was append on it.
			if (92 != szExtractPath[strlen(szExtractPath) - 1]) {strncat(szExtractPath, "\\", 1);}
		}

	}
	__except(EXCEPTION_EXECUTE_HANDLER)
	{
		// PUT YOUR ERROR HANDLE CODE HERE

	}

	// Set the return value
	return strlen(szExtractPath);
}




//  PURPOSE:  
//		- Start extract the update data file from the compressed merge data
//		  under the resource table to the user define output folder.
//  PARAMETERS:
//		- hWnd	:: Parent window handle
//  OPERATION:
//		- 
//  RETURN VALUE:
//      - 0
//
DWORD WINAPI ExtractBinaryFile (LPVOID pParam)
{
	char	szWinTmpPath[MAX_PATH]  = {NULL};
	char	szTmpBinFile1[MAX_PATH] = {NULL};
	char	szTmpBinFile2[MAX_PATH] = {NULL};

	HRSRC	hResource	= 0;
	HGLOBAL	hResData	= 0;
	DWORD	dwResSize	= 0;
	LPVOID	lpData		= NULL;

	HANDLE	hFile		= NULL;
	LPBYTE	lpBinData	= NULL;
	DWORD	dwIndex1	= 0;
	DWORD	dwIndex2	= 0;
	DWORD	dwFileSize	= 0;
	DWORD	dwDataPtr	= 0;
	DWORD	dwDataSize	= 0;
	DWORD	dwByteRead  = 0;
	DWORD	dwByteWrite = 0;

	long	lCurPosition		= 0;
	char	szBuffer[MAX_PATH]	= {NULL};
	HWND	hWnd				= NULL;

	SETUPINFO			si		= {NULL};
	LPEXTRACTFILEINFO	lpefi	= NULL;


	
	__try
	{
		// Map the pass in varibale
		hWnd = (HWND)pParam;

		// PRE-EXTRACT STAGE
		{
			/*
			//	NOTE:
			//		THE FOLLOWING CODE REMARK, DUE THE THE "SYSTEM MODEL"
			//		PROPERTIES WAS SET IN THE DIALOG. SO, THERE WAS NO
			//		EXTRA COMMAND REQUIRE TO SWITCH THE DIALOG BECOME
			//		TOP-MOST WINDOWS.

			// Change current window Z-Order
			SetWindowPos(hWnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
			*/

			// Set the progressbar Min,Max value
			SendDlgItemMessage(hWnd, IDC_PROGRESS1,  PBM_SETRANGE, 0, MAKELPARAM(1, 100));
			// Set the initial position
			SendDlgItemMessage(hWnd, IDC_PROGRESS1, PBM_SETPOS, 0, 0);
			// Set the initial value of the progress %
			SetDlgItemText(hWnd, IDC_PROGRESSVALUE, "");
			// Refresh the window
			UpdateWindow(GetDlgItem(hWnd, IDC_PROGRESS1));

			// NOTE:
			//		IN ORDER TO HAVE A BETTER VISIUALIZE EFFECT ON THE PROFRESS.
			//		STAGE #1 - STAGE#4 WILL HAVE THE PROGRESS BAR RUNNING FROM 0% - 100%
			//		WHICH MEAN, EACH STAGE WILL HAVE 25% UPDATE ON THE PROGRESS BAR.
			//
			//		WHILE, ON STAGE#5, IT WILL HAVE ANOTHER ROUND OF PROGRESS BAR
			//		UPDATE FROM 0% - 100%, BASE ON HOW MANY BINDARY DATA FILE
			//		REQUIRE TO UPDATE (ui.dwFileCount).
			//
			//		IN STAGE#6, IT WILL HAVE ANOTHER ROUND OF PROGRESS BAR UPDATE
			//		FROM 0% - 100%, DUE TO MOVING THE EXTRACTED BINARY FILE FROM
			//		WINDOW TEMP FOLDER INTO THE ACTUAL DEFINE PROGRAM FOLDER AS
			//		SPECIFIED BY THE USER AT THE BEGINING OF THIS PROGRAM
			//
			//		AT STAGE#7, IT WILL ON UPDATE ON THE STATUS. TO KEEP THE
			//		USER UPDATE ON THE SYSTEM IS GOING TO PURGE THE TEMPORARY
			//		DATA FILE & CLEAN UP ALL THE ALLOCATED MEMORY & ETC...
			//
			//		LAST, AT STAGE#8, IT WILL RELAUNCH THE APPLICATION (IF REQUIRE).
			//		IF YES, THEN THIS PROGRAM IS RESPONSIBLE TO RELAUNCH THE SPECIFIED
			//		PROGRAM, WHICH REQUIRE TO UNLOAD DUE TO THE RELATED PROGRAM FILE
			//		IS ABOUT TO UPDATE.
			//


			// Update the status text
			SetDlgItemText(hWnd, IDC_STATUS, "Preparing data file...");
			// Refresh the window
			UpdateWindow(GetDlgItem(hWnd, IDC_STATUS));
		}

		// Check is the App terminated? Else, terminate current thread
		if (WAIT_OBJECT_0 == WaitForSingleObject(hEvent, 100)) {goto CleanExit;}

		
		// STAGE #1
		//	IF YOU WISH TO CLOSE SOME RELATED PROGRAM, PLEASE PLACE YOUR CODE HERE
		//	
		//	YOU CAN FIRST LOCATE THE APPLICATION BY USING THE FindWindow/FindWindowEx API,
		//  SUBSEQUENCE USING SendMessage (hWnd, WM_SYSCOMMAND, SC_CLOSE, NULL);
		//
		//	AFTER SEND THE WM_SYSCOMMAND, IT IS RECOMMENDED YOU ENSURE THE
		//	RELATED APPLICATION IS CLOSE, BUT CALLING THE FindWindow or FindWindowEx
		//	AGAIN.
		/*
		{
			// #GUI UPDATE#
			// Set the initial position
			SendDlgItemMessage(hWnd, IDC_PROGRESS1, PBM_SETPOS, 0, 0);
			// Refresh the window
			UpdateWindow(GetDlgItem(hWnd, IDC_PROGRESS1));

			// Update the status text
			SetDlgItemText(hWnd, IDC_PROGRESSVALUE, "0%");
			// Refresh the window
			UpdateWindow(GetDlgItem(hWnd, IDC_PROGRESSVALUE));


			// #ACUTAL TASK#
			// 
		}
		*/


		// Check is the App terminated? Else, terminate current thread
		if (WAIT_OBJECT_0 == WaitForSingleObject(hEvent, 100)) {goto CleanExit;}


		// STAGE #2
		// Create the necessary temporary binary filename (full path)
		// Delete all these file (if any)
		{
			// #GUI UPDATE#
			// Set the initial position
			SendDlgItemMessage(hWnd, IDC_PROGRESS1, PBM_SETPOS, 25, 0);
			// Refresh the window
			UpdateWindow(GetDlgItem(hWnd, IDC_PROGRESS1));

			// Update the status text
			SetDlgItemText(hWnd, IDC_PROGRESSVALUE, "25%");
			// Refresh the window
			UpdateWindow(GetDlgItem(hWnd, IDC_PROGRESSVALUE));

			
			// #ACUTAL TASK#
			// Get the current window temporary folder
			GetTempPath(sizeof(szWinTmpPath), szWinTmpPath);
			//MessageBox(NULL, szWinTmpPath, APP_TITLE, MB_OK | MB_ICONEXCLAMATION);
			//sprintf(szWinTmpPath,"E:\\temp\\");
			// Get the random generated file name
			sprintf(szTmpBinFile1,
					"%s%08X",
					szWinTmpPath, GetTickCount());
			// Get the random generate file name for holding the decompressed merge data file
			sprintf(szTmpBinFile2,
					"%s%08X",
					szWinTmpPath, (DWORD)(GetTickCount()*12345)/8725);

			// Delete all the existing file (if any)
			DeleteFile(szTmpBinFile1);
			DeleteFile(szTmpBinFile2);
		}



		// Check is the App terminated? Else, terminate current thread
		if (WAIT_OBJECT_0 == WaitForSingleObject(hEvent, 100)) {goto CleanExit;}



		// STAGE #3
		// Read the data from custom resource table & save it into 
		// temporary data file in window temp folder
		{
			// #GUI UPDATE#
			// Set the initial position
			SendDlgItemMessage(hWnd, IDC_PROGRESS1, PBM_SETPOS, 50, 0);
			// Refresh the window
			UpdateWindow(GetDlgItem(hWnd, IDC_PROGRESS1));

			// Update the status text
			SetDlgItemText(hWnd, IDC_PROGRESSVALUE, "50%");
			// Refresh the window
			UpdateWindow(GetDlgItem(hWnd, IDC_PROGRESSVALUE));

			// Update the status text
			SetDlgItemText(hWnd, IDC_STATUS, "Extracting file information...");
			// Refresh the window
			UpdateWindow(GetDlgItem(hWnd, IDC_STATUS));

			
			// #ACUTAL TASK#
			// Locate the resource template handle
			hResource = FindResource(hInst, MAKEINTRESOURCE(IDR_SETUP1), "SETUP");
			// Get the total resource size
			dwResSize = SizeofResource(hInst, hResource);
			// Load the resource content
			hResData = LoadResource(hInst, hResource);
			// Checking
			if (hResData != NULL && dwResSize != 0)
			{
				// Ensure the lpData is NULL
				lpData = NULL;
				// Obtain the string pointer from the loaded resource handle
				lpData = LockResource(hResData);

				// Save the current read data into a file
				hFile = CreateFile(szTmpBinFile1,
								   GENERIC_WRITE,
								   FILE_SHARE_WRITE,
								   NULL,
								   CREATE_ALWAYS,
								   FILE_ATTRIBUTE_NORMAL,
								   NULL);

				// Check the return handle value
				if (NULL != hFile && INVALID_HANDLE_VALUE != hFile)
				{
					// Move the file pointer to the begin of the file
					SetFilePointer(hFile, 0, 0, FILE_BEGIN);
					// Write the read data into a temp file
					WriteFile(hFile,
							  (LPBYTE)lpData,
							  dwResSize,
							  &dwByteWrite,
							  NULL);
					// Close the current open data file
					if (NULL != hFile) {CloseHandle(hFile);}
					hFile = NULL;
				}
				else
				{
					// Notify user about the error
					MessageBox(hWnd, "Fail to create temporary data file, extracting process aborted!", APP_TITLE, MB_OK | MB_ICONSTOP);
					// Jump the the "CleanExit" routine
					goto CleanExit;
				}
			}
			else
			{
				// Notify user about the error
				//MessageBox(hWnd, "Fail to extract the data, extracting process aborted!", APP_TITLE, MB_OK | MB_ICONSTOP);
				// Jump the the "CleanExit" routine
				//goto CleanExit;
		}	}


		// Check is the App terminated? Else, terminate current thread
		if (WAIT_OBJECT_0 == WaitForSingleObject(hEvent, 100)) {goto CleanExit;}

		
		// STAGE #4
		// Decompress the merge data file saved in the window temp folder after
		// read from custom resource table.
		{
			// #GUI UPDATE#
			// Set the initial position
			SendDlgItemMessage(hWnd, IDC_PROGRESS1, PBM_SETPOS, 75, 0);
			// Refresh the window
			UpdateWindow(GetDlgItem(hWnd, IDC_PROGRESS1));

			// Update the status text
			SetDlgItemText(hWnd, IDC_PROGRESSVALUE, "75%");
			// Refresh the window
			UpdateWindow(GetDlgItem(hWnd, IDC_PROGRESSVALUE));

			// Update the status text
			SetDlgItemText(hWnd, IDC_STATUS, "Decompress merge data file...");
			// Refresh the window
			UpdateWindow(GetDlgItem(hWnd, IDC_STATUS));


			// #ACUTAL TASK#
			// Uncompress the read resource data from szTmpBinFile1 into szTmpBinFile2
			if (0 != Uncompress(szTmpBinFile1, szTmpBinFile2))
			{
				// Notify user about the error
				//MessageBox(hWnd, "Fail to decompress the data, extracting process aborted!", APP_TITLE, MB_OK | MB_ICONSTOP);
				// Jump the the "CleanExit" routine
				//goto CleanExit;
			}


			// Delete the used temp file
			DeleteFile(szTmpBinFile1);


			// #GUI UPDATE#
			// Set the initial position
			SendDlgItemMessage(hWnd, IDC_PROGRESS1, PBM_SETPOS, 100, 0);
			// Refresh the window
			UpdateWindow(GetDlgItem(hWnd, IDC_PROGRESS1));

			// Update the status text
			SetDlgItemText(hWnd, IDC_PROGRESSVALUE, "100%");
			// Refresh the window
			UpdateWindow(GetDlgItem(hWnd, IDC_PROGRESSVALUE));

			// Update the status text
			SetDlgItemText(hWnd, IDC_STATUS, "");
			// Refresh the window
			UpdateWindow(GetDlgItem(hWnd, IDC_STATUS));
		}


		// Check is the App terminated? Else, terminate current thread
		if (WAIT_OBJECT_0 == WaitForSingleObject(hEvent, 100)) {goto CleanExit;}

		
		// STAGE #5
		// Start read the decompressed merge dta file in window temp folder
		{
			// #GUI UPDATE#
			// Set the initial position
			SendDlgItemMessage(hWnd, IDC_PROGRESS1, PBM_SETPOS, 0, 0);
			// Refresh the window
			UpdateWindow(GetDlgItem(hWnd, IDC_PROGRESS1));

			// Update the status text
			SetDlgItemText(hWnd, IDC_PROGRESSVALUE, "0%");
			// Refresh the window
			UpdateWindow(GetDlgItem(hWnd, IDC_PROGRESSVALUE));

			// Update the status text
			SetDlgItemText(hWnd, IDC_STATUS, "Initialize file information...");
			// Refresh the window
			UpdateWindow(GetDlgItem(hWnd, IDC_STATUS));


			// #ACUTAL TASK#
			// SUB-STAGE#5-1
			// Open the decompress data file
			hFile = CreateFile(szTmpBinFile2,
							   GENERIC_READ,
							   FILE_SHARE_READ,
							   NULL,
							   OPEN_EXISTING,
							   FILE_ATTRIBUTE_NORMAL,
							   NULL);

			// Check the return handle value
			if (NULL != hFile && INVALID_HANDLE_VALUE != hFile)
			{
				// Get the current file size
				dwFileSize = GetFileSize(hFile, NULL);
				// Ensure the file size of not 0Byte
				if (0 < dwFileSize)
				{
					// Allocate local memory to hold the data
					lpBinData = (LPBYTE)LocalAlloc(LPTR, dwFileSize);
					// Reset the local memory buffer
					ZeroMemory(lpBinData, dwFileSize);

					// Move the file pointer to the begin of the file
					SetFilePointer(hFile, 0, 0, FILE_BEGIN);
					// Read data from a temp file
					ReadFile(hFile,
							 lpBinData,
							 dwFileSize,
							 &dwByteRead,
							 NULL);
				}
				else
				{
					// Notify user about the error
					MessageBox(hWnd, "No data in the decompressed data file, extracting process aborted!", APP_TITLE, MB_OK | MB_ICONSTOP);
					// Jump the the "CleanExit" routine
					goto CleanExit;
				}
			}
			else
			{
				// Notify user about the error
				MessageBox(hWnd, "Fail to read temporary data file, updating extracting aborted!", APP_TITLE, MB_OK | MB_ICONSTOP);
				// Jump the the "CleanExit" routine
				goto CleanExit;
			}

			// Close the current open data file
			if (NULL != hFile) {CloseHandle(hFile);}
			hFile = NULL;

			// Delete the used temp file
			DeleteFile(szTmpBinFile2);


			// SUB-STAGE#5-2
			// Reset the local variable SETUPINFO structure
			ZeroMemory(&si, sizeof(SETUPINFO));
			// Copy the read information into respective structure
			CopyMemory(&si, &lpBinData[0], sizeof(SETUPINFO));

			// Check the bindary file count
			if (0 < si.dwFileCount)
			{
				// Allocate the local variable to hold the extract file information
				lpefi = (LPEXTRACTFILEINFO)LocalAlloc(LPTR, sizeof(EXTRACTFILEINFO)*si.dwFileCount);
				// Reset the local variable
				ZeroMemory(lpefi, sizeof(EXTRACTFILEINFO)*si.dwFileCount);
				// Copy the read information into respective structure
				CopyMemory(lpefi, &lpBinData[sizeof(SETUPINFO)], sizeof(EXTRACTFILEINFO)*si.dwFileCount);
			}
			else
			{
				// Notify user about the error
				MessageBox(hWnd, "No file require to extract, process aborted!", APP_TITLE, MB_OK | MB_ICONSTOP);
				// Jump the the "CleanExit" routine
				goto CleanExit;
			}
			

			// Update the status text
			SetDlgItemText(hWnd, IDC_STATUS, "Extracting file information...");
			// Refresh the window
			UpdateWindow(GetDlgItem(hWnd, IDC_STATUS));


			// Initialize the local variable to hold the data pointer
			dwDataPtr = sizeof(SETUPINFO) + (sizeof(EXTRACTFILEINFO)*si.dwFileCount); 

			// SUB-STAGE#5-3
			// Walk through each binary file
			for (dwIndex1=0; dwIndex1<si.dwFileCount; dwIndex1++)
			{
				EXTRACTFILEINFO efi=lpefi[dwIndex1];
				if(!efi.isDirectory){
				// Reset the loca variable
					ZeroMemory(szTmpBinFile1, sizeof(szTmpBinFile1));
					// Build the temp filename (full path)
					sprintf(szTmpBinFile1,
							"%s%s%s",
							szWinTmpPath, efi.prefix, efi.szBinFileName);

					// Create the new update binary file
					hFile = CreateFile(szTmpBinFile1,
									   GENERIC_WRITE,
									   FILE_SHARE_WRITE,
									   NULL,
									   CREATE_ALWAYS,
									   FILE_ATTRIBUTE_NORMAL,
									   NULL);

					// Check the return handle value
					if (NULL != hFile && INVALID_HANDLE_VALUE != hFile)
					{
						// Get the current data file size base on the information
						// store in the lpefi:
						//		lpefi[dwIndex1].dwFileSizeHigh
						//		lpefi[dwIndex1].dwFileSizeLow 
						//
						dwDataSize = (lpefi[dwIndex1].dwFileSizeHigh*(MAXDWORD+1)) + 
									  lpefi[dwIndex1].dwFileSizeLow;

						// Ensure the file size of not 0Byte
						if (0 < dwDataSize)
						{
							// Move the file pointer to the begin of the file
							SetFilePointer(hFile, 0, 0, FILE_BEGIN);
							// Write data into a temp file
							WriteFile(hFile,
									  &lpBinData[dwDataPtr],
									  dwDataSize,
									  &dwByteWrite,
									  NULL);

							// Ensure the byte write is equal to the calculated data size
							if (dwByteWrite != dwDataSize)
							{
								// Notify user about the error
								//MessageBox(hWnd, szTmpBinFile1, APP_TITLE, MB_OK | MB_ICONSTOP);
								// Jump the the "RollBack" routine
								//goto RollBack;
							}

							// Update the filetime information
							SetFileTime(hFile,
										&lpefi[dwIndex1].CreateTime,
										&lpefi[dwIndex1].LastAcessTime,
										&lpefi[dwIndex1].LastWriteTime); 

							// Close the current open file handle
							if (NULL != hFile) {CloseHandle(hFile);}
							hFile = NULL;

							// Increate the local data pointer (dwDataPtr)
							dwDataPtr += dwDataSize;

						}
						else
						{
							// Notify user about the error
							//MessageBox(hWnd, "Invalid data file size, extracting process aborted!", APP_TITLE, MB_OK | MB_ICONSTOP);
							// Jump the the "RollBack" routine
							//goto RollBack;
						}
					}
					else
					{
						// Notify user about the error
						dwDataSize = (lpefi[dwIndex1].dwFileSizeHigh*(MAXDWORD+1)) + 
									  lpefi[dwIndex1].dwFileSizeLow;
						dwDataPtr += dwDataSize;
						//MessageBox(hWnd, lpefi[dwIndex1].szBinFileName, APP_TITLE, MB_OK | MB_ICONSTOP);
						// Jump the the "RollBack" routine
						//goto RollBack;
					}
				}
				else{
					ZeroMemory(szTmpBinFile1, sizeof(szTmpBinFile1));
					// Build the temp filename (full path)
					sprintf(szTmpBinFile1,
							"%s%s%s",
							szWinTmpPath, efi.prefix, efi.szBinFileName);
					CreateDirectory(szTmpBinFile1,NULL);
				}
				lCurPosition = (long)(((double)(dwIndex1+1)/si.dwFileCount)*100);
				// Update the progress bar position
				SendDlgItemMessage(hWnd, IDC_PROGRESS1, PBM_SETPOS, lCurPosition, 0);
				// Refresh the window
				UpdateWindow(GetDlgItem(hWnd, IDC_PROGRESS1));

				// Reset the local variable
				ZeroMemory(szBuffer, sizeof(szBuffer));
				// Format the display value
				sprintf(szBuffer,
						"%ld%%",
						(int)lCurPosition);
				// Update the status text
				SetDlgItemText(hWnd, IDC_PROGRESSVALUE, szBuffer);
				// Refresh the window
				UpdateWindow(GetDlgItem(hWnd, IDC_PROGRESSVALUE));
				// Check is the App terminated? Else, Jump to "RollBack" routine before terminate current thread
				//if (WAIT_OBJECT_0 == WaitForSingleObject(hEvent, 100)) {goto RollBack;}
			}
		}

		
		// Check is the App terminated? Else, terminate current thread
		if (WAIT_OBJECT_0 == WaitForSingleObject(hEvent, 100)) {goto CleanExit;}


		// STAGE #6
		// Moving all the extracted binary data file from window temp folder
		// to the user define folder
		{
			// #GUI UPDATE#
			// Set the initial position
			SendDlgItemMessage(hWnd, IDC_PROGRESS1, PBM_SETPOS, 0, 0);
			// Refresh the window
			UpdateWindow(GetDlgItem(hWnd, IDC_PROGRESS1));

			// Update the status text
			SetDlgItemText(hWnd, IDC_PROGRESSVALUE, "0%");
			// Refresh the window
			UpdateWindow(GetDlgItem(hWnd, IDC_PROGRESSVALUE));

			// Update the status text
			SetDlgItemText(hWnd, IDC_STATUS, "");
			// Refresh the window
			UpdateWindow(GetDlgItem(hWnd, IDC_STATUS));

			// Walk through each binary file
			for (dwIndex1=0; dwIndex1<si.dwFileCount; dwIndex1++)
			{
				EXTRACTFILEINFO efi=lpefi[dwIndex1];
				// #GUI UPDATE#
				// Calculate the current position
				lCurPosition = (long)(((double)(dwIndex1+1)/si.dwFileCount)*100);
				// Update the progress bar position
				SendDlgItemMessage(hWnd, IDC_PROGRESS1, PBM_SETPOS, lCurPosition, 0);
				// Refresh the window
				UpdateWindow(GetDlgItem(hWnd, IDC_PROGRESS1));

				// Reset the local variable
				ZeroMemory(szBuffer, sizeof(szBuffer));
				// Format the display value
				sprintf(szBuffer,
						"%ld%%",
						lCurPosition);
				// Update the status text
				SetDlgItemText(hWnd, IDC_PROGRESSVALUE, szBuffer);
				// Refresh the window
				UpdateWindow(GetDlgItem(hWnd, IDC_PROGRESSVALUE));
				
				// Reset the local variable
				ZeroMemory(szBuffer, sizeof(szBuffer));
				// Format the display value
				sprintf(szBuffer,
						"Copying ...\\%s",
						lpefi[dwIndex1].szBinFileName);
				// Update the status text
				SetDlgItemText(hWnd, IDC_STATUS, szBuffer);
				// Refresh the window
				UpdateWindow(GetDlgItem(hWnd, IDC_STATUS));


				// #ACUTAL TASK#
				// Reset the loca variable
				ZeroMemory(szTmpBinFile1, sizeof(szTmpBinFile1));
				ZeroMemory(szTmpBinFile2, sizeof(szTmpBinFile2));

				// Build the temp filename (full path)
				// Source filepath
				sprintf(szTmpBinFile1,
						"%s%s%s",
						szWinTmpPath, efi.prefix, efi.szBinFileName);
				// Destination filepath
				sprintf(szTmpBinFile2,
						"%s%s%s",
						szExtractPath, efi.prefix, efi.szBinFileName);

				// Moving the file from window temp folder to the specified target folder
				if(!efi.isDirectory){
					BOOL bResult = CopyFile(szTmpBinFile1, szTmpBinFile2, FALSE);
				// Delete the source file
					DeleteFile(szTmpBinFile1);
				}
				else{
					BOOL bResult = CreateDirectory(szTmpBinFile2, NULL);
					//MessageBox(hWnd, szTmpBinFile2, APP_TITLE, MB_OK | MB_ICONSTOP);
				// Delete the source file
				}

				// Check is the App terminated? Else, Jump to "RollBack" routine before terminate current thread
				if (WAIT_OBJECT_0 == WaitForSingleObject(hEvent, 100)) {goto RollBack;}
			}

			// Reset the local variable
			ZeroMemory(szTmpBinFile1, sizeof(szTmpBinFile1));
			ZeroMemory(szTmpBinFile2, sizeof(szTmpBinFile2));
		}


		// Check is the App terminated? Else, terminate current thread
		if (WAIT_OBJECT_0 == WaitForSingleObject(hEvent, 100)) {goto CleanExit;}



		// STAGE #7
		// Purging all the allocated/used memory/temp file
		{
			// #GUI UPDATE#
			// Set the initial position
			SendDlgItemMessage(hWnd, IDC_PROGRESS1, PBM_SETPOS, 50, 0);
			// Refresh the window
			UpdateWindow(GetDlgItem(hWnd, IDC_PROGRESS1));

			// Update the status text
			SetDlgItemText(hWnd, IDC_PROGRESSVALUE, "50%");
			// Refresh the window
			UpdateWindow(GetDlgItem(hWnd, IDC_PROGRESSVALUE));



			//	IF YOU WISH TO UPDATE SOME OF THE REGISTRY ENTRY, PLEASE PLACE YOUR CODE HERE
			//	
			//	YOU MIGHT NEED TO PUT OTHERS TASK AT THIS SECTION AND THIS IS 100% BASE ON
			//	YOUR APPLICATION NEED.

		}


		// Check is the App terminated? Else, terminate current thread
		if (WAIT_OBJECT_0 == WaitForSingleObject(hEvent, 100)) {goto CleanExit;}


		// STAGE #8
		// FINAL TOUCH UP OF THE ENTIR EPROCESS
		//	
		{
			// #GUI UPDATE#
			// Set the initial position
			SendDlgItemMessage(hWnd, IDC_PROGRESS1, PBM_SETPOS, 100, 0);
			// Refresh the window
			UpdateWindow(GetDlgItem(hWnd, IDC_PROGRESS1));

			// Update the status text
			SetDlgItemText(hWnd, IDC_PROGRESSVALUE, "100%");
			// Refresh the window
			UpdateWindow(GetDlgItem(hWnd, IDC_PROGRESSVALUE));

			// Update the status text
			SetDlgItemText(hWnd, IDC_STATUS, "Update completed.");
			// Refresh the window
			UpdateWindow(GetDlgItem(hWnd, IDC_STATUS));

			// Notify the user about the process completed.
			MessageBox(hWnd, "All files successfully extract to the specified folder.", APP_TITLE, MB_OK | MB_ICONINFORMATION);


			// Show the release update note
			if (0 < strlen(si.szAutoExecFile))
			{
				// Reset local variable
				ZeroMemory(szTmpBinFile1, sizeof(szTmpBinFile1));
				// Build the launching application path
				sprintf(szTmpBinFile1,
						"%s%s",
						szExtractPath, si.szAutoExecFile);
				// Launch the MailGate
				ShellExecute(hWnd, "open", szTmpBinFile1, NULL, szExtractPath, SW_SHOW);
				
				// NOTE:
				//		THIS IS TO AVOID THE FILE (SPECIFIED IN THE szTmpBinFile1) WAS
				//		BEING DELETED AT THE "CleanExit" SECTION.
				//
				// Reset the local variable
				ZeroMemory(szTmpBinFile1, sizeof(szTmpBinFile1));
			}


			// #ACUTAL TASK#
			//	IF YOU HAVE TO CLOSE SOME RELATED PROGRAM AT THE BEGINING OF THIS THREAD,
			//	AND YOU WISH TO RELOAD IT AFTER COMPLETED UPDATE THE RELATED PROGRAM FILE.
			//	THEN, PLEASE PLACE YOUR CODE HERE

			// Jump to "CleanExit"
			goto CleanExit;

		}

	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		// PUT YOUR ERROR HANDLING CODE HERE

	}

RollBack:
	// PERFORM TEMPORARY BINDARY FILE CLEAN UP IN WINDOW TEMP FOLDER
	
	// #GUI UPDATE#
	// Update the status text
	SetDlgItemText(hWnd, IDC_STATUS, "Rollback the updated file...");
	// Refresh the window
	UpdateWindow(hWnd);	
	
	// #ACUTAL TASK#
	// Close the current open file handle
	if (NULL != hFile) {CloseHandle(hFile);}
	hFile = NULL;

	// Walk through the expanded binary data file index
	for (dwIndex2=dwIndex1; 0<=dwIndex2; dwIndex2--)
	{
		// Calculate the current position
		lCurPosition = (long)(((double)(dwIndex2)/si.dwFileCount)*100);
		// Update the progress bar position
		SendDlgItemMessage(hWnd, IDC_PROGRESS1, PBM_SETPOS, lCurPosition, 0);
		// Refresh the window
		UpdateWindow(GetDlgItem(hWnd, IDC_PROGRESS1));

		// Reset local variable
		ZeroMemory(szTmpBinFile2, sizeof(szTmpBinFile2));
		// Build the current binary file name
		sprintf(szTmpBinFile2,
				"%s%s",
				szWinTmpPath, lpefi[dwIndex2].szBinFileName); 
		// Delete the current binary file
		DeleteFile(szTmpBinFile2);

		// Exit the for loop when the dwIndex2 = 0
		if (0 == dwIndex2) {break;}
	}


CleanExit:
	// PERFORM ALL THE CLEAN UP JOB FOR THOSE ALLOCATED/USED RESOURCES

	// Close all the open file
	if (NULL != hFile) {CloseHandle(hFile);}
	hFile = NULL;

	// Release all the allocated/used local variable
	if (NULL != lpefi) {LocalFree((LPEXTRACTFILEINFO)lpefi);}
	lpefi = NULL;

	if (NULL != lpBinData) {LocalFree((LPBYTE)lpBinData);}
	lpBinData = NULL;

	// Delete all the created temp file (if any)
	if (0 < strlen(szTmpBinFile1)) {DeleteFile(szTmpBinFile1);}
	if (0 < strlen(szTmpBinFile2)) {DeleteFile(szTmpBinFile2);}

	// Set the initial position
	SendDlgItemMessage(hWnd, IDC_PROGRESS1, PBM_SETPOS, 0, 0);
	// Update the status text
	SetDlgItemText(hWnd, IDC_PROGRESSVALUE, "");
	SetDlgItemText(hWnd, IDC_STATUS, "");
	// Refresh the window
	UpdateWindow(hWnd);

	// Terminate current application
	PostMessage(hWnd, WM_DESTROY, 0, 0);

	// Set return value
	return 0;

}


