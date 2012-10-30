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


#include "inc\SelfExtract.h"


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
	icex.dwICC  = ICC_LISTVIEW_CLASSES;
	// Initialize the Windows Common Control Library
	InitCommonControlsEx (&icex);

	// Create main form
	hWndMain = CreateDialog(hInst, MAKEINTRESOURCE(IDD_MAINWND), NULL, (DLGPROC)MainWndProc);

	if (hWndMain == NULL)
	{
		// Notified your about the failure
		MessageBox(NULL, TEXT("Fail to start self-extract list generating program!"), APP_TITLE, MB_OK | MB_ICONEXCLAMATION);
		// Set the return value
		return false;
	}
/*
	else
		// Display the dialouge
		ShowWindow(hWndMain, nCmdShow);
*/

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
	static HICON	hIcon2 = NULL;

	LVCOLUMN	pcol	= {NULL};
	int			nIndex	= 0;

	DWORD		dwTime		= 0;
	DWORD		dwExitCode1 = 0;
	DWORD		dwExitCode2 = 0;

	switch (message) 
	{
		case WM_INITDIALOG:
			// Update Window Caption
			SetWindowText(hWnd, APP_TITLE);

			// Create the main menu
			hMenu = LoadMenu(hInst, (LPCTSTR)IDR_MENU1);
			// Assign the loaded menu to the current dialog
			SetMenu(hWnd, hMenu);


			// Set the dialog ICON
			hIcon1 = LoadIcon(hInst, (LPCTSTR)IDI_ICON1);
			SendMessage(hWnd, WM_SETICON, ICON_BIG, (LPARAM)hIcon1);

			// Set the button icon
			hIcon2 = (HICON)LoadImage(hInst, MAKEINTRESOURCE(IDI_ICON2), IMAGE_ICON, 16, 16, LR_DEFAULTCOLOR);
			SendDlgItemMessage(hWnd, IDC_BUTTON1, BM_SETIMAGE, IMAGE_ICON, (LPARAM)hIcon2);
			SendDlgItemMessage(hWnd, IDC_BUTTON2, BM_SETIMAGE, IMAGE_ICON, (LPARAM)hIcon2);

			// Set the progressbar Min,Max value
			SendDlgItemMessage(hWnd, IDC_PROGRESS1,  PBM_SETRANGE, 0, MAKELPARAM(1, 100));


			// Setup the listview column header
			{
				// Modified the Listview style by adding a FullRowSelect capability
				SendMessage(GetDlgItem(hWnd, IDC_LIST1),
							LVM_SETEXTENDEDLISTVIEWSTYLE,
							0,
							LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | LVS_EX_HEADERDRAGDROP);
				
				// Walk through the COLUMNHEADERINFO structure
				for(nIndex=0; nIndex<sizeof(chi)/sizeof(COLUMNHEADERINFO); nIndex++)
				{
					// Initialize listview column structure
					pcol.mask		= LVCF_FMT | LVCF_WIDTH | LVCF_TEXT;
					pcol.fmt		= chi[nIndex].ColFormat;
					pcol.pszText	= chi[nIndex].ColName;
					pcol.cx			= chi[nIndex].ColWidth;
					// Append listview column
					SendMessage(GetDlgItem(hWnd, IDC_LIST1), LVM_INSERTCOLUMN, nIndex, (LPARAM)&pcol);
				};
			}

			// Get the current window temporary folder
			GetTempPath(sizeof(szWinTmpPath), szWinTmpPath);

			// Create the "CreateSelfExtractFile" thread control event
			hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
			//
			return TRUE;

		case WM_COMMAND:
			switch (LOWORD(wParam))
			{
				case IDC_BUTTON1:
					// Get the current user define Source folder
					GetOutputFolder (hWnd, wParam);
					// Get all the distributed filename from the current selected folder
					GetDistributeFileList(hWnd);
					//
					break;

				case IDC_BUTTON2:
					// Get the current user select self-extrat data filename
					GetSelfExtractFileName (hWnd, wParam);
					//
					break;

				case IDM_FILE_EXIT:
					// Destroy the current application
					DestroyWindow(hWnd);
					//
					break;

				case IDM_HELP_ABOUT:
					// Show the "Aboutbox" dialog
					DialogBox(hInst, (LPCTSTR)IDD_ABOUTBOX, hWnd, (DLGPROC)AboutBoxProc);
					//
					break;

				case IDM_ACTION_BUILD:
					// Reset the thread control event
					ResetEvent(hEvent);
					// Begin create the self-extract data file
					hThread1 = CreateThread(NULL,
										    0,
										    &CreateSelfExtractFile,
										    hWnd,
										    0,
										    &dwThreadId1);
					//
					break;

				case IDM_ACTION_STOP_BUILD:
					// Cancel the current Build/Browse
					// Get the current thread exit code state
					if (NULL != hThread1) {GetExitCodeThread(hThread1, &dwExitCode1);}
					if (NULL != hThread2) {GetExitCodeThread(hThread2, &dwExitCode2);}
					// Check both the thread state. If remain active, set the "CreateSelfExtractFile" control event
					if (STILL_ACTIVE == dwExitCode1 || STILL_ACTIVE == dwExitCode2) {SetEvent(hEvent);}
					//
					break;

				case IDM_ACTION_BROWSE:
					// Reset the thread control event
					ResetEvent(hEvent);
					// Begin browse the self-extract file content
					hThread2 = CreateThread(NULL,
										    0,
										    &BrowseSelfExtractFile,
										    hWnd,
										    0,
										    &dwThreadId2);
					//
					break;

				default:
					// DO NOTHING
					break;

			}
			//
			break;

		case WM_CLOSE:
		case WM_DESTROY:
			// Get the current thread exit code state
			if (NULL != hThread1) {GetExitCodeThread(hThread1, &dwExitCode1);}
			if (NULL != hThread2) {GetExitCodeThread(hThread2, &dwExitCode2);}

			// Check both the thread state
			if (STILL_ACTIVE == dwExitCode1 || STILL_ACTIVE == dwExitCode2)
			{
				// Set the "CreateSelfExtractFile" control event
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
			
			// release the used memory
			if (NULL != hEvent) {CloseHandle(hEvent);}
			hEvent = NULL;

			// Destroy the created menu
			if (NULL != hMenu) {DestroyMenu(hMenu);}
			hMenu = NULL;

			// Delete the loaded ICON
			if (NULL != hIcon1) {DestroyIcon(hIcon1);}
			hIcon1 = NULL;
			if (NULL != hIcon2) {DestroyIcon(hIcon2);}
			hIcon2 = NULL;

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
//		- AboutBox dialouge callback procedure
//  PARAMETERS:
//		- Standard window callback procedure parameters
//  OPERATION:
//		- Customize
//  RETURN VALUE:
//      - TRUE/FALSE

LRESULT CALLBACK AboutBoxProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	LOGFONT	lf;
	HDC		hdc = NULL;
	
	static	HBRUSH	hBrush1 = NULL;
	static	HFONT	hFont1  = NULL;
	static	HFONT	hFont2  = NULL;

	LPTSTR	pszBuffer	    = NULL;
	LPTSTR	pszPath		    = NULL;

	TCHAR	szBuffer[1024]  = {NULL};
	
	// FOR LISTVIEW USED ONLY
	LVCOLUMN	pcol	= {NULL};
	int			nIndex	= 0;

	switch (message) 
	{
		case WM_INITDIALOG:
			// Create the background color brush
			hBrush1 = CreateSolidBrush(GetSysColor(COLOR_BTNFACE));
			
			{
				// Get the current device context
				hdc = GetDC(hWnd);
				// Initialize the LOGFONT structure
				memset(&lf, '\0', sizeof(LOGFONT));
				// Setup the LOGFONT structure
				lf.lfHeight			= -MulDiv(7, GetDeviceCaps(hdc, LOGPIXELSY), 72);
				lf.lfWidth			= 0;
				lf.lfWeight			= FW_NORMAL;
				lf.lfEscapement		= 0;
				lf.lfOrientation	= 0;
				lf.lfItalic			= false;
				lf.lfUnderline		= false;
				lf.lfStrikeOut		= false;
				lf.lfCharSet		= ANSI_CHARSET;
				strcpy(lf.lfFaceName, TEXT("Tahoma"));
				// Create the define LOGFONT
				hFont1 = CreateFontIndirect(&lf);

				// Initialize the LOGFONT structure
				memset(&lf, '\0', sizeof(LOGFONT));
				// Setup the LOGFONT structure
				lf.lfHeight			= -MulDiv(10, GetDeviceCaps(hdc, LOGPIXELSY), 72);
				lf.lfWidth			= 0;
				lf.lfWeight			= FW_BOLD;
				lf.lfEscapement		= 0;
				lf.lfOrientation	= 0;
				lf.lfItalic			= false;
				lf.lfUnderline		= false;
				lf.lfStrikeOut		= false;
				lf.lfCharSet		= ANSI_CHARSET;
				strcpy(lf.lfFaceName, TEXT("Tahoma"));
				// Create the define LOGFONT
				hFont2 = CreateFontIndirect(&lf);

				// Assign the new font to the 'Warning Message' static control
				SendDlgItemMessage(hWnd, IDC_WARNING,    WM_SETFONT, (WPARAM)hFont1, true);
				SendDlgItemMessage(hWnd, IDC_DISCLAIMER, WM_SETFONT, (WPARAM)hFont1, true);
				// Assign the new font to the 'Appplication Title' static control
				SendDlgItemMessage(hWnd, IDC_APP_TITLE,  WM_SETFONT, (WPARAM)hFont2, true);

				// Free the used DC
				ReleaseDC(hWnd, hdc);
			}
			
			// Load the resource string
			LoadString(hInst, IDS_WARNING, szBuffer, 1024);
			// Assign new text to the control
			SetDlgItemText(hWnd, IDC_WARNING, szBuffer);

			{
				// Allocate memory
				pszBuffer = (LPTSTR)LocalAlloc(LPTR, 260);
				pszPath = (LPTSTR)LocalAlloc(LPTR, 260);
				// Get current WDX Services path...
				if (0 != GetModuleFileName(hInst, pszPath, 260))
				{
					// Get Dll version...
					GetFileVersion(pszPath, pszBuffer, 260);
					//Update display Text
					SetDlgItemText(hWnd, IDC_APP_VERSION, pszBuffer);
				}
				// Release used memory
				LocalFree((LPTSTR)pszBuffer);
				LocalFree((LPTSTR)pszPath);
			}

			// Modified the Listview style to full-row selection
			SendDlgItemMessage(hWnd,
							   IDC_LIST1,
							   LVM_SETEXTENDEDLISTVIEWSTYLE,
							   0,
							   LVS_EX_FULLROWSELECT);

			// Setup dependacies listview column header...
			{
				// Setup column names.
				for(nIndex=0; nIndex<sizeof(ColHeaderInfo)/sizeof(COLUMNHEADERINFO); nIndex++)
				{
					// Initialize listview column structure
					pcol.mask		= LVCF_FMT | LVCF_WIDTH | LVCF_TEXT;
					pcol.fmt		= chi[nIndex].ColFormat;
					pcol.pszText	= chi[nIndex].ColName;
					pcol.cx			= chi[nIndex].ColWidth;

					// Append listview column
					SendDlgItemMessage(hWnd, IDC_LIST1, LVM_INSERTCOLUMN, nIndex, (LPARAM)&pcol);
				};
			}


			// Validation
			if (NULL != GetDlgItem(hWnd, IDC_LIST1))
			{
				// NOTE:
				//		THE FOLLOWING CODE WAS REMARK, DUE TO GSM DRIVER WAS UPGRADED
				//		FROM WIN32 DLL TO COM+ ACTIVEX DLL
				//		
				//		IF YOU REQUIRE TO DISPLAY SOME EXTERNAL DLL INFORMATION,
				//		THEN CALL THE LoadLibrary TO GET THE DLL INSTANCE. FOLLOW BY
				//		CALL THE GetComponentVer TO APPEND THE RESPECTIVE INFORMATION
				//		INTO THE LISTVIEW CONTORL.
				//		LAST, CALL THE FreeLibrary TO RELEASE THE LOADED DLL.
			}
			//
			return TRUE;

		case WM_CTLCOLORSTATIC:
			// NOTE:
			//		wParam :: handle to DC (HDC)
			//		lParam :: handle to static control (HWND)
			//
			if ((HWND)lParam == GetDlgItem(hWnd, IDC_APP_TITLE))
			{
				SetTextColor((HDC)wParam, RGB(0, 0, 255));
				SetBkMode((HDC)wParam, TRANSPARENT);
				return (long)hBrush1;
			}
			//
			break;

		case WM_CLOSE:
		case WM_DESTROY:
			// Delete the used font
			if (NULL != hFont1) {DeleteObject(hFont1);}
			hFont1 = NULL;
			if (NULL != hFont2) {DeleteObject(hFont2);}
			hFont2 = NULL;
			if (NULL != hBrush1) {DeleteObject(hBrush1);}
			hBrush1 = NULL;

			// Destroy current dialouge window
			EndDialog(hWnd, LOWORD(wParam));
			//
			break;

		default:
			// DO NOTHING
			return 0;

   }
   return 0;
}


void CreateSelfExtractFileIn (HWND hWnd,char* szBuffer1,char* prefix=""){
	WIN32_FIND_DATA	wfs		= {NULL};
	HANDLE	hFile	= NULL;
	char	szBuffer2[1000]	= {NULL};
	long	lCurPosition = 0;
	DWORD	dwCount2	 = 0;

	sprintf(szBuffer2,
			"%s\\%s*.*",
			szBuffer1,prefix);
	// Start scaning the directory
	hFile = FindFirstFile(szBuffer2, &wfs);

	// Check the return handle value
	do
	{
		// Check is the current found file is directory?
		// Increase local variable
		dwCount2++;

		// #UPDATE GUI#
		// Calculate the current position
		lCurPosition = (long)(((double)(dwCount2)/dwTotalFile)*100);
		// Update the progress bar position
		SendDlgItemMessage(hWnd, IDC_PROGRESS1, PBM_SETPOS, lCurPosition, 0);
		// Refresh the window
		UpdateWindow(GetDlgItem(hWnd, IDC_PROGRESS1));
					

		// Reset local variable

		// #ACTUAL TASK#
		// Save the information into data file
		if(!(FILE_ATTRIBUTE_DIRECTORY & wfs.dwFileAttributes)){
			// Save the information into data file
			WriteSelfExtractHeader (hWnd, &wfs, prefix, false);
			WriteSelfExtractBinData (hWnd, &wfs, prefix);
			RefreshListviewContent (hWnd, &wfs);
		}
		else if(wfs.cFileName[0]!='.'){
			char temp[2000]={NULL};
			sprintf(temp,"%s%s\\",prefix,wfs.cFileName);
			WriteSelfExtractHeader(hWnd, &wfs, prefix, true);
			CreateSelfExtractFileIn(hWnd, szBuffer1, temp);
			RefreshListviewContent (hWnd, &wfs);
		}
		char temp1[2000]={NULL};
		// Format the current display status
		sprintf(temp1,
				" Reading ...\\%s",
				wfs.cFileName); 
		// Update the status text
		SetDlgItemText(hWnd, IDC_STATUS, temp1);
		// Refesh the window
		UpdateWindow(GetDlgItem(hWnd, IDC_STATUS));

			// Update listview control
		// Scan the next match item in the directory
		if (!FindNextFile(hFile, &wfs))
		{
			if (ERROR_NO_MORE_FILES == GetLastError()) {break;}
		}

		// Check is the App terminated? Else, terminate current thread
		//if (WAIT_OBJECT_0 == WaitForSingleObject(hEvent, 100)) {goto CleanExit;}

	} while (NULL != hFile || INVALID_HANDLE_VALUE != hFile);
}


//  PURPOSE:  
//		- Scan the current folder for all the file require to
//		  distribute & include into the self-extract package
//  PARAMETERS:
//		- hWnd	:: Parent window handle
//  OPERATION:
//		- Call "FindFirstFile", "FindNextFile", "FindClose" API
//  RETURN VALUE:
//      - NIL
//
DWORD WINAPI CreateSelfExtractFile (LPVOID pParam)
{
	WIN32_FIND_DATA	wfs		= {NULL};
	HANDLE	hFile	= NULL;
	HANDLE	hFile2	= NULL;

	HRSRC	hResource	= 0;
	HGLOBAL	hResData	= 0;
	DWORD	dwResSize	= 0;
	DWORD	dwFileSize	= 0;
	DWORD	dwByteWrite = 0;
	DWORD	dwByteRead	= 0;

	LPVOID	lpData		= NULL;
	LPBYTE	lpBinData	= NULL;

	char	szBuffer1[MAX_PATH]	= {NULL};
	char	szBuffer2[MAX_PATH]	= {NULL};

	long	lCurPosition = 0;
	DWORD	dwCount2	 = 0;

	HWND	hWnd	= NULL;


	__try
	{
		// Map the pass in arguement
		hWnd = (HWND)pParam;

		// PRE-EXTRACT STAGE#1
		{
			// #UPDATE GUI#
			// Enable/Disable the menu
			EnableMenuItem(hMenu, IDM_ACTION_BUILD,		 MF_BYCOMMAND | MF_GRAYED);
			EnableMenuItem(hMenu, IDM_ACTION_BROWSE,	 MF_BYCOMMAND | MF_GRAYED);
			EnableMenuItem(hMenu, IDM_ACTION_STOP_BUILD, MF_BYCOMMAND | MF_ENABLED);

			// Disable control
			EnableWindow(GetDlgItem(hWnd, IDC_BUTTON1), FALSE);
			EnableWindow(GetDlgItem(hWnd, IDC_BUTTON2), FALSE);
			EnableWindow(GetDlgItem(hWnd, IDC_COMBO1),  FALSE);

			// Clear the current listview content
			SendDlgItemMessage(hWnd,
							   IDC_LIST1,
							   LVM_DELETEALLITEMS,
							   0,
							   0);

			// Set the initial position
			SendDlgItemMessage(hWnd, IDC_PROGRESS1, PBM_SETPOS, 0, 0);
			// Refresh the window
			UpdateWindow(GetDlgItem(hWnd, IDC_PROGRESS1));

			// Update the status text
			SetDlgItemText(hWnd, IDC_STATUS, " Initializing...");
			// Refesh the window
			UpdateWindow(GetDlgItem(hWnd, IDC_STATUS));

			// #INITIALIZE LOCAL VARIABLE#
			// Reset the global file count
			dwFileCount = 0;

			// Reset global variable
			ZeroMemory(szTmpBinFile1, sizeof(szTmpBinFile1));
			// Format the full path for the output filename
			sprintf(szTmpBinFile1,
					"%s\\%08X",
					szWinTmpPath, (long)(GetTickCount()*12345)/5987);
			// Delete the existing file in the window temporary folder
			if (0 < strlen(szTmpBinFile1)) {DeleteFile(szTmpBinFile1);}

			// Reset global variable
			ZeroMemory(szTmpBinFile2, sizeof(szTmpBinFile2));
			// Get the binary temporary data file name
			sprintf(szTmpBinFile2,
					"%s\\%08X",
					szWinTmpPath, (long)(GetTickCount()*8253)/971);
			// Delete the existing file in the window temporary folder
			if (0 < strlen(szTmpBinFile2)) {DeleteFile(szTmpBinFile2);}

			// Reset global variable
			ZeroMemory(szTmpBinFile3, sizeof(szTmpBinFile3));
			// Get the binary temporary data file name
			sprintf(szTmpBinFile3,
					"%s\\%08X",
					szWinTmpPath, (long)(GetTickCount()*5846)/346);
			// Delete the existing file in the window temporary folder
			if (0 < strlen(szTmpBinFile3)) {DeleteFile(szTmpBinFile3);}

			// Reset global variable
			ZeroMemory(szTmpBinFile4, sizeof(szTmpBinFile4));
			// Get the binary temporary data file name
			sprintf(szTmpBinFile4,
					"%s\\%08X.exe",
					szWinTmpPath, GetTickCount());
			// Delete the existing file in the window temporary folder
			if (0 < strlen(szTmpBinFile4)) {DeleteFile(szTmpBinFile4);}


			// Reset local variable
			ZeroMemory(szSelfExtractFile, sizeof(szSelfExtractFile));
			// Get the user define output folder
			GetDlgItemText(hWnd, IDC_EDIT2, szSelfExtractFile, sizeof(szSelfExtractFile));
			// Delete the existing file in the window temporary folder
			if (0 < strlen(szSelfExtractFile)) {DeleteFile(szSelfExtractFile);}

			// Get the current user define source folder
			GetDlgItemText(hWnd, IDC_EDIT1, szBuffer1, sizeof(szBuffer1));
			// Format the full path for the source folder
			//sprintf(szBuffer2,"%s\\*.*",szBuffer1);
		}


		// Check is the App terminated? Else, terminate current thread
		//if (WAIT_OBJECT_0 == WaitForSingleObject(hEvent, 100)) {goto CleanExit;}


		// STAGE #1
		// Scan all the available file in the user define "Source folder"
		/*{
			// Start scaning the directory
			hFile = FindFirstFile(szBuffer2, &wfs);

			// Check the return handle value
			do
			{
				// Check is the current found file is directory?
				if (!(FILE_ATTRIBUTE_DIRECTORY & wfs.dwFileAttributes))
				{
					// Increase local variable
					dwCount2++;

					// #UPDATE GUI#
					// Calculate the current position
					lCurPosition = (long)(((double)(dwCount2)/dwTotalFile)*100);
					// Update the progress bar position
					SendDlgItemMessage(hWnd, IDC_PROGRESS1, PBM_SETPOS, lCurPosition, 0);
					// Refresh the window
					UpdateWindow(GetDlgItem(hWnd, IDC_PROGRESS1));
					

					// Reset local variable
					ZeroMemory(szBuffer1, sizeof(szBuffer1));
					// Format the current display status
					sprintf(szBuffer1,
							" Reading ...\\%s",
							wfs.cFileName); 
					// Update the status text
					SetDlgItemText(hWnd, IDC_STATUS, szBuffer1);
					// Refesh the window
					UpdateWindow(GetDlgItem(hWnd, IDC_STATUS));


					// #ACTUAL TASK#
					// Save the information into data file
					WriteSelfExtractHeader (hWnd, &wfs);
					// Save the information into data file
					WriteSelfExtractBinData (hWnd, &wfs);
					// Update listview control
					RefreshListviewContent (hWnd, &wfs);
				}

				// Scan the next match item in the directory
				if (!FindNextFile(hFile, &wfs))
				{
					if (ERROR_NO_MORE_FILES == GetLastError()) {break;}
				}

				// Check is the App terminated? Else, terminate current thread
				//if (WAIT_OBJECT_0 == WaitForSingleObject(hEvent, 100)) {goto CleanExit;}

			} while (NULL != hFile || INVALID_HANDLE_VALUE != hFile);
		}*/

		CreateSelfExtractFileIn(hWnd,szBuffer1);
		// Check is the App terminated? Else, terminate current thread
		//if (WAIT_OBJECT_0 == WaitForSingleObject(hEvent, 100)) {goto CleanExit;}


		// STAGE #2
		// @1. Merge the self-extract file information and actual binary file data
		// @2. Then follow by compressed the merge file by using the ZLib.lib
		//     compression algorithm.
		// @3. Final copy the self-extract file into user define location & filename.
		// into a single file
		{
			// #UPDATE GUI#
			// Update the progress bar position
			SendDlgItemMessage(hWnd, IDC_PROGRESS1, PBM_SETPOS, 0, 0);
			// Refresh the window
			UpdateWindow(GetDlgItem(hWnd, IDC_PROGRESS1));

			// Update the status text
			SetDlgItemText(hWnd, IDC_STATUS, " Binding read data file...");
			// Refesh the window
			UpdateWindow(GetDlgItem(hWnd, IDC_STATUS));

	
			// #ACTUAL TASK#
			// Update the total file count value
			if (TRUE == MergeSelfExtractData (hWnd))
			{
				// #UPDATE GUI#
				// Update the progress bar position
				SendDlgItemMessage(hWnd, IDC_PROGRESS1, PBM_SETPOS, 20, 0);
				// Refresh the window
				UpdateWindow(GetDlgItem(hWnd, IDC_PROGRESS1));

				// Update the status text
				SetDlgItemText(hWnd, IDC_STATUS, " Compressing data file...");
				// Refesh the window
				UpdateWindow(GetDlgItem(hWnd, IDC_STATUS));


				// # ACTUAL TASK#
				// Compress the current data file to the user define location & 
				// Notify user about the process is completed
				if (0 == Compress(szTmpBinFile1, szTmpBinFile3))
				{
					// Check is the App terminated? Else, terminate current thread
					if (WAIT_OBJECT_0 == WaitForSingleObject(hEvent, 100)) {goto CleanExit;}

					// #UPDATE GUI#
					// Update the progress bar position
					SendDlgItemMessage(hWnd, IDC_PROGRESS1, PBM_SETPOS, 40, 0);
					// Refresh the window
					UpdateWindow(GetDlgItem(hWnd, IDC_PROGRESS1));

					// Reset local variable
					ZeroMemory(szBuffer1, sizeof(szBuffer1));
					// Format the display status
					sprintf(szBuffer1,
							" Spawning self-extract kernel ...",
							szSelfExtractFile);


					// #ACTUAL TASK# (CORE#1)
					// Create the SetupEx.exe by reading the pre-created binary data
					// from the custom resource table
					{
						// Locate the resource template handle
						hResource = FindResource(hInst, MAKEINTRESOURCE(IDR_EXTRACTOR1), "EXTRACTOR");
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
							hFile2 = CreateFile(szTmpBinFile4,
											    GENERIC_WRITE,
											    FILE_SHARE_WRITE,
											    NULL,
											    CREATE_ALWAYS,
											    FILE_ATTRIBUTE_NORMAL,
											    NULL);

							// Check the return handle value
							if (NULL != hFile2 && INVALID_HANDLE_VALUE != hFile2)
							{
								// Move the file pointer to the begin of the file
								SetFilePointer(hFile2, 0, 0, FILE_BEGIN);
								// Write the read data into a temp file
								WriteFile(hFile2,
										  (LPBYTE)lpData,
										  dwResSize,
										  &dwByteWrite,
										  NULL);
								// Close the current open data file
								if (NULL != hFile2) {CloseHandle(hFile2);}
								hFile2 = NULL;
							}
							else
							{
								// Notify user about the error
								MessageBox(hWnd, "Fail to spawning the self-extract kernel!", APP_TITLE, MB_OK | MB_ICONSTOP);
								// Jump the the "CleanExit" routine
								goto CleanExit;
							}
						}
						else
						{
							// Notify user about the error
							MessageBox(hWnd, "Fail to read the self-extract kernel binary data!", APP_TITLE, MB_OK | MB_ICONSTOP);
							// Jump the the "CleanExit" routine
							goto CleanExit;
						}
					}


					// #UPDATE GUI#
					// Update the progress bar position
					SendDlgItemMessage(hWnd, IDC_PROGRESS1, PBM_SETPOS, 60, 0);
					// Refresh the window
					UpdateWindow(GetDlgItem(hWnd, IDC_PROGRESS1));

					// Update the status text
					SetDlgItemText(hWnd, IDC_STATUS, " Generating self-extract file ...");
					// Refesh the window
					UpdateWindow(GetDlgItem(hWnd, IDC_STATUS));


					// #ACTUAL TASK# (CORE#2)
					// Modify the current spawned self-extract program, but inserting the 
					// newly pack and compressed selected binary file data into the 
					// IDR_SETUP1 resource under the custom resource of "SETUP"
					{
						// Read the current compressed bindary data
						hFile2 = CreateFile(szTmpBinFile3,
										    GENERIC_READ,
										    FILE_SHARE_READ,
										    NULL,
										    OPEN_EXISTING,
										    FILE_ATTRIBUTE_NORMAL,
										    NULL);

						// Check the return handle value
						if (NULL != hFile2 && INVALID_HANDLE_VALUE != hFile2)
						{
							// Get the current file size
							dwFileSize = GetFileSize(hFile2, NULL);
							
							// Allocate & initialize local memory buffer.
							lpBinData = (LPBYTE)LocalAlloc(LPTR, dwFileSize);
							ZeroMemory(lpBinData, dwFileSize);

							// Move the file pointer to the begin of the file
							SetFilePointer(hFile2, 0, 0, FILE_BEGIN);
							// Read data into a local buffer
							ReadFile(hFile2,
									 lpBinData,
									 dwFileSize,
									 &dwByteRead,
									 NULL);
							// Close the current open data file
							if (NULL != hFile2) {CloseHandle(hFile2);}
							hFile2 = NULL;
						}
						else
						{
							// Notify user about the error
							MessageBox(hWnd, "Fail to read the compressed binary data!", APP_TITLE, MB_OK | MB_ICONSTOP);
							// Jump the the "CleanExit" routine
							goto CleanExit;
						}


						
						// Modify the szTmpBinFile4 resource table
						{
							// Open the file require to alter the resource table
							hFile2 = BeginUpdateResource (szTmpBinFile4, FALSE);

							// Check the return handle value
							if (NULL != hFile2 && INVALID_HANDLE_VALUE != hFile2)
							{
								// Update the file resource table
								if (FALSE == UpdateResource (hFile2,
															 "SETUP",
															 MAKEINTRESOURCE(IDR_SETUP1),
															 MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US),
															 lpBinData,
															 dwFileSize))
								{
									// Notify user about the error
									MessageBox(hWnd, "Fail to update the resource table of the self-extract executable file!", APP_TITLE, MB_OK | MB_ICONSTOP);
									// Reset the local variable
									hFile2 = NULL;
									// Jump the the "CleanExit" routine
									goto CleanExit;
								}

								// Close the modify file
								if (FALSE == EndUpdateResource (hFile2, FALSE))
								{
									// Notify user about the error
									MessageBox(hWnd, "Fail to modify the resource table of the self-extract executable file!", APP_TITLE, MB_OK | MB_ICONSTOP);
									// Reset the local variable
									hFile2 = NULL;
									// Jump the the "CleanExit" routine
									goto CleanExit;
								}

								// Reset the local variable
								hFile2 = NULL;

								// Release the allocated memory
								if (NULL != lpBinData) {LocalFree((LPBYTE)lpBinData);}
								lpBinData = NULL;
							}
							else
							{
								// Notify user about the error
								MessageBox(hWnd, "Fail to open the resource table of the self-extract executable file!", APP_TITLE, MB_OK | MB_ICONSTOP);
								// Reset the local variable
								hFile2 = NULL;
								// Jump the the "CleanExit" routine
								goto CleanExit;
							}
						}
					}


					// #UPDATE GUI#
					// Update the progress bar position
					SendDlgItemMessage(hWnd, IDC_PROGRESS1, PBM_SETPOS, 80, 0);
					// Refresh the window
					UpdateWindow(GetDlgItem(hWnd, IDC_PROGRESS1));


					// # ACTUAL TASK#
					// Transfer the file to the user specified name & location
					CopyFile(szTmpBinFile4, szSelfExtractFile, FALSE);


					// #UPDATE GUI#
					// Update the progress bar position
					SendDlgItemMessage(hWnd, IDC_PROGRESS1, PBM_SETPOS, 100, 0);
					// Refresh the window
					UpdateWindow(GetDlgItem(hWnd, IDC_PROGRESS1));

					// Update the status text
					SetDlgItemText(hWnd, IDC_STATUS, " Completed");
					// Refesh the window
					UpdateWindow(GetDlgItem(hWnd, IDC_STATUS));


					// Notify user about the process completed
					MessageBox(hWnd, "self-extract file successful created.", APP_TITLE, MB_OK | MB_ICONINFORMATION);
				}
				else
					MessageBox(hWnd, "Fail to compress the self-extract file!", APP_TITLE, MB_OK | MB_ICONSTOP);
			}
			else
				// Notify user about the process is completed
				MessageBox(hWnd, "Fail to merge the selected file information!", APP_TITLE, MB_OK | MB_ICONSTOP);
		}
			

	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		// PUT YOUR ERROR HANDLING CODE HERE

		// Notify user about the error
		MessageBox(hWnd, "Problem occur while generating the self-extract file!", APP_TITLE, MB_OK | MB_ICONSTOP);
	}


CleanExit:
	// PERFORM ALL THE CLEAN UP JOB FOR THOSE ALLOCATED/USED RESOURCES

	// Close the search handle
	if (NULL != hFile) {FindClose(hFile);}
	hFile = NULL;

	// Close the search handle
	if (NULL != hFile2) {CloseHandle(hFile2);}
	hFile2 = NULL;

	if (NULL != lpBinData) {LocalFree((LPBYTE)lpBinData);}
	lpBinData = NULL;

	// Delete the temporary binary data file
	if (0 < strlen(szTmpBinFile1)) {DeleteFile(szTmpBinFile1);}
	if (0 < strlen(szTmpBinFile2)) {DeleteFile(szTmpBinFile2);}
	if (0 < strlen(szTmpBinFile3)) {DeleteFile(szTmpBinFile3);}
	if (0 < strlen(szTmpBinFile4)) {DeleteFile(szTmpBinFile4);}

	// #UPDATE GUI#
	// Update the progress bar position
	SendDlgItemMessage(hWnd, IDC_PROGRESS1, PBM_SETPOS, 0, 0);
	// Refresh the window
	UpdateWindow(GetDlgItem(hWnd, IDC_PROGRESS1));
	// Update the status text
	SetDlgItemText(hWnd, IDC_STATUS, "");
	// Refesh the window
	UpdateWindow(GetDlgItem(hWnd, IDC_STATUS));

	// Enable/Disable the menu
	EnableMenuItem(hMenu, IDM_ACTION_BUILD,		 MF_BYCOMMAND | MF_ENABLED);
	EnableMenuItem(hMenu, IDM_ACTION_BROWSE,	 MF_BYCOMMAND | MF_ENABLED);
	EnableMenuItem(hMenu, IDM_ACTION_STOP_BUILD, MF_BYCOMMAND | MF_GRAYED);

	// Enable control
	EnableWindow(GetDlgItem(hWnd, IDC_BUTTON1), TRUE);
	EnableWindow(GetDlgItem(hWnd, IDC_BUTTON2), TRUE);
	EnableWindow(GetDlgItem(hWnd, IDC_COMBO1),  TRUE);

	// Clear the current listview content
	SendDlgItemMessage(hWnd,
					   IDC_LIST1,
					   LVM_DELETEALLITEMS,
					   0,
					   0);
			
	// Reset global variable
	hThread1 = NULL;

	// Set return value
	return 0;
}



//  PURPOSE:  
//		- Read the self-extract content & display on the listview
//  PARAMETERS:
//		- hWnd	:: Parent window handle
//  OPERATION:
//		- ...
//  RETURN VALUE:
//      - NIL
//
DWORD WINAPI BrowseSelfExtractFile (LPVOID pParam)
{
	SETUPINFO			si	  = {NULL};
	LPEXTRACTFILEINFO	lpesi = NULL;
	WIN32_FIND_DATA		wfs	  = {NULL};
	
	HMODULE	hExeFile	 = NULL;
	HANDLE	hFile		 = NULL;
	HRSRC	hResource	 = 0;
	HGLOBAL	hResData	 = 0;
	DWORD	dwResSize	 = 0;
	DWORD	dwFileSize	 = 0;
	DWORD	dwByteRead   = 0;
	DWORD	dwByteWrite  = 0;

	LPVOID	lpBinData	 = NULL;


	HWND	hWnd		 = NULL;

	int		nIndex		 = 0;
	long	lCurPosition = 0;

	char	szBuffer[MAX_PATH]	   = {NULL};


	__try
	{
		// Map the pass in arguement
		hWnd = (HWND)pParam;

		// PRE-EXTRACT STAGE
		{
			// #UPDATE GUI#
			// Enable/Disable the menu
			EnableMenuItem(hMenu, IDM_ACTION_BUILD,		 MF_BYCOMMAND | MF_GRAYED);
			EnableMenuItem(hMenu, IDM_ACTION_BROWSE,	 MF_BYCOMMAND | MF_GRAYED);
			EnableMenuItem(hMenu, IDM_ACTION_STOP_BUILD, MF_BYCOMMAND | MF_ENABLED);

			// Disable control
			EnableWindow(GetDlgItem(hWnd, IDC_BUTTON1), FALSE);
			EnableWindow(GetDlgItem(hWnd, IDC_BUTTON2), FALSE);

			// Clear all the available item in the combo box
			SendDlgItemMessage(hWnd, IDC_COMBO1, CB_RESETCONTENT, 0, 0);

			// Clear the current listview content
			SendDlgItemMessage(hWnd,
							   IDC_LIST1,
							   LVM_DELETEALLITEMS,
							   0,
							   0);

			// Set the initial position
			SendDlgItemMessage(hWnd, IDC_PROGRESS1, PBM_SETPOS, 0, 0);
			// Refresh the window
			UpdateWindow(GetDlgItem(hWnd, IDC_PROGRESS1));

			// Update the status text
			SetDlgItemText(hWnd, IDC_STATUS, " Initializing...");
			// Refesh the window
			UpdateWindow(GetDlgItem(hWnd, IDC_STATUS));

			// #INITIALIZE LOCAL VARIABLE#
			// Reset local variable
			ZeroMemory(szSelfExtractFile, sizeof(szSelfExtractFile));
			// Get the user define output folder
			GetDlgItemText(hWnd, IDC_EDIT2, szSelfExtractFile, sizeof(szSelfExtractFile));

			// Reset global variable
			ZeroMemory(szTmpBinFile1, sizeof(szTmpBinFile1));
			// Format the full path for the output filename
			sprintf(szTmpBinFile1,
					"%s\\%08X",
					szWinTmpPath, (long)(GetTickCount()*12345)/5987);
			// Delete the existing file in the window temporary folder
			if (0 < strlen(szTmpBinFile1)) {DeleteFile(szTmpBinFile1);}

			// Reset global variable
			ZeroMemory(szTmpBinFile2, sizeof(szTmpBinFile2));
			// Get the binary temporary data file name
			sprintf(szTmpBinFile2,
					"%s\\%08X",
					szWinTmpPath, (long)(GetTickCount()*8253)/971);
			// Delete the existing file in the window temporary folder
			if (0 < strlen(szTmpBinFile2)) {DeleteFile(szTmpBinFile2);}
			
		}


		// Check is the App terminated? Else, terminate current thread
		if (WAIT_OBJECT_0 == WaitForSingleObject(hEvent, 100)) {goto CleanExit;}


		// STAGE #1
		// Load the embedded data file from the self-extract executable file
		{
			// #UPDATE GUI#
			// Set the initial position
			SendDlgItemMessage(hWnd, IDC_PROGRESS1, PBM_SETPOS, 0, 0);
			// Refresh the window
			UpdateWindow(GetDlgItem(hWnd, IDC_PROGRESS1));

			// Update the status text
			SetDlgItemText(hWnd, IDC_STATUS, " Loading compressed data from self-extract executable file...");
			// Refesh the window
			UpdateWindow(GetDlgItem(hWnd, IDC_STATUS));


			// #ACTUAL TASK#
			// Load the user selected self-extract executable file
			hExeFile = LoadLibrary(szSelfExtractFile);
			// Check the return value
			if (NULL != hExeFile)
			{
				// Locate the resource template handle
				hResource = FindResource(hExeFile, MAKEINTRESOURCE(IDR_SETUP1), "SETUP");
				// Get the total resource size
				dwResSize = SizeofResource(hExeFile, hResource);
				// Load the resource content
				hResData = LoadResource(hExeFile, hResource);
				// Checking
				if (hResData != NULL && dwResSize != 0)
				{
					// Ensure the lpData is NULL
					lpBinData = NULL;
					// Obtain the string pointer from the loaded resource handle
					lpBinData = LockResource(hResData);

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
								  (LPBYTE)lpBinData,
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
						MessageBox(hWnd, "Fail to spawning the compressed binary data!", APP_TITLE, MB_OK | MB_ICONSTOP);
						// Jump the the "CleanExit" routine
						goto CleanExit;
					}
				}
				else
				{
					// Notify user about the error
					MessageBox(hWnd, "Fail to read the compressed binary data!", APP_TITLE, MB_OK | MB_ICONSTOP);
					// Jump the the "CleanExit" routine
					goto CleanExit;
				}

				// Free the loaded self-extract executable file
				if (NULL != hExeFile) {FreeLibrary(hExeFile);}
				hExeFile = NULL;
			}
			else
			{
				// Notify user about the error
				MessageBox(hWnd, "Fail to load the self-extract executable file!", APP_TITLE, MB_OK | MB_ICONSTOP);
				// Jump the the "CleanExit" routine
				goto CleanExit;
			}
		}


		// Check is the App terminated? Else, terminate current thread
		if (WAIT_OBJECT_0 == WaitForSingleObject(hEvent, 100)) {goto CleanExit;}

		// STAGE #2
		// Uncompress the self-extrat data
		{
			// #UPDATE GUI#
			// Set the initial position
			SendDlgItemMessage(hWnd, IDC_PROGRESS1, PBM_SETPOS, 50, 0);
			// Refresh the window
			UpdateWindow(GetDlgItem(hWnd, IDC_PROGRESS1));

			// Update the status text
			SetDlgItemText(hWnd, IDC_STATUS, " Uncompressing data file...");
			// Refesh the window
			UpdateWindow(GetDlgItem(hWnd, IDC_STATUS));
				
			
			// # ACTUAL TASK#
			// Uncompress the selected self-extract data file
			if (0 != Uncompress(szTmpBinFile1, szTmpBinFile2))
			{
				// Notify user about the error
				MessageBox(hWnd, "Fail to uncompress the self-extract file!", APP_TITLE, MB_OK | MB_ICONSTOP);
				// Jump to clean exit
				goto CleanExit;
			}
		}

		// Check is the App terminated? Else, terminate current thread
		if (WAIT_OBJECT_0 == WaitForSingleObject(hEvent, 100)) {goto CleanExit;}


		// STAGE #3
		// Read the uncompress self-extract data
		{
			// #UPDATE GUI#
			// Set the initial position
			SendDlgItemMessage(hWnd, IDC_PROGRESS1, PBM_SETPOS, 0, 0);
			// Refresh the window
			UpdateWindow(GetDlgItem(hWnd, IDC_PROGRESS1));

			// Update the status text
			SetDlgItemText(hWnd, IDC_STATUS, " Reading self-extract file properties...");
			// Refesh the window
			UpdateWindow(GetDlgItem(hWnd, IDC_STATUS));


			// #ACTUAL TASK#
			// Open the existing temporary data file
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
				// Get the current self-extract file size
				dwFileSize = GetFileSize(hFile, NULL);

				// Check the current file size
				if (0 < dwFileSize)
				{
					// Move the file pointer to the begin
					SetFilePointer(hFile, 0, 0, FILE_BEGIN);
					// Read the SETUPINFO block
					ReadFile(hFile,
							 &si,					
							 sizeof(SETUPINFO),
							 &dwByteRead,
							 NULL);

					// Check the current file count
					if (si.dwFileCount > 0)
					{
						// Allocate local memory buffer
						lpesi = (LPEXTRACTFILEINFO)LocalAlloc(LPTR, sizeof(EXTRACTFILEINFO)*si.dwFileCount); 
						// Initialize the local buffer
						ZeroMemory(lpesi, sizeof(EXTRACTFILEINFO)*si.dwFileCount);
				
						// Move the file pointer to the begin
						SetFilePointer(hFile, sizeof(SETUPINFO), 0, FILE_BEGIN);
						// Read the SETUPINFO block
						ReadFile(hFile,
								 lpesi,					
								 sizeof(EXTRACTFILEINFO)*si.dwFileCount,
								 &dwByteRead,
								 NULL);
						
						// Close the search handle
						if (NULL != hFile) {CloseHandle(hFile);}
						hFile = NULL;

						// Update the selected Auto-Exec File
						if (0 < strlen(si.szAutoExecFile))
						{
							// Update the combobox
							SendDlgItemMessage(hWnd, IDC_COMBO1, CB_INSERTSTRING, -1, (LPARAM)si.szAutoExecFile); 
							// Set default selection
							SendDlgItemMessage(hWnd, IDC_COMBO1, CB_SETCURSEL, 0, 0);
						}
						
						// Check is the App terminated? Else, terminate current thread
						if (WAIT_OBJECT_0 == WaitForSingleObject(hEvent, 100)) {goto CleanExit;}


						// Walk through the read information
						for (nIndex=0; nIndex<(int)si.dwFileCount; nIndex++)
						{
							// #UPDATE GUI#
							// Calculate the current position
							lCurPosition = (long)(((double)(nIndex+1)/si.dwFileCount)*100);
							// Update the progress bar position
							SendDlgItemMessage(hWnd, IDC_PROGRESS1, PBM_SETPOS, lCurPosition, 0);
							// Refresh the window
							UpdateWindow(GetDlgItem(hWnd, IDC_PROGRESS1));
							

							// Reset local variable
							ZeroMemory(szBuffer, sizeof(szBuffer));
							// Format the current display status
							sprintf(szBuffer,
									" Reading ...\\%s",
									lpesi[nIndex].szBinFileName); 
							// Update the status text
							SetDlgItemText(hWnd, IDC_STATUS, szBuffer);
							// Refesh the window
							UpdateWindow(GetDlgItem(hWnd, IDC_STATUS));


							// # ACTUAL TASK#
							// Reset the local variable
							ZeroMemory(&wfs, sizeof(WIN32_FIND_DATA));

							// Copy the data from EXTRACTFILEINFO INTO WIN32_FIND_DATA structure
							CopyMemory(&wfs.ftCreationTime,   &lpesi[nIndex].CreateTime,	 sizeof(FILETIME));
							CopyMemory(&wfs.ftLastAccessTime, &lpesi[nIndex].LastAcessTime,  sizeof(FILETIME));
							CopyMemory(&wfs.ftLastWriteTime,  &lpesi[nIndex].LastWriteTime,  sizeof(FILETIME));
							CopyMemory(&wfs.nFileSizeHigh,    &lpesi[nIndex].dwFileSizeHigh, sizeof(DWORD));
							CopyMemory(&wfs.nFileSizeLow,     &lpesi[nIndex].dwFileSizeLow,  sizeof(DWORD));
							CopyMemory(&wfs.cFileName,        &lpesi[nIndex].szBinFileName,  strlen(lpesi[nIndex].szBinFileName));

							// Display the information in the listview control
							RefreshListviewContent (hWnd, &wfs);

							// Check is the App terminated? Else, terminate current thread
							if (WAIT_OBJECT_0 == WaitForSingleObject(hEvent, 100)) {goto CleanExit;}
						}

						// Notify user about the process completed
						MessageBox(hWnd, "self-extract file contents successful browse.", APP_TITLE, MB_OK | MB_ICONINFORMATION);
					}
					else
						// Notify user about the error
						MessageBox(hWnd, "No self-extract information in the selected self-extract file!", APP_TITLE, MB_OK | MB_ICONSTOP);
				}
				else
					// Notify the user about the error
					MessageBox(hWnd, "No data found in the selected self-extract file!", APP_TITLE, MB_OK | MB_ICONSTOP);
			}
			else
				// Notify user about the error
				MessageBox(hWnd, "Unable to read the selected self-extract file!", APP_TITLE, MB_OK | MB_ICONSTOP);
		}

	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		// PUT YOUR ERROR HANDLING CODE HERE

		// Notify user about the error
		MessageBox(hWnd, "Problem occur while reading the self-extract file!", APP_TITLE, MB_OK | MB_ICONSTOP);
	}


CleanExit:
	// PERFORM ALL THE CLEAN UP JOB FOR THOSE ALLOCATED/USED RESOURCES

	// Close the search handle
	if (NULL != hFile) {CloseHandle(hFile);}
	hFile = NULL;

	// Free the loaded module
	if (NULL != hExeFile) {FreeLibrary(hExeFile);}
	hExeFile = NULL;

	// Free the allocate buffer
	if (NULL != lpesi) {LocalFree((LPEXTRACTFILEINFO)lpesi);}
	lpesi = NULL;

	// Delete the created temporary file
	if (0 < strlen(szTmpBinFile1)) {DeleteFile(szTmpBinFile1);}
	if (0 < strlen(szTmpBinFile2)) {DeleteFile(szTmpBinFile2);}

	// #UPDATE GUI#
	// Update the progress bar position
	SendDlgItemMessage(hWnd, IDC_PROGRESS1, PBM_SETPOS, 0, 0);
	// Refresh the window
	UpdateWindow(GetDlgItem(hWnd, IDC_PROGRESS1));
	// Update the status text
	SetDlgItemText(hWnd, IDC_STATUS, "");
	// Refesh the window
	UpdateWindow(GetDlgItem(hWnd, IDC_STATUS));

	// Clear the current listview content
	SendDlgItemMessage(hWnd,
					   IDC_LIST1,
					   LVM_DELETEALLITEMS,
					   0,
					   0);

	// Reset local variable
	ZeroMemory(szBuffer, sizeof(szBuffer));
	// Get the current source folder path
	GetDlgItemText(hWnd, IDC_EDIT1, szBuffer, sizeof(szBuffer));

	// Enable/Disable the menu
	if (0 < strlen(szBuffer)) {EnableMenuItem(hMenu, IDM_ACTION_BUILD, MF_BYCOMMAND | MF_ENABLED);}
	EnableMenuItem(hMenu, IDM_ACTION_BROWSE,	 MF_BYCOMMAND | MF_ENABLED);
	EnableMenuItem(hMenu, IDM_ACTION_STOP_BUILD, MF_BYCOMMAND | MF_GRAYED);

	// Enable control
	EnableWindow(GetDlgItem(hWnd, IDC_BUTTON1), TRUE);
	EnableWindow(GetDlgItem(hWnd, IDC_BUTTON2), TRUE);

	// Reset global variable
	hThread2 = NULL;

	// Set return value
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
void GetOutputFolder (HWND hWnd, WPARAM wParam)
{
	TCHAR			szSelFolder[MAX_PATH] = {NULL};
	BROWSEINFO		bi = {NULL};
	LPITEMIDLIST	lpIDList = NULL;

	__try
	{
		// Setup BROWSEINFO structure
		bi.hwndOwner 	= hWnd;
		bi.pidlRoot		= NULL;
		bi.lpszTitle	= TEXT("Save self-extract data file to...");
		bi.ulFlags		= BIF_RETURNONLYFSDIRS;
		bi.lpfn			= NULL;
		bi.pszDisplayName	= szSelFolder;

		// Load the Browse folder dialog
		lpIDList = SHBrowseForFolder(&bi);
		// Get the selected path from IDList
		SHGetPathFromIDList(lpIDList, szSelFolder);
		// Free the resources
		CoTaskMemFree(lpIDList);
		
		// Check the selected folder
		if (strlen(szSelFolder) > 0)
		{
			// Check is the last character is "\", if not, an extra "\" was append on it.
			if (92 != szSelFolder[strlen(szSelFolder) - 1]) {strncat(szSelFolder, "\\", 1);}

			// Display the selected folder in the edit control
			SetDlgItemText(hWnd, IDC_EDIT1, szSelFolder);
		}

		// Check require information
		IsInfoComplete (hWnd);
	}
	__except(EXCEPTION_EXECUTE_HANDLER)
	{
		// PUT YOUR ERROR HANDLE CODE HERE

	}
}


//  PURPOSE:  
//		- Get all the distributed filename from the current selected folder
//  PARAMETERS:
//		- hWnd		:: Parent window handle
//  OPERATION:
//		- ...
//  RETURN VALUE:
//      - NIL
//
void GetDistributeFileListIn(HWND hWnd,char *path){
	WIN32_FIND_DATA	wfs		= {NULL};
	char temp2[2000]={NULL};
	HANDLE	hFile	= NULL;
	sprintf(temp2,
			"%s\\*.*",
			path);

	hFile = FindFirstFile(temp2, &wfs);

	// Check the return handle value
	do
	{
		// Check is the current found file is directory?
		if (!(FILE_ATTRIBUTE_DIRECTORY & wfs.dwFileAttributes))
		{
			// Increase global variable
			dwTotalFile ++;
				
			// Insert the current filename into the combo box
			SendDlgItemMessage(hWnd, IDC_COMBO1, CB_INSERTSTRING, -1, (LPARAM)wfs.cFileName);
		}
		else if(wfs.cFileName[0]!='.'){
			char temp[2000]={NULL};
			sprintf(temp,"%s\\%s\\",path,wfs.cFileName);
			dwTotalFile ++;
			GetDistributeFileListIn(hWnd,temp);
		}
		// Scan the next match item in the directory
		if (!FindNextFile(hFile, &wfs))
		{
			if (ERROR_NO_MORE_FILES == GetLastError()) {break;}
		}

		// Check is the App terminated? Else, terminate current thread
		//if (WAIT_OBJECT_0 == WaitForSingleObject(hEvent, 100)) {goto CleanExit;}

	} while (NULL != hFile || INVALID_HANDLE_VALUE != hFile);
CleanExit:
	// NOTE:
	//		THIS ROUTINE IS TO ENSURE THE FindFile RETURN HANDLE WAS CLOSED
	//
	// Close the search handle
	if (NULL != hFile) {FindClose(hFile);}
	hFile = NULL;	
}
void GetDistributeFileList (HWND hWnd)
{
	WIN32_FIND_DATA	wfs		= {NULL};
	HANDLE	hFile	= NULL;

	char	szBuffer1[MAX_PATH] = {NULL};
	char	szBuffer2[MAX_PATH] = {NULL};

	__try
	{
		// Initialize global variable
		dwTotalFile = 0;

		// Clear all the current combobox list item
		SendDlgItemMessage(hWnd, IDC_COMBO1, CB_RESETCONTENT, 0, 0);

		// Get the current user define output folder
		GetDlgItemText(hWnd, IDC_EDIT1, szBuffer1, sizeof(szBuffer1));
		GetDistributeFileListIn(hWnd,szBuffer1);
		// Format the full path for the output filename
		/*sprintf(szBuffer2,
				"%s\\*.*",
				szBuffer1);

		// Start scaning the directory
		hFile = FindFirstFile(szBuffer2, &wfs);

		// Check the return handle value
		do
		{
			// Check is the current found file is directory?
			if (!(FILE_ATTRIBUTE_DIRECTORY & wfs.dwFileAttributes))
			{
				// Increase global variable
				dwTotalFile ++;
				
				// Insert the current filename into the combo box
				SendDlgItemMessage(hWnd, IDC_COMBO1, CB_INSERTSTRING, -1, (LPARAM)wfs.cFileName);
			}
			else{
			}
			// Scan the next match item in the directory
			if (!FindNextFile(hFile, &wfs))
			{
				if (ERROR_NO_MORE_FILES == GetLastError()) {break;}
			}

			// Check is the App terminated? Else, terminate current thread
			if (WAIT_OBJECT_0 == WaitForSingleObject(hEvent, 100)) {goto CleanExit;}

		} while (NULL != hFile || INVALID_HANDLE_VALUE != hFile);*/

		// Close the search handle
		if (NULL != hFile) {FindClose(hFile);}
		hFile = NULL;

		// Verified the current file count
		if (0 < dwTotalFile)
		{
			// Set default selection
			SendDlgItemMessage(hWnd, IDC_COMBO1, CB_SETCURSEL, 0, 0);
		}
		else
		{
			// Notify user about the error
			MessageBox(hWnd, "No file require to distribute in the current specified folder!", APP_TITLE, MB_OK | MB_ICONSTOP);
		}

	}
	__except(EXCEPTION_EXECUTE_HANDLER)
	{
		// PUT YOUR ERROR HANDLE CODE HERE

	}

CleanExit:
	// NOTE:
	//		THIS ROUTINE IS TO ENSURE THE FindFile RETURN HANDLE WAS CLOSED
	//
	// Close the search handle
	if (NULL != hFile) {FindClose(hFile);}
	hFile = NULL;	


}



//  PURPOSE:  
//		- Load the open file dialog
//  PARAMETERS:
//		- hWnd		: Parent window handle
//  OPERATION:
//		- ..
//  RETURN VALUE:
//      - NIL

void GetSelfExtractFileName (HWND hWnd, WPARAM wParam)
{
	OPENFILENAME	ofn;
	char			szFilename [MAX_PATH] = {NULL};
	char			szBuffer1  [MAX_PATH] = {NULL};
	char			szBuffer2  [MAX_PATH] = {NULL};

	__try
	{
		//Get the selected Map data file name
		memset(&ofn, TEXT('\0'), sizeof(OPENFILENAME));
		ofn.lStructSize			= sizeof (OPENFILENAME);
		ofn.hwndOwner			= hWnd;
		ofn.hInstance			= (HINSTANCE)hInst;
		ofn.lpstrCustomFilter	= (LPTSTR)NULL;
		ofn.nMaxCustFilter		= 0L;
		ofn.nFilterIndex		= 1;
		ofn.lpstrFile			= szFilename;
		ofn.nMaxFile			= MAX_PATH;
		ofn.lpstrFileTitle		= NULL;
		ofn.nMaxFileTitle		= 0;
		ofn.lpstrInitialDir		= "\\";
		ofn.lpstrTitle			= "Open Self-Extract File...";
		ofn.nFileOffset			= 0;
		ofn.nFileExtension		= 0;
		ofn.Flags				= OFN_EXPLORER;
		ofn.lpstrDefExt			= TEXT("*.exe");
		ofn.lpstrFilter			= TEXT("Self-Extract Data File (*.exe)\0 *.exe\0");

		//
		if (GetOpenFileName(&ofn))
		{
			// Display the user selected file
			SetDlgItemText(hWnd, IDC_EDIT2, szFilename);
			// Check require information
			IsInfoComplete (hWnd);
			// Enable the "Browse" menu item
			EnableMenuItem(hMenu, IDM_ACTION_BROWSE, MF_BYCOMMAND | MF_ENABLED);
		}
	}
	__except(EXCEPTION_EXECUTE_HANDLER)
	{
		// PUT YOUR ERROR HANDLE CODE HERE

	}

}


//  PURPOSE:  
//		- Check is all teh require information is supplied?
//  PARAMETERS:
//		- hWnd		: Parent window handle
//  OPERATION:
//		- ..
//  RETURN VALUE:
//      - NIL

void IsInfoComplete (HWND hWnd)
{
	BOOL	bState = FALSE;
	char	szBuffer1[MAX_PATH] = {NULL};
	char	szBuffer2[MAX_PATH] = {NULL};

	__try
	{
		// Get all the 2 editbox control text
		GetDlgItemText(hWnd, IDC_EDIT1, szBuffer1, MAX_PATH);
		GetDlgItemText(hWnd, IDC_EDIT2, szBuffer2, MAX_PATH);

		// Check the return string length & set the return state
		bState = BOOL(strlen(szBuffer1)) && 
				 BOOL(strlen(szBuffer2));

		// Check the current information & Enable/Disable the "Build" menu item
		if (TRUE == bState)
			EnableMenuItem(hMenu, IDM_ACTION_BUILD, MF_BYCOMMAND | MF_ENABLED);
		else
			EnableMenuItem(hMenu, IDM_ACTION_BUILD, MF_BYCOMMAND | MF_GRAYED);

		
	}
	__except(EXCEPTION_EXECUTE_HANDLER)
	{
		// PUT YOUR ERROR HANDLE CODE HERE

	}
}


//  PURPOSE:  
//		- Write the file information into the setup list
//  PARAMETERS:
//		- hWnd				:: Parent window handle
//		- lpFindFileData	:: Pointer point to WIND32_FIND_DATA structure
//  OPERATION:
//		- Call "FindFirstFile", "FindNextFile", "FindClose" API
//  RETURN VALUE:
//      - NIL
//
void WriteSelfExtractHeader (HWND hWnd, LPWIN32_FIND_DATA lpFindFileData, char* prefix, bool isDirectory)
{
	EXTRACTFILEINFO	efi = {NULL};
	
	HANDLE	hFile		= NULL;
	DWORD	dwByteWrite = 0;

	__try
	{
		// Open the existing temporary data file
		hFile = CreateFile(szTmpBinFile1,
						   GENERIC_WRITE,
						   FILE_SHARE_WRITE,
						   NULL,
						   OPEN_EXISTING,
						   FILE_ATTRIBUTE_NORMAL,
						   NULL);

		// Check the return handle value
		if (NULL == hFile || INVALID_HANDLE_VALUE == hFile)
		{
			// Open the existing temporary data file
			hFile = CreateFile(szTmpBinFile1,
							   GENERIC_WRITE,
							   FILE_SHARE_WRITE,
							   NULL,
							   CREATE_NEW,
							   FILE_ATTRIBUTE_NORMAL,
							   NULL);			
		}

		// Check the return handle value
		if (NULL != hFile && INVALID_HANDLE_VALUE != hFile)
		{
			// Reset the local EXTRACTFILEINFO structure
			ZeroMemory(&efi, sizeof(EXTRACTFILEINFO));

			// Initialize the EXTRACTFILEINFO structure
			efi.dwIndex = dwFileCount;
			CopyMemory(&efi.CreateTime,	    &lpFindFileData->ftCreationTime,   sizeof(FILETIME));
			CopyMemory(&efi.LastAcessTime,  &lpFindFileData->ftLastAccessTime, sizeof(FILETIME));
			CopyMemory(&efi.LastWriteTime,  &lpFindFileData->ftLastWriteTime,  sizeof(FILETIME));
			CopyMemory(&efi.dwFileSizeHigh, &lpFindFileData->nFileSizeHigh ,   sizeof(DWORD));
			CopyMemory(&efi.dwFileSizeLow,  &lpFindFileData->nFileSizeLow,     sizeof(DWORD));
			CopyMemory(&efi.szBinFileName,  &lpFindFileData->cFileName,        strlen(lpFindFileData->cFileName));
			CopyMemory(&efi.prefix,  prefix, strlen(prefix));
			efi.isDirectory=isDirectory;

			// Check current file count & move the file pointer
			if (0 == dwFileCount)
				SetFilePointer(hFile, sizeof(SETUPINFO), 0, FILE_BEGIN);
			else
				SetFilePointer(hFile, 0, 0, FILE_END);

			// Write the data into setup list file
			WriteFile(hFile,
					  &efi,
					  sizeof(EXTRACTFILEINFO),
					  &dwByteWrite,
					  NULL);

			// Increate the counter
			dwFileCount++;
		}
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		// PUT YOUR ERROR HANDLING CODE HERE

	}

	// Close the open file handle
	if (NULL != hFile) {CloseHandle(hFile);}
	hFile = NULL;

}


//  PURPOSE:  
//		- Write the file information into the temporary setup data file
//  PARAMETERS:
//		- hWnd				:: Parent window handle
//		- lpFindFileData	:: Pointer point to WIND32_FIND_DATA structure
//  OPERATION:
//		- ...
//  RETURN VALUE:
//      - NIL
//
void WriteSelfExtractBinData (HWND hWnd, LPWIN32_FIND_DATA lpFindFileData, char* prefix)
{
	HANDLE	hFile		= NULL;
	LPBYTE	lpData		= NULL;
	DWORD	dwSize		= 0;
	DWORD	dwByteRead	= 0;
	DWORD	dwByteWrite = 0;

	char	szBuffer1[MAX_PATH] = {NULL};
	char	szBuffer2[MAX_PATH] = {NULL};

	__try
	{
		// Get the user define source folder
		GetDlgItemText(hWnd, IDC_EDIT1, szBuffer1, sizeof(szBuffer1));
		// Format the full file path
		sprintf(szBuffer2, 
				"%s%s\\%s",
				szBuffer1, prefix, lpFindFileData->cFileName); 
		
		// STAGE #1
		// Read the current binary file data
		hFile = CreateFile(szBuffer2,
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
			dwSize = (lpFindFileData->nFileSizeHigh*(MAXDWORD+1)) + 
					  lpFindFileData->nFileSizeLow;
			// Allocate local data buffer
			lpData = (LPBYTE)LocalAlloc(LPTR, dwSize);
			// Reset local data buffer
			ZeroMemory(lpData, dwSize);
			
			// Move the file pointer to the begining
			SetFilePointer(hFile, 0, 0, FILE_BEGIN);
			// Read the binary data
			ReadFile(hFile, lpData, dwSize, &dwByteRead, NULL);
		}

		// Close the open file handle
		if (NULL != hFile) {CloseHandle(hFile);}
		hFile = NULL;


		// STAGE #2
		//		WRITE THE READ BINDARY DATA INTO THE TEMPORARY FILE
		// Open the existing setup data file (szTmpBinFile2)
		hFile = CreateFile(szTmpBinFile2,
						   GENERIC_WRITE,
						   FILE_SHARE_WRITE,
						   NULL,
						   OPEN_EXISTING,
						   FILE_ATTRIBUTE_NORMAL,
						   NULL);

		// Check the return handle value
		if (NULL == hFile || INVALID_HANDLE_VALUE == hFile)
		{
			// Open the existing setup.lst data file
			hFile = CreateFile(szTmpBinFile2,
							   GENERIC_WRITE,
							   FILE_SHARE_WRITE,
							   NULL,
							   CREATE_NEW,
							   FILE_ATTRIBUTE_NORMAL,
							   NULL);			
		}

		// Check the return handle value
		if (NULL != hFile && INVALID_HANDLE_VALUE != hFile)
		{
			// Move the file pointer
			SetFilePointer(hFile, 0, 0, FILE_END);

			// Write the data into setup list file
			WriteFile(hFile,
					  lpData,
					  dwSize,
					  &dwByteWrite,
					  NULL);
		}
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		// PUT YOUR ERROR HANDLING CODE HERE

	}

	// Release the allocated data buffer
	if (NULL != lpData){LocalFree((LPBYTE)lpData);}
	lpData = NULL;

	// Close the open file handle
	if (NULL != hFile) {CloseHandle(hFile);}
	hFile = NULL;

}


//  PURPOSE:  
//		- Combine the FileCount (First 4 Bytes)
//		  and append the actual binary data at the end of
//		  the setup data file
//  PARAMETERS:
//		- hWnd	:: Parent window handle
//  OPERATION:
//		- ...
//  RETURN VALUE:
//      - BOOL
//			TRUE  : SUCCESSFUL
//			FALSE : FAIL TO MERGE INFORMATION
//
BOOL MergeSelfExtractData (HWND hWnd)
{
	SETUPINFO	si		= {NULL};

	BOOL	bResult		= FALSE;
	
	LPBYTE	lpData		= NULL;
	HANDLE	hFile		= NULL;
	DWORD	dwSize		= 0;
	DWORD	dwByteWrite = 0;
	DWORD	dwByteRead	= 0;
	DWORD	dwIndex		= 0;

	__try
	{
		// STAGE #1
		{
			// Read the current binary file data
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
				dwSize = GetFileSize(hFile, 0);
				// Allocate local data buffer
				lpData = (LPBYTE)LocalAlloc(LPTR, dwSize);
				// Reset local data buffer
				ZeroMemory(lpData, dwSize);
				
				// Move the file pointer to the begining
				SetFilePointer(hFile, 0, 0, FILE_BEGIN);
				// Read the binary data
				ReadFile(hFile, lpData, dwSize, &dwByteRead, NULL);
			}

			// Close the open file handle
			if (NULL != hFile) {CloseHandle(hFile);}
			hFile = NULL;
		}


		// STAGE #2
		// Open the existing temporary data file
		hFile = CreateFile(szTmpBinFile1,
						   GENERIC_WRITE,
						   FILE_SHARE_WRITE,
						   NULL,
						   OPEN_EXISTING,
						   FILE_ATTRIBUTE_NORMAL,
						   NULL);

		// Check the return handle value
		if (NULL != hFile && INVALID_HANDLE_VALUE != hFile)
		{
			// Reset local variable
			ZeroMemory(&si, sizeof(SETUPINFO));
			// Setup the SETUPINFO structure
			si.dwFileCount = dwFileCount;
			// Get the current selected Auto-Exec filename
			dwIndex = SendDlgItemMessage(hWnd, IDC_COMBO1, CB_GETCURSEL, 0, 0);
			// Check the return value
			if (CB_ERR != dwIndex && CB_ERRSPACE != dwIndex)
			{
				// Get the current selected Auto-Exec filename
				SendDlgItemMessage(hWnd, IDC_COMBO1, CB_GETLBTEXT, dwIndex, (LPARAM)si.szAutoExecFile);
			}
			
			// Move the file pointer
			SetFilePointer(hFile, 0, 0, FILE_BEGIN);
			// Write the total binary data file being included
			WriteFile(hFile,
					  &si,
					  sizeof(SETUPINFO),
					  &dwByteWrite,
					  NULL);

			// Move the file pointer
			SetFilePointer(hFile, 0, 0, FILE_END);
			// Append the actual binary data from the temp file
			WriteFile(hFile,
					  lpData,
					  dwSize,
					  &dwByteWrite,
					  NULL);

			// Set return value
			bResult = TRUE;
		}
		else
			// Set return value
			bResult = FALSE;


	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		// PUT YOUR ERROR HANDLING CODE HERE

		// Set default return value
		bResult = FALSE;
	}

	// Release the allocated data buffer
	if (NULL != lpData){LocalFree((LPBYTE)lpData);}
	lpData = NULL;

	// Close the open file handle
	if (NULL != hFile) {CloseHandle(hFile);}
	hFile = NULL;
	
	// Return local result
	return bResult;
}



//  PURPOSE:  
//		- Update the current data into the listview control (for display purpose)
//  PARAMETERS:
//		- hWnd				:: Parent window handle
//		- lpFindFileData	:: Pointer point to WIND32_FIND_DATA structure
//  OPERATION:
//		- ...
//  RETURN VALUE:
//      - NIL
//
void RefreshListviewContent (HWND hWnd, LPWIN32_FIND_DATA lpFindFileData)
{
	long	lItem	  = 0;
	HWND	hWndList  = NULL;

	char	szBuffer[32]	= {NULL};
	char	szDateValue[32] = {NULL};
	char	szTimeValue[32] = {NULL};
	char	szTimeStamp[64] = {NULL};

	SYSTEMTIME	st		= {NULL};
	LVITEM		item	= {NULL};

	__try
	{
		// Get the current listview handle
		hWndList = GetDlgItem(hWnd, IDC_LIST1);

		// Item #1
		{
			// Reset local variable
			ZeroMemory(szBuffer, sizeof(szBuffer));
			// Convert the counter from int to string
			sprintf(szBuffer,
					"%d",
					SendMessage(hWndList, LVM_GETITEMCOUNT, 0, 0));

			// Reset local variable
			memset(&item, NULL, sizeof(LVITEM));
			// Setup the LVITEM structure
			item.mask		= LVIF_TEXT;
			item.iItem		= SendMessage(hWndList, LVM_GETITEMCOUNT, 0, 0);
			item.iSubItem	= 0;
			item.pszText	= szBuffer;
			// Post the information to the listview control
			lItem = SendMessage(hWndList, LVM_INSERTITEM, 0, (LPARAM)&item);
		}

		// Item #2
		{
			// Reset local variable
			memset(&item, NULL, sizeof(LVITEM));
			// Setup the LVITEM structure
			item.mask		= LVIF_TEXT;
			item.iItem		= lItem;
			item.iSubItem	= 1;
			item.pszText	= lpFindFileData->cFileName;
			// Post the information to the listview control
			SendMessage(hWndList, LVM_SETITEM, 0, (LPARAM)&item);
		}

		// Item #3
		{
			// Reset local variable
			ZeroMemory(szBuffer, sizeof(szBuffer));
			// Convert the counter from int to string
			sprintf(szBuffer,
					"%u",
					(lpFindFileData->nFileSizeHigh*(MAXDWORD+1)) + lpFindFileData->nFileSizeLow);

			// Reset local variable
			memset(&item, NULL, sizeof(LVITEM));
			// Setup the LVITEM structure
			item.mask		= LVIF_TEXT;
			item.iItem		= lItem;
			item.iSubItem	= 2;
			item.pszText	= szBuffer;
			// Post the information to the listview control
			SendMessage(hWndList, LVM_SETITEM, 0, (LPARAM)&item);
		}

		// Item #4
		{
			// Reset local variable
			ZeroMemory(&st, sizeof(SYSTEMTIME));
			// Convert the CreateTime from FILETIME TO SYSTEMTIME
			FileTimeToSystemTime(&lpFindFileData->ftCreationTime, &st); 
			// Format the CreateTime value
			GetDateFormat(LOCALE_SYSTEM_DEFAULT,
						  0,
						  &st,
						  "dd/MMM/yyyy",
						  szDateValue,
						  sizeof(szDateValue));

			GetTimeFormat(LOCALE_SYSTEM_DEFAULT,
						  TIME_FORCE24HOURFORMAT,
						  &st,
						  "HH:mm:ss",
						  szTimeValue,
						  sizeof(szTimeValue));
			// Combine the Date/Time Value
			sprintf(szTimeStamp,
					"%s %s",
					szDateValue, szTimeValue);

			// Reset local variable
			memset(&item, NULL, sizeof(LVITEM));
			// Setup the LVITEM structure
			item.mask		= LVIF_TEXT;
			item.iItem		= lItem;
			item.iSubItem	= 3;
			item.pszText	= szTimeStamp;
			// Post the information to the listview control
			SendMessage(hWndList, LVM_SETITEM, 0, (LPARAM)&item);
		}

		// Item #5
		{
			// Reset local variable
			ZeroMemory(&st, sizeof(SYSTEMTIME));
			// Convert the CreateTime from FILETIME TO SYSTEMTIME
			FileTimeToSystemTime(&lpFindFileData->ftLastAccessTime, &st); 
			// Format the CreateTime value
			GetDateFormat(LOCALE_SYSTEM_DEFAULT,
						  0,
						  &st,
						  "dd/MMM/yyyy",
						  szDateValue,
						  sizeof(szDateValue));

			GetTimeFormat(LOCALE_SYSTEM_DEFAULT,
						  TIME_FORCE24HOURFORMAT,
						  &st,
						  "HH:mm:ss",
						  szTimeValue,
						  sizeof(szTimeValue));
			// Combine the Date/Time Value
			sprintf(szTimeStamp,
					"%s %s",
					szDateValue, szTimeValue);

			// Reset local variable
			memset(&item, NULL, sizeof(LVITEM));
			// Setup the LVITEM structure
			item.mask		= LVIF_TEXT;
			item.iItem		= lItem;
			item.iSubItem	= 4;
			item.pszText	= szTimeStamp;
			// Post the information to the listview control
			SendMessage(hWndList, LVM_SETITEM, 0, (LPARAM)&item);
		}

		// Item #6
		{
			// Reset local variable
			ZeroMemory(&st, sizeof(SYSTEMTIME));
			// Convert the CreateTime from FILETIME TO SYSTEMTIME
			FileTimeToSystemTime(&lpFindFileData->ftLastWriteTime, &st); 
			// Format the CreateTime value
			GetDateFormat(LOCALE_SYSTEM_DEFAULT,
						  0,
						  &st,
						  "dd/MMM/yyyy",
						  szDateValue,
						  sizeof(szDateValue));

			GetTimeFormat(LOCALE_SYSTEM_DEFAULT,
						  TIME_FORCE24HOURFORMAT,
						  &st,
						  "HH:mm:ss",
						  szTimeValue,
						  sizeof(szTimeValue));
			// Combine the Date/Time Value
			sprintf(szTimeStamp,
					"%s %s",
					szDateValue, szTimeValue);

			// Reset local variable
			memset(&item, NULL, sizeof(LVITEM));
			// Setup the LVITEM structure
			item.mask		= LVIF_TEXT;
			item.iItem		= lItem;
			item.iSubItem	= 5;
			item.pszText	= szTimeStamp;
			// Post the information to the listview control
			SendMessage(hWndList, LVM_SETITEM, 0, (LPARAM)&item);
		}

		// Refresh the listview
		UpdateWindow(GetDlgItem(hWnd, IDC_LIST1));

	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		// PUT YOUR ERROR HANDLING CODE HERE

	}

}