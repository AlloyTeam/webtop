#ifndef __SYSTEM_H__
#define __SYSTEM_H__
#include <Windows.h>
#include <xstring>
#include <CommDlg.h>
#include <comdef.h>
#include "myutil.h"
#include <GdiPlus.h>
using namespace Gdiplus;
using namespace std;
wstring GetExt(wstring path);
BOOL EnableShutdownPrivilege();
BOOL Shutdown(BOOL bForce);
BOOL Logoff(BOOL bForce);
BOOL Reboot(BOOL bForce);
BOOL  SaveFileDialog(HWND hWnd, const TCHAR* fileNameStr, TCHAR* szFile);
BOOL  OpenFileDialog(HWND hWnd, const TCHAR* fileNameStr, TCHAR* szFile);
int CDECL MessageBoxPrintf (TCHAR * szCaption, TCHAR * szFormat, ...);

void SetDefaultBrowser(const TCHAR *strAppName);
BOOL CheckFileRelation(const TCHAR *strExt, const TCHAR *strAppKey, TCHAR *strAppName, TCHAR *strDefaultIcon, TCHAR *strDescribe);
void RegisterFileRelation(TCHAR *strExt, TCHAR *strAppName, TCHAR *strAppKey, TCHAR *strDefaultIcon, TCHAR *strDescribe);
wstring find(wstring path,bool cursive=false);
Bitmap* GetImageFromBase64(string encodedImage);
int   GetEncoderClsid(const   WCHAR*   format,   CLSID*   pClsid);
void SaveBitmap(Bitmap* pbm, wstring path);

#endif