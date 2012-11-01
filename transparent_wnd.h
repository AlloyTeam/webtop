#ifndef _TRANSPARENT_WND_H
#define _TRANSPARENT_WND_H

#include <stdio.h>
#include <string>
#include "cefclient/cefclient.h"
#include "my_handler.h"
#include "cefclient/resource.h"
#include <shellapi.h>
#include <fstream>
#include "SFMMem.h"
#include "AmfStream.h"
#include "P2P.h"

using namespace std;

void InitCallback();

class TransparentWnd
{
private:
	CefString url;
	HINSTANCE hinst;
public:
	CefRefPtr<MyHandler> g_handler;
	HWND hWnd;
	HWND renderWindow;
	int x;
	int y;
	int dragX;
	int dragY;
	int width;
	int height;
	POINT startPoint;
	bool isDrag;
	UINT renderTimer;
	bool isEnableDrag;
	BYTE *m_buffer;
	bool isTransparent;
	TransparentWnd(void);
	~TransparentWnd(void);
	void Move(int x, int y);
	void SetSize(int w, int h);
	void SetUrl(CefString url);
	CefString GetUrl();
	void Render(const void* buffer=NULL);
	void SetTopMost();
	void SetToolWindow();
	void SetWindowStyle(UINT dwNewExStyle);
	void SetHinst(HINSTANCE hinst);
	void CreateBrowserWindow(CefString url, UINT ex_style=0, bool isTransparent=false);
	void EnableTransparent(UINT ex_style=0);
	void CreateBrowser(CefString url, CefString param="");
	void Browse(CefString& url);
	HWND GetHwnd();
	HWND GetRenderHwnd();
	void RunApp(CefString appName, CefString param="", CefString baseUrl="");
	void RunAppIn(CefString appName, CefString param="", CefString baseUrl="");
	void MoveHandler(int x, int y);
	void GetSize(int& w, int& h);
	void SizeHandler();
	void CloseHandler();
	void TaskMouseHandler(UINT type);
	void FocusHandler();
	void DropHandler(HDROP hDrop);
	void ActiveHandler();
	void BringToTop();
	void Drag();
	void EnableDrag();
	void StartDrag();
	void SetAsChild(TransparentWnd *parent);
	void ReloadIgnoreCache();
	void Reload();
	void Focus();
	void Max();
	void Mini();
	void Hide();
	void Restore();
	void LeaveHandler();
	bool enableResize;
	CefString GetSaveName(CefString fileName);
	CefString GetOpenName(CefString fileName);
	CefString GetOpenNames(CefString fileName);
	void ExecJS(CefString s);
	void Ready();
	HICON GetIcon(CefString path);
	void SetReadyHandler(CefString s);
	void SetTitle(CefString title);
	CefString TransparentWnd::ReadFile(CefString path);
	bool TransparentWnd::WriteFile(CefString path, CefString s);
	void CreateBrowserWindowBase(CefString path, UINT ex_style=0, bool isTransparent=true);
	CefString readyHandler;
	CefRefPtr<CefDownloadHandler> downloadHandler;
	ATOM RegisterClass(HINSTANCE hInstance);
	ATOM RegisterTransparentClass(HINSTANCE hInstance);
	static LRESULT CALLBACK TransparentWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
	static LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
	static VOID   CALLBACK   OnTimerProc(HWND   hwnd,UINT   uMsg,UINT_PTR   idEvent,DWORD   dwTime) ;
	static int count;
	void ToImage(CefString path);
	void ToImageEx(CefString path, int x, int y, int width, int height);
	void SetTaskIcon(int id, CefString iconPath, CefString title);
	void DelTaskIcon(int id);
	CefString TranslatePath(CefString path);
	CSFMServer* CreateMemory(CefString name, CefString fileName, int size);
	void DeleteMemory(CSFMServer* mem);
	AmfStream* CreateStream(CSFMServer* mem);
	void SaveImageFromStream(CefString path,AmfStream* pStream,int width,int height);
	void DeleteStream(AmfStream* pStream);
	CefString GetFolder();
	HICON hIcon;
	CSFMClient* pClient;
	AmfStream* pStream;
	HBITMAP hBitMap;
	bool enableRefresh;
	bool enableDevelop;
	bool isMini;
	bool isHide;
	CefString GetCurrentDirectory();
	void RecieveMessage(int type, char* message, char* ip, unsigned short port);
	void ShowTip(CefString& text);
	void *pTipWin;
	static CSFMServer* pSever;
	P2P p2p;
	CefString folder;
	int restoreX;
	int restoreY;
	void GetUsers();
};
#endif
