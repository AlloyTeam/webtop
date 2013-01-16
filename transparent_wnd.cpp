#pragma comment ( lib, "imm32.lib" )
#include "transparent_wnd.h"
#include <sstream>
#include "cefclient/string_util.h"
#include <tchar.h>
#pragma comment( lib, "gdiplus.lib" )
#include <comdef.h>
#include <GdiPlus.h>
#include "base64.h"
#include "system.h"
using namespace Gdiplus;
#define WM_INIT WM_USER+2
#define ODS(msg) MessageBox(NULL, msg, msg, 0);
#define WM_NOTIFYICON	WM_USER+5
#define IDI_ICON		0x0005
//#ifndef ULONG_PTR
//#define ULONG_PTR unsigned long*
//#endif
int TransparentWnd::count = 0;
CSFMServer* pServer=new CSFMServer(NULL, L"Webtop", 1);
extern HINSTANCE hInst;
ULONG_PTR m_gdiplusToken;
extern wstring modulePath;
GdiplusStartupInput gdiplusStartupInput;
void TransparentWnd::CreateBrowserWindow(CefString url, UINT ex_style, bool isTransparent){
	RegisterClass(hInst);
	this->url=url;
	this->isTransparent=isTransparent;
	UINT ex_style1=ex_style;
	if(isTransparent){
		ex_style1=WS_EX_TOOLWINDOW;
	}
	hWnd = CreateWindowEx(ex_style1, L"browser", L"透明浏览器",
			WS_OVERLAPPED&~(WS_CAPTION|WS_BORDER), 0, 0, 0,
			0, NULL, NULL, hInst, NULL);
	g_handler=new MyHandler();
	g_handler->win=(long)this;
	InitCallback();
	SetWindowLong(hWnd, GWL_USERDATA, (LONG)this);
	if(isTransparent){
		EnableTransparent(ex_style);
	}
	else{
		SendMessage(hWnd, WM_INIT, NULL, NULL);
		DWORD dwStyle = GetWindowLong(hWnd, GWL_STYLE);//获取旧样式  
		DWORD dwNewStyle = WS_OVERLAPPED | WS_VISIBLE| WS_SYSMENU |WS_MINIMIZEBOX|WS_MAXIMIZEBOX|WS_CLIPCHILDREN|WS_CLIPSIBLINGS;  
		dwNewStyle&=dwStyle;//按位与将旧样式去掉  
		SetWindowLong(hWnd,GWL_STYLE,dwNewStyle);//设置成新的样式  
		DWORD dwExStyle = GetWindowLong(hWnd, GWL_EXSTYLE);//获取旧样式  
		DWORD dwNewExStyle = WS_EX_TOOLWINDOW|WS_EX_LEFT |WS_EX_LTRREADING |WS_EX_RIGHTSCROLLBAR;  
		dwNewExStyle&=dwExStyle;//按位与将旧扩展样式去掉  
		SetWindowLong(hWnd,GWL_EXSTYLE,dwNewExStyle);//设置新的扩展样式  
		ShowWindow(hWnd, SW_SHOW);
		UpdateWindow(hWnd);
	}
}
CefString TransparentWnd::GetSaveName(CefString fileName){
	TCHAR szFile[2048];
	SaveFileDialog(hWnd, fileName.ToWString().data(), szFile);
	wstring d(szFile);
	CefString s(d);
	return s;
}
CefString TransparentWnd::GetOpenNames(CefString fileName){
	TCHAR szFiles[4096];
	OpenMultiFilesDialog(hWnd, fileName.ToWString().data(), szFiles);
    // Multi-Select
	std::wstringstream ss;
	ss<<"[\"";
    wchar_t* p = szFiles;
	ss<<p<<"\"";
    p += lstrlen(p) + 1;
    while(*p)
    {
		wstring s(p);
		ss<<","<<"\""<<s<<"\"";
		p += lstrlen(p) + 1;
        // "p" - name of each files
    }
	ss<<"]";
	return ss.str();
}
CefString TransparentWnd::GetOpenName(CefString fileName){
	TCHAR szFile[4096];
	OpenFileDialog(hWnd, fileName.ToWString().data(), szFile);
	wstring d(szFile);
	CefString s(d);
	return s;
}
CefString TransparentWnd::GetFolder(){
	TCHAR szFolder[4096];
	::GetFolder(hWnd,szFolder);
	wstring d(szFolder);
	CefString s(d);
	return s;
}
void TransparentWnd::EnableTransparent(UINT ex_style){
	isTransparent=true;
	SendMessage(hWnd, WM_INIT, NULL, NULL);
	RegisterTransparentClass(hInst);
	renderWindow=CreateWindowEx(WS_EX_LAYERED|ex_style, L"transparent", L"透明浏览器",
		WS_POPUP&~(WS_CAPTION|WS_BORDER), 0, 0, 0,
		0, NULL, NULL, hInst, NULL);
	SetWindowLong(renderWindow, GWL_USERDATA, (LONG)this);
	SendMessage(renderWindow, WM_INIT, NULL, NULL);
	//MoveWindow(renderWindow, 0, 0, 1000, 650, false);
	//MoveWindow(hWnd, -1000, -1000, 1000, 650, false);
	ShowWindow(renderWindow, SW_SHOW);
	UpdateWindow(renderWindow);
	if(g_handler->GetBrowser()){
		g_handler->GetBrowser()->GetMainFrame()->LoadURL(url);
	}
	ModifyStyle(renderWindow,0,WS_MINIMIZEBOX,0);
}
void TransparentWnd::CreateBrowserWindowBase(CefString path, UINT ex_style, bool isTransparent){
	TCHAR   szPath[1000];   
	GetModuleFileName(NULL,szPath,MAX_PATH);
	wstring _path(szPath);
	_path=_path.substr(0,_path.find_last_of('\\')+1);
	replace_allW(_path, L"\\", L"/");
	CefString pathC(_path);
	std::stringstream ss;
	ss<<pathC.ToString().c_str()<<path.ToString().c_str();
	CefString s(ss.str());
	CreateBrowserWindow(s, ex_style, isTransparent);
}
ATOM TransparentWnd::RegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= TransparentWnd::WndProc;
	wcex.cbClsExtra		= sizeof(HANDLE);
	wcex.cbWndExtra		= TRUE;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= hIcon;//LoadIcon(hInstance, MAKEINTRESOURCE(IDI_CEFCLIENT));
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName	= NULL;
	wcex.lpszClassName	= L"browser";
	wcex.hIconSm		= hIcon;//LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	return RegisterClassEx(&wcex);
}
void TransparentWnd::RunApp(CefString appName, CefString param, CefString baseUrl){
	wstring appNameW=appName.ToWString();
	wstring path;
	if(!baseUrl.length()){
		path=modulePath;
	}
	else{
		path=baseUrl.ToWString();
		replace_allW(path, L"\\", L"/");
		path=path.substr(0,path.find_last_of('/')+1);
	}
	if(appNameW.find(L":")==-1){
		appNameW=path.append(appNameW);
	}
	CreateBrowser(appNameW,param);
}
CefString TransparentWnd::GetCurrentDirectory(){
	return folder;
}
void TransparentWnd::RunAppIn(CefString appName, CefString param, CefString baseUrl){
	wstring appNameW=appName.ToWString();
	if(appNameW.find(L":")==-1){
		wstring path;
		if(!baseUrl.length()){
			path=modulePath;//szPath;
		}
		else{
			path=baseUrl.ToWString();
			replace_allW(path, L"\\", L"/");
			path=path.substr(0,path.find_last_of('/')+1);
		}
		appNameW=path.append(appNameW);
	}
	replace_allW(appNameW, L"\\", L"/");
	int w,h,_x,_y;
	int enableDrag=0,disableTransparent=0,exStyle=0,hasBorder=false,_max=false,_enableResize=false,disableRefresh=0,disableDevelop=0;
	TCHAR url[1000],name[100],iconPath[1000];
	wstring _folder=appNameW.substr(0,appNameW.find_last_of('/')+1);
	folder=_folder;
	GetPrivateProfileString(L"BASE",L"url",NULL,url,1000,appNameW.data());
	GetPrivateProfileString(L"BASE",L"name",NULL,name,100,appNameW.data());
	GetPrivateProfileString(L"BASE",L"icon",NULL,iconPath,1000,appNameW.data());
	w=GetPrivateProfileInt(L"BASE",L"width",0,appNameW.data());
	h=GetPrivateProfileInt(L"BASE",L"height",0,appNameW.data());
	_x=GetPrivateProfileInt(L"BASE",L"x",0,appNameW.data());
	_y=GetPrivateProfileInt(L"BASE",L"y",0,appNameW.data());
	enableDrag=GetPrivateProfileInt(L"BASE",L"enableDrag",0,appNameW.data());
	disableRefresh=GetPrivateProfileInt(L"BASE",L"disableRefresh",0,appNameW.data());
	disableDevelop=GetPrivateProfileInt(L"BASE",L"disableDevelop",0,appNameW.data());
	_enableResize=GetPrivateProfileInt(L"BASE",L"enableResize",0,appNameW.data());
	disableTransparent=GetPrivateProfileInt(L"BASE",L"disableTransparent",0,appNameW.data());
	hasBorder=GetPrivateProfileInt(L"BASE",L"hasBorder",0,appNameW.data());
	_max=GetPrivateProfileInt(L"BASE",L"max",0,appNameW.data());
	exStyle=GetPrivateProfileInt(L"BASE",L"exStyle",0,appNameW.data());
	int l=wcslen(iconPath);
	if(l>0){
		hIcon=GetIcon(iconPath);
	}
	if(_enableResize>0){
		enableResize=true;
	}
	enableRefresh=disableRefresh==0;
	enableDevelop=disableDevelop==0;
	CefString cefFile(url);
	wstring file=cefFile.ToWString();
	if(file.find(L":")==-1){
		file=_folder.append(file);
	}
	bool isTransparent=disableTransparent==0;
	if(hasBorder){
		char t[10];
		_itoa(isTransparent,t,10);
		CefString ct(t);
		wstring _name(name);
		file=modulePath.append(L"window\\index.html?name=").append(_name).append(L"&url=").append(file).append(L"&transparent=").append(ct.ToWString().data()).append(L"&x=");
		_itoa(_x,t,10);
		ct=t;
		file.append(ct.ToWString().data());
		file.append(L"&y=");
		_itoa(_y,t,10);
		ct=t;
		file.append(ct.ToWString().data());
		file.append(L"&width=");
		_itoa(w,t,10);
		ct=t;
		file.append(ct.ToWString().data());
		file.append(L"&height=");
		_itoa(h,t,10);
		ct=t;
		file.append(ct.ToWString().data());
		file.append(L"&max=");
		_itoa(_max,t,10);
		ct=t;
		file.append(ct.ToWString().data());
		isTransparent=true;
		if(param.length()){
			file.append(L"&param="+param.ToWString());
		}
	}
	else {
		if(param.length()){
			file.append(L"?param="+param.ToWString());
		}
	}
	CreateBrowserWindow(file, exStyle, isTransparent);
	if(_max){
		this->Max();
	}
	else{
		SetSize(w,h);
		Move(_x,_y);
	}
	this->SetTitle(name);
	if(enableDrag>0){
		EnableDrag();
	}

}
ATOM TransparentWnd::RegisterTransparentClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= TransparentWnd::TransparentWndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= sizeof(HANDLE);
	wcex.hInstance		= hInstance;
	wcex.hIcon			= hIcon;
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName	= NULL;
	wcex.lpszClassName	= L"transparent";
	wcex.hIconSm		= hIcon;//LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	return RegisterClassEx(&wcex);
}

LRESULT CALLBACK TransparentWnd::TransparentWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	CefRefPtr<CefBrowser> browser = NULL;
	HWND browserhWnd=NULL;
	TransparentWnd* handler=NULL;
	static int top=0,left=0;
	static HWND editHWnd;
	static BOOL _bMouseTrack=TRUE;
	extern HINSTANCE hInst;
	switch (message)
	{
	case WM_CREATE:
		DragAcceptFiles(hWnd,TRUE);
		break;
	case WM_DROPFILES:
		{
			handler=(TransparentWnd*)GetWindowLong(hWnd, GWL_USERDATA);
			CefRefPtr<ClientHandler> g_handler=handler->g_handler;
			HWND browserHwnd = g_handler->GetBrowserHwnd();
			//当文件拖进来时
			HDROP hDrop=HDROP(wParam);
			handler->DropHandler(hDrop);
			SendMessage(handler->hWnd, message, wParam, lParam);
			SendMessage(browserHwnd, message, wParam, lParam);
			DragFinish(hDrop);
		}
		break;
	case WM_INIT:
		{
			handler=(TransparentWnd*)GetWindowLong(hWnd, GWL_USERDATA);
			CefRefPtr<ClientHandler> g_handler=handler->g_handler;
			browser = g_handler->GetBrowser();
			left=GetSystemMetrics(SM_CXFRAME);
			top=GetSystemMetrics(SM_CYFRAME)+GetSystemMetrics(SM_CYCAPTION)-2;
			DragAcceptFiles(g_handler->GetBrowserHwnd(),TRUE);
		}
		break;
	case WM_NCHITTEST:
		{
			handler=(TransparentWnd*)GetWindowLong(hWnd, GWL_USERDATA);
			if(!handler->enableResize){
				if(handler->isTransparent){
					return HTCLIENT;
				}
				else{
					return HTCAPTION;
				}
			}
			else{
				int x,y,width=handler->width,height=handler->height;
				x=LOWORD(lParam)-handler->x;
				y=HIWORD(lParam)-handler->y;
				TCHAR w[100];
				wsprintf(w,L"%d,%d",x,y);
				if(x<10&&y<10){
					return HTTOPLEFT;
				}
				else if(x+10>width&&y+10>height){
					return HTBOTTOMRIGHT;
				}
				else if(x+10>width&&y<10){
					return HTTOPRIGHT;
				}
				else if(x<10&&y+10>height){
					return HTBOTTOMLEFT;
				}
				else if(x<5){
					return HTLEFT;
				}
				else if(y<5){
					return HTTOP;
				}
				else if(x+5>width){
					return HTRIGHT;
				}
				else if(y+5>height){
					return HTBOTTOM;
				}
			}
		}
		break;
	case WM_NCLBUTTONUP:
		{
			handler=(TransparentWnd*)GetWindowLong(hWnd, GWL_USERDATA);
			RECT rect;
			GetWindowRect(hWnd, &rect);
			handler->Move(rect.left, rect.top);
			POINT pt;
			GetCursorPos(&pt);
				
			handler->SetSize(rect.right-rect.left,rect.bottom-rect.top);
		}
		break;
	case WM_LBUTTONDOWN:
	case WM_RBUTTONDOWN:
		{
			handler=(TransparentWnd*)GetWindowLong(hWnd, GWL_USERDATA);
			CefRefPtr<ClientHandler> g_handler=handler->g_handler;
			browser = g_handler->GetBrowser();
			SetCapture(hWnd);
			SetFocus(hWnd);
			browser->SendMouseClickEvent(LOWORD(lParam), HIWORD(lParam),
				(message==WM_LBUTTONDOWN?MBT_LEFT:MBT_RIGHT), false, 1);
			if(handler->isEnableDrag){
				handler->dragX=LOWORD(lParam);
				handler->dragY=HIWORD(lParam);
				handler->isDrag=true;
			}
		}
		break;

	case WM_LBUTTONUP:
	case WM_RBUTTONUP:
		{
			handler=(TransparentWnd*)GetWindowLong(hWnd, GWL_USERDATA);
			CefRefPtr<ClientHandler> g_handler=handler->g_handler;
			browser = g_handler->GetBrowser();
			if (GetCapture() == hWnd)
				ReleaseCapture();
			browser->SendMouseClickEvent(LOWORD(lParam), HIWORD(lParam),
				(message==WM_LBUTTONUP?MBT_LEFT:MBT_RIGHT), true, 1);
			handler->isDrag=false;
		}
		break;

	case WM_MOUSEMOVE:
		{
			handler=(TransparentWnd*)GetWindowLong(hWnd, GWL_USERDATA);
			if(handler->isDrag){
				POINT pt;
				GetCursorPos(&pt);
				handler->Move(pt.x-handler->dragX, pt.y-handler->dragY);
				handler->MoveHandler(handler->x,handler->y);
			}
			else{
				CefRefPtr<ClientHandler> g_handler=handler->g_handler;
				browser = g_handler->GetBrowser();
				browser->SendMouseMoveEvent(LOWORD(lParam), HIWORD(lParam),	false);
			}
			 if (_bMouseTrack)    //若允许追踪，则。
			 {
			  TRACKMOUSEEVENT csTME;
			  csTME.cbSize = sizeof(csTME);
			  csTME.dwFlags = TME_LEAVE|TME_HOVER;
			  csTME.hwndTrack = hWnd;//指定要追踪的窗口
			  csTME.dwHoverTime = 10;  //鼠标在按钮上停留超过10ms，才认为状态为HOVER
			  ::TrackMouseEvent(&csTME); //开启Windows的WM_MOUSELEAVE，WM_MOUSEHOVER事件支持
			  _bMouseTrack=FALSE;   //若已经追踪，则停止追踪
			}
		}
		break;

	case WM_MOUSELEAVE:
		{
			handler=(TransparentWnd*)GetWindowLong(hWnd, GWL_USERDATA);
			CefRefPtr<ClientHandler> g_handler=handler->g_handler;
			browser = g_handler->GetBrowser();
			browser->SendMouseMoveEvent(LOWORD(lParam)-handler->x, HIWORD(lParam)-handler->y, true);
			handler->LeaveHandler();
			_bMouseTrack=TRUE;
		}
		break;

	case WM_MOUSEWHEEL:
		{
			handler=(TransparentWnd*)GetWindowLong(hWnd, GWL_USERDATA);
			CefRefPtr<ClientHandler> g_handler=handler->g_handler;
			browser = g_handler->GetBrowser();
			browser->SendMouseWheelEvent(LOWORD(lParam)-handler->x, HIWORD(lParam)-handler->y, GET_WHEEL_DELTA_WPARAM(wParam), GET_WHEEL_DELTA_WPARAM(wParam));
		}
		break;

	case WM_SETFOCUS:
		{
			handler=(TransparentWnd*)GetWindowLong(hWnd, GWL_USERDATA);
			CefRefPtr<ClientHandler> g_handler=handler->g_handler;
			browser = g_handler->GetBrowser();
			handler->FocusHandler();
			browser->SendFocusEvent(message==WM_SETFOCUS);
			//handler->isMini=false;
			//GetCapture();
		}
		break;
	case WM_KILLFOCUS:
		{
			handler=(TransparentWnd*)GetWindowLong(hWnd, GWL_USERDATA);
			CefRefPtr<ClientHandler> g_handler=handler->g_handler;
			browser = g_handler->GetBrowser();
			browser->SendFocusEvent(message==WM_SETFOCUS);
			if (GetCapture() == hWnd)
				ReleaseCapture();
			//handler->isMini=true;
		}
		break;
	case WM_CAPTURECHANGED:
	case WM_CANCELMODE:
		{
			handler=(TransparentWnd*)GetWindowLong(hWnd, GWL_USERDATA);
			CefRefPtr<ClientHandler> g_handler=handler->g_handler;
			browser = g_handler->GetBrowser();
			browser->SendCaptureLostEvent();
		}
		break;
	case WM_INPUTLANGCHANGE:
	case WM_INPUTLANGCHANGEREQUEST:
		break;
	case WM_KEYDOWN:
	case WM_KEYUP:
	case WM_SYSKEYDOWN:
	case WM_SYSKEYUP:
	case WM_CHAR:
	case WM_SYSCHAR:
	case WM_IME_CHAR:
		{
			CefBrowser::KeyType type = KT_CHAR;
			CefKeyInfo cki;
			bool sysChar = false, imeChar = false;

			if (message == WM_KEYDOWN || message == WM_SYSKEYDOWN)
				type = KT_KEYDOWN;
			else if (message == WM_KEYUP || message == WM_SYSKEYUP)
				type = KT_KEYUP;

			if (message == WM_SYSKEYDOWN || message == WM_SYSKEYUP ||
				message == WM_SYSCHAR)
				sysChar = true;

			if (message == WM_IME_CHAR)
				imeChar = true;

			cki.imeChar=imeChar;
			cki.sysChar=sysChar;
			cki.key=wParam;
			handler=(TransparentWnd*)GetWindowLong(hWnd, GWL_USERDATA);
			CefRefPtr<ClientHandler> g_handler=handler->g_handler;
			browser = g_handler->GetBrowser();
			browser->SendKeyEvent(type, cki, lParam);
			return 0;
		}
		break;
	case WM_CLOSE:
		{
			handler=(TransparentWnd*)GetWindowLong(hWnd, GWL_USERDATA);
			if(handler->g_handler->win){
				handler->CloseHandler();
				return 0;
			};
		}
		break;
	case WM_SIZING:
		{
			handler=(TransparentWnd*)GetWindowLong(hWnd, GWL_USERDATA);
			LPRECT lpRect=(LPRECT)lParam;
			int m_nWidth = lpRect->right-lpRect->left;
			int m_nHeight = lpRect->bottom-lpRect->top;
			handler->Move(lpRect->left,lpRect->top);
			handler->SetSize(m_nWidth,m_nHeight);
		}
		break;
	case WM_NCMOUSEMOVE:
		{
			RECT rect;
			handler=(TransparentWnd*)GetWindowLong(hWnd, GWL_USERDATA);
			if(handler){
				int x = (int)LOWORD(lParam);
				int y = (int)HIWORD(lParam);
				GetWindowRect(hWnd, &rect);
			}
		}
		break;
	case WM_MOVE:
		{
			RECT rect;
			handler=(TransparentWnd*)GetWindowLong(hWnd, GWL_USERDATA);
			if(handler){
				int x = (int)LOWORD(lParam);
				int y = (int)HIWORD(lParam);
				GetWindowRect(hWnd, &rect);
			}
		}
		break;
	case WM_PAINT:
		{
			PAINTSTRUCT ps;
			BeginPaint(hWnd, &ps);
			EndPaint(hWnd, &ps);
		}
		return 0;
	case WM_ERASEBKGND:
		return 0;
	default:
		break;
	}
	return DefWindowProc(hWnd, message, wParam, lParam);
}
HICON TransparentWnd::GetIcon(CefString path){
	if(path.ToWString().find(L":")==-1){
		wstring _path;
		_path=url.ToWString();
		replace_allW(_path, L"\\", L"/");
		_path=_path.substr(0,_path.find_last_of('/')+1);
		path=_path.append(path);
	}
	return (HICON)::LoadImage(NULL,path.ToWString().data(),IMAGE_ICON,0,0,LR_DEFAULTSIZE|LR_LOADFROMFILE); 
	//Bitmap bm(path.ToWString().data());
	//HICON hIcon;
	//bm.GetHICON(&hIcon);
	//return hIcon;
}
void TransparentWnd::SetTaskIcon(int id, CefString iconPath, CefString title){
	// 将图标放入系统托盘
	NOTIFYICONDATA nd;
	nd.cbSize	= sizeof (NOTIFYICONDATA);
	nd.hWnd	= hWnd;
	nd.uID	= id;
	nd.uFlags	= NIF_ICON|NIF_MESSAGE|NIF_TIP;
	nd.uCallbackMessage	= WM_NOTIFYICON;
	nd.hIcon = GetIcon(iconPath);
	wcscpy(nd.szTip, title.ToWString().data());
	Shell_NotifyIcon(NIM_ADD, &nd);
}
void TransparentWnd::TaskMouseHandler(UINT type){
	std::stringstream ss;
	ss<<"var e = new CustomEvent('webtopTaskMouse',{"
		<<"	detail:{"
		<<"		type:"<<type
		<<"	}"
		<<"});"
		<<"dispatchEvent(e);";
	ExecJS(ss.str());
}
void TransparentWnd::DelTaskIcon(int id){
		// 将图标放入系统托盘
	NOTIFYICONDATA nd;
	nd.cbSize	= sizeof (NOTIFYICONDATA);
	nd.hWnd	= hWnd;
	nd.uID	= id;
	nd.uFlags	= NIF_ICON|NIF_MESSAGE|NIF_TIP;
	nd.uCallbackMessage	= WM_NOTIFYICON;
	nd.hIcon	= NULL;//m_hIcon;
	
	Shell_NotifyIcon(NIM_DELETE, &nd);
}
VOID CALLBACK TransparentWnd::OnTimerProc(HWND   hWnd,UINT   uMsg,UINT_PTR   idEvent,DWORD   dwTime) 
{ 
	TransparentWnd *handler=(TransparentWnd*)GetWindowLong(hWnd, GWL_USERDATA);
	handler->Render();
}
void TransparentWnd::SaveImageFromStream(CefString path,AmfStream* pStream,int width,int height){
	Bitmap bm(width,height);
	Rect r(0,0,width,height);
	BitmapData bmData;
	bm.LockBits(&r, ImageLockModeWrite, PixelFormat32bppARGB, &bmData);
	int l=width*height*4;
	byte* p = (byte*)bmData.Scan0;
	BYTE* pb2=pStream->GetStream();
	for(int i=0;i<l;i+=4){
		p[i+2]=pb2[i];
		p[i]=pb2[i+2];
		p[i+1]=pb2[i+1];
		p[i+3]=pb2[i+3];
	}
	bm.UnlockBits(&bmData);
	CLSID tiffClsid;
	wstring type=wstring(L"image/")+GetExtW(path.ToWString());
	GetEncoderClsid(type.data(), &tiffClsid);
	bm.Save(TranslatePath(path).ToWString().data(), &tiffClsid);
}
LRESULT CALLBACK TransparentWnd::WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	static CefRefPtr<CefBrowser> browser = NULL;
	static HWND browserhWnd=NULL;
	static RECT rect;
	static TransparentWnd* handler;
	static int top=GetSystemMetrics(SM_CYFRAME)+GetSystemMetrics(SM_CYCAPTION)-2,left=GetSystemMetrics(SM_CXFRAME);
	switch (message)
	{
	case WM_INIT:
		{
			// Create the single static handler class instance
			handler=(TransparentWnd*)GetWindowLong(hWnd, GWL_USERDATA);
			CefRefPtr<MyHandler> g_handler=handler->g_handler;
			g_handler->SetMainHwnd(hWnd);


			GetClientRect(hWnd, &rect);

			CefWindowInfo info;
			CefBrowserSettings settings;

			// Populate the settings based on command line arguments.
			AppGetBrowserSettings(settings);
			// Initialize window info to the defaults for a child window
			info.SetAsChild(hWnd, rect);

			if(handler->isTransparent){
				info.m_bWindowRenderingDisabled = TRUE;
				info.SetTransparentPainting(TRUE);
			}

			// Creat the new child browser window
			CefBrowser::CreateBrowserSync(info,
				static_cast<CefRefPtr<CefClient> >(g_handler),
				handler->GetUrl(), settings);
			DragAcceptFiles(hWnd,TRUE);
		}
		break;
	case WM_NOTIFYICON:
		{
			handler=(TransparentWnd*)GetWindowLong(hWnd, GWL_USERDATA);
			handler->TaskMouseHandler(lParam);
		}
		break;
	case WM_ACTIVATE:
		{
			handler=(TransparentWnd*)GetWindowLong(hWnd, GWL_USERDATA);
			handler->FocusHandler();
		}
		break;
	case WM_CLOSE:
		{
			handler=(TransparentWnd*)GetWindowLong(hWnd, GWL_USERDATA);
			if(handler->g_handler->win){
				handler->CloseHandler();
				return 0;
			};
		}
		break;
	case WM_LBUTTONDOWN:
	case WM_RBUTTONDOWN:
		{
			handler=(TransparentWnd*)GetWindowLong(hWnd, GWL_USERDATA);
			if(handler->isEnableDrag){
				SetCapture(hWnd);
				handler->dragX=LOWORD(lParam);
				handler->dragY=HIWORD(lParam);
				handler->isDrag=true;
			}
		}
		break;

	case WM_LBUTTONUP:
	case WM_RBUTTONUP:
		{
			handler=(TransparentWnd*)GetWindowLong(hWnd, GWL_USERDATA);
			if (GetCapture() == hWnd)
				ReleaseCapture();
			handler->isDrag=false;
		}
		break;

	case WM_MOUSEMOVE:
		{
			handler=(TransparentWnd*)GetWindowLong(hWnd, GWL_USERDATA);
			if(handler->isDrag){
				POINT pt;
				GetCursorPos(&pt);
				handler->Move(pt.x-handler->dragX, pt.y-handler->dragY);
				handler->MoveHandler(handler->x,handler->y);
			}
		}
		break;

	case WM_SIZE:
		{
			handler=(TransparentWnd*)GetWindowLong(hWnd, GWL_USERDATA);
			switch(wParam){
				case SIZE_MINIMIZED:
					ShowWindow(handler->hWnd, SW_MINIMIZE);
					break;
				case SIZE_RESTORED:
					break;
				default:
					handler=(TransparentWnd*)GetWindowLong(hWnd, GWL_USERDATA);
					GetWindowRect(hWnd, &rect);
					int m_x = rect.left;
					int m_y = rect.top;
					int m_nWidth = rect.right - rect.left;
					int m_nHeight = rect.bottom - rect.top;
					if(rect.right-rect.left>0){
						handler->SetSize(m_nWidth,m_nHeight);
					}
			}
			if(!handler->isTransparent){
				RECT  rect;
				GetClientRect(hWnd, &rect);
				MoveWindow(handler->g_handler->GetBrowserHwnd(), rect.left, rect.top, rect.right-rect.left, rect.bottom-rect.top, false);
			}
		}
		break;
	case WM_SIZING:
		{
			handler=(TransparentWnd*)GetWindowLong(hWnd, GWL_USERDATA);
			LPRECT lpRect=(LPRECT)lParam;
			int m_nWidth = lpRect->right-lpRect->left;
			int m_nHeight = lpRect->bottom-lpRect->top;
			handler->Move(lpRect->left,lpRect->top);
			handler->SetSize(m_nWidth,m_nHeight);
		}
		break;
	default:
		break;
	}	

	return DefWindowProc(hWnd, message, wParam, lParam);
}
CSFMServer* TransparentWnd::CreateMemory(CefString name, CefString fileName, int size){
	if(fileName.length()>0){
		return new CSFMServer(TranslatePath(fileName).ToWString().data(), name.ToWString().data(), size);
	}
	else{
		return new CSFMServer(NULL, name.ToWString().data(), size);
	}
}
void TransparentWnd::DeleteMemory(CSFMServer* mem){
	delete mem;
}
AmfStream* TransparentWnd::CreateStream(CSFMServer* mem){
	PBYTE p=(PBYTE)mem->GetBuffer();
	return new AmfStream(p);
}
void TransparentWnd::DeleteStream(AmfStream* pStream){
	delete pStream;
}
void TransparentWnd::ShowTip(CefString& text){
	return;
	TransparentWnd* pWin;
	if(!pTipWin){
		if(text.length()){
			pWin=new TransparentWnd();
			pTipWin=(void *)pWin;
			std::stringstream ss;
			POINT pt;
			GetCursorPos(&pt);
			ss<<"{\"text\":\""<<text.ToString()<<"\",\"x\":"<<pt.x<<",\"y\":"<<pt.y<<"}";
			pWin->RunAppIn("tip/index.app",ss.str());
		}
	}
	else{
		pWin=(TransparentWnd *)pTipWin;
		if(text.length()){
			std::wstringstream ss;
			POINT pt;
			GetCursorPos(&pt);
			wstring t=text.ToWString();
			replace_allW(t,L"'",L"\'");
			ss<<"var e = new CustomEvent('webtopShowTip', {"
				"detail: {"
				"	'text':'"<<t<<"',"<<
				"   'x':"<<pt.x<<","
				"   'y':"<<pt.y<<
				"}"
			"});"
			"dispatchEvent(e);";
			pWin->ExecJS(ss.str());
			ExecJS(ss.str());
			if(pWin->isHide){
				pWin->Restore();
			}
		}
		else{
			pWin->Hide();
		}
	}
}
TransparentWnd::TransparentWnd(void)
{
	x=0;
	y=0;
	width=0;
	height=0;
	isDrag=false;
	isEnableDrag=false;
	m_buffer=NULL;
	readyHandler="";
	isMini=false;
	downloadHandler=NULL;
	url="";
	hIcon=LoadIcon(hInst, MAKEINTRESOURCE(IDI_CEFCLIENT));
	enableResize=false;
	++count;
	if(count==1){
		GdiplusStartup(&m_gdiplusToken, &gdiplusStartupInput, NULL);
	}
	p2p.winHandler=(LPVOID)this;
	pClient=new CSFMClient(FILE_MAP_READ|FILE_MAP_WRITE, L"Webtop");
	PBYTE p=(PBYTE)pClient->GetBuffer();
	pStream = new AmfStream(p);
	hBitMap=NULL;
	pTipWin=NULL;
}

TransparentWnd::~TransparentWnd(void)
{
	if(m_buffer){
		delete m_buffer;
	}
	g_handler->win=NULL;
	SendMessage(hWnd, WM_CLOSE, 0, 0);
	SendMessage(renderWindow, WM_CLOSE, 0, 0);
	--count;
	if (g_handler.get()) {
		CefRefPtr<CefBrowser> browser = g_handler->GetBrowser();
		if (browser.get()) {
			// Let the browser window know we are about to destroy it.
			browser->ParentWindowWillClose();
		}
	}
	delete pClient;
	delete pStream;
	if(hBitMap){
		DeleteObject(hBitMap);
	}
	if(pTipWin){
		delete (TransparentWnd*)pTipWin;
	}
	if(count==0){
		delete pServer;
		GdiplusShutdown(m_gdiplusToken);
		PostQuitMessage(0);
	}
}

void TransparentWnd::SetUrl(CefString url){
	this->url=url;
	g_handler->GetBrowser()->GetMainFrame()->LoadURL(url);
}

void TransparentWnd::ReloadIgnoreCache(){
	//g_handler->GetBrowser()->Release();
	g_handler->GetBrowser()->ReloadIgnoreCache();
}

void TransparentWnd::SetWindowStyle(UINT dwNewExStyle){
	HWND hWndTemp=isTransparent?renderWindow:hWnd;
	DWORD dwExStyle = GetWindowLong(hWndTemp, GWL_EXSTYLE);//获取旧样式  
	dwNewExStyle|=dwExStyle;//按位与将旧扩展样式去掉  
	SetWindowLong(hWndTemp,GWL_EXSTYLE,dwNewExStyle);//设置新的扩展样式  
}

void TransparentWnd::SetToolWindow(){
}

void TransparentWnd::SetTopMost(){
	DWORD dwExStyle = GetWindowLong(hWnd, GWL_EXSTYLE);//获取旧样式  
	DWORD dwNewExStyle = WS_EX_TOPMOST;  
	dwNewExStyle|=dwExStyle;//按位与将旧扩展样式去掉  
	SetWindowLong(hWnd,GWL_EXSTYLE,dwNewExStyle);//设置新的扩展样式
	SetWindowPos(hWnd,HWND_TOPMOST,0,0,0,0,SWP_NOMOVE | SWP_NOSIZE);
	if(isTransparent){
		SetWindowPos(renderWindow,HWND_TOPMOST,0,0,0,0,SWP_NOMOVE | SWP_NOSIZE);
	}
}

void TransparentWnd::Reload(){
	g_handler->GetBrowser()->Reload();
}

HWND TransparentWnd::GetHwnd(){
	return hWnd;
}

HWND TransparentWnd::GetRenderHwnd(){
	return renderWindow;
}

CefString TransparentWnd::GetUrl(){
	return url;
}

void TransparentWnd::SetHinst(HINSTANCE hinst){
	this->hinst=hinst;
}

void TransparentWnd::Browse(CefString& url){
	RunApp("browser/index.app",url);
}

void TransparentWnd::DropHandler(HDROP hDrop){
	CefRefPtr<CefFrame> frame=g_handler->GetBrowser()->GetMainFrame();
	TCHAR   szFilePathName[MAX_PATH+1] = {0};
	UINT  nNumOfFiles = DragQueryFile(hDrop, 0xFFFFFFFF, NULL, 0); //得到文件个数
	std::stringstream ss;
	ss<<"var e = new CustomEvent('webtopDragDrop',{"
		<<"	detail:{"
		<<"		list:[\"";
	for (UINT nIndex=0 ; nIndex< nNumOfFiles-1; ++nIndex)
	{
		DragQueryFile(hDrop, nIndex, szFilePathName, MAX_PATH);  //得到文件名
		wstring path(szFilePathName);
		replace_allW(path, L"\\", L"/");
		CefString url(path);
		ss<<url.ToString().c_str()<<"\",\"";
	}
	DragQueryFile(hDrop, nNumOfFiles-1, szFilePathName, MAX_PATH);
	wstring path(szFilePathName);
	replace_allW(path, L"\\", L"/");
	CefString url(path);
	ss<<url.ToString().c_str()<<"\"]"
		<<"	}"
		<<"});"
		<<"dispatchEvent(e);";
	ExecJS(ss.str());
}
void TransparentWnd::MoveHandler(int x, int y){
	std::stringstream ss;
	ss<<"var e = new CustomEvent('webtopWindowMove',{"
		<<"	detail:{"
		<<"		x:"<<x<<","
		<<"		y:"<<y
		<<"	}"
		<<"});"
		<<"dispatchEvent(e);";
	ExecJS(ss.str());
}
void TransparentWnd::LeaveHandler(){
	string s="var e = new CustomEvent('webtopMouseLeave');"
	"dispatchEvent(e);";
	ExecJS(s);
}

CefString TransparentWnd::ReadFile(CefString path){
	wstring pathS=path.ToWString();
	wstring urlS=url.ToWString();
	if(pathS.find(L":")==-1){
		if(urlS.find(L"http")==-1){
			int index1=urlS.find_last_of(L"/");
			pathS=urlS.substr(0,index1+1).append(pathS);
		}
	}
	ifstream fin(pathS);
	std::stringstream ss;
	string s;
	if(getline(fin, s)){
		ss << s;
		while(getline(fin, s)){
			ss <<'\n'<< s;
		}
	}
	s=ss.str();
	int l=s.length();
	const char* s1=s.c_str();
	unsigned char* s3=(unsigned char* )s1;
	int type=3;
	if(s3[0]==0xff&&s3[1]==0xfe){
		type=0;
	}
	else if(s3[0]==0xfe&&s3[1]==0xff){
		type=1;
	}
	else if(s3[0]==0xef&&s3[1]==0xbb&&s3[2]==0xbf){
		type=2;
	}
	else if(IsTextUTF8(s1,l)){
		type=2;
	}
	CefString cs;
	if(type==3){
		DWORD dwNum = MultiByteToWideChar (CP_ACP, 0, s1, -1, NULL, 0);
		WCHAR *s2=new WCHAR[dwNum];
		::MultiByteToWideChar(CP_ACP,0,s1,-1,s2,dwNum);
		cs=s2;
		delete []s2;
	}
	else if(type==0||type==1){
		wstring s4=(WCHAR *)s3;
		cs=s4;
	}
	else{
		cs=s;
	}
	fin.close();
	return cs;
}

bool TransparentWnd::WriteFile(CefString path, CefString s){
	wstring pathS=path.ToWString();
	wstring urlS=url.ToWString();
	replace_allW(urlS,L"file:///",L"");
	if(pathS.find(L":")==-1){
		if(urlS.find(L"http")==-1){
			int index1=urlS.find_last_of(L"/");
			pathS=urlS.substr(0,index1+1).append(pathS);
		}
	}
	ofstream fout(pathS);
	fout<<s.ToString();
	fout.flush();
	fout.close();
	return true;
}

void TransparentWnd::CloseHandler(){
	string s="var e = new CustomEvent('webtopWindowClose');"
	"dispatchEvent(e);"
	"setTimeout('webtop.close(handler)',100);";
	ExecJS(s);
}
CefString TransparentWnd::TranslatePath(CefString path){
	wstring pathS=path.ToWString();
	wstring urlS=url.ToWString();
	replace_allW(urlS,L"file:///",L"");
	if(pathS.find(L":")==-1){
		int index1=urlS.find_last_of(L"/");
		pathS=urlS.substr(0,index1+1).append(pathS);
	}
	return pathS;
}

void TransparentWnd::ToImage(CefString path){
	if(!hBitMap){
		return;
	}
	Bitmap bm(width,height);
	Rect r(0,0,width,height);
	BitmapData bmData;
	bm.LockBits(&r, ImageLockModeWrite, PixelFormat32bppPARGB, &bmData);
	int l=width*height*4;
	PBYTE p1=new BYTE[l];
	GetBitmapBits(hBitMap, l, p1);
	byte* p = (byte*)bmData.Scan0;
	for (int j = 0; j < l; j++)
	{
		p[j]=p1[j];
	}
	delete []p1;
	bm.UnlockBits(&bmData);
	CLSID tiffClsid;
	GetEncoderClsid((L"image/"+GetExtW(path.ToWString())).data(), &tiffClsid);
	bm.Save(TranslatePath(path).ToWString().data(), &tiffClsid);
}

void TransparentWnd::ToImageEx(CefString path, int _x, int _y, int _width, int _height){
	if(!hBitMap){
		return;
	}
	Bitmap bm(_width,_height);
	Rect r(0,0,_width,_height);
	BitmapData bmData;
	bm.LockBits(&r, ImageLockModeWrite, PixelFormat32bppPARGB, &bmData);
	int l=width*height*4;
	PBYTE p1=new BYTE[l];
	GetBitmapBits(hBitMap, l, p1);
	byte* p = (byte*)bmData.Scan0;
	int count=0;
	for(int j=0;j<_height;++j){
		for (int i = 0; i < _width; i++)
		{
			for(int k=0; k<4;++k){
				int pos = (_y+j)*width*4+(_x+i)*4;
				p[count]=p1[pos+k];
				++count;
			}
		}
	}
	delete []p1;
	bm.UnlockBits(&bmData);
	CLSID tiffClsid;
	GetEncoderClsid((L"image/"+GetExtW(path.ToWString())).data(), &tiffClsid);
	bm.Save(TranslatePath(path).ToWString().data(), &tiffClsid);
}

void TransparentWnd::SizeHandler(){
	std::stringstream ss;
	ss<<"var e = new CustomEvent('webtopWindowResize',{"
		<<"	detail:{"
		<<"		width:"<<width<<","
		<<"		height:"<<height
		<<"	}"
		<<"});"
		<<"dispatchEvent(e);";
	ExecJS(ss.str());
}

void TransparentWnd::FocusHandler(){
	std::stringstream ss;
	string s="var e = new CustomEvent('webtopWindowFocus');"
	"dispatchEvent(e);";
	ExecJS(s);
}
void TransparentWnd::ActiveHandler(){
	std::stringstream ss;
	string s="var e = new CustomEvent('webtopWindowActive');"
	"dispatchEvent(e);";
	ExecJS(s);
}

void TransparentWnd::ExecJS(CefString s){
	CefRefPtr<CefFrame> frame=g_handler->GetBrowser()->GetMainFrame();
	if(frame){
		frame->ExecuteJavaScript(s, "", 0);
	}
}

void TransparentWnd::Ready(){
	ExecJS(readyHandler);
}

void TransparentWnd::Max(){
	Move(0,0);
	SetSize(GetSystemMetrics(SM_CXFULLSCREEN),GetSystemMetrics(SM_CYFULLSCREEN)+GetSystemMetrics(SM_CYCAPTION));
}

void TransparentWnd::Mini(){
	if(isTransparent){
		ShowWindow(hWnd, SW_HIDE);
	}
	else{
		if(GetWindowLong(hWnd, GWL_EXSTYLE)&WS_EX_TOOLWINDOW){
			ShowWindow(hWnd, SW_HIDE);
		}
		else{
			ShowWindow(hWnd, SW_MINIMIZE);
		}
	}
	ShowWindow(renderWindow, SW_MINIMIZE);
}

void TransparentWnd::Hide(){
	ShowWindow(hWnd, SW_HIDE);
	ShowWindow(renderWindow, SW_HIDE);
	isHide=true;
}

void TransparentWnd::Restore(){
	ShowWindow(renderWindow, SW_RESTORE);
	ShowWindow(hWnd, SW_RESTORE);
	isHide=false;
}

void TransparentWnd::BringToTop(){
	if(isTransparent){
		SetWindowPos(renderWindow, HWND_TOP, x, y, width, height, SWP_NOACTIVATE);
	}
	else{
		SetWindowPos(hWnd, HWND_TOP, x, y, width, height, SWP_NOACTIVATE);
	}
}

void TransparentWnd::SetAsChild(TransparentWnd *parent){
}

void TransparentWnd::Focus(){
	if(isTransparent){
		SetFocus(renderWindow);
	}
	else{
		SetFocus(hWnd);
	}
	g_handler->GetBrowser()->SendFocusEvent(true);
}

void TransparentWnd::Move(int x, int y){
	this->x=x;
	this->y=y;
	MoveHandler(x,y);
	if(isTransparent){
		SetWindowPos(renderWindow, 0, x, y, width, height, SWP_NOZORDER|SWP_NOACTIVATE|SWP_NOSIZE);
		//MoveWindow(renderWindow, x, y, width, height, false);
	}
	else{
		MoveWindow(hWnd, x, y, width, height, false);
	}
}
void TransparentWnd::Render(const void* buffer){
	UINT size = width * height * 4;
	if(isMini){
		return;
	}
	else if(size==0){
		return;
	}
	CefRefPtr<CefBrowser> browser = NULL;
	HWND browserhWnd=NULL;
	static int top=GetSystemMetrics(SM_CYFRAME)+GetSystemMetrics(SM_CYCAPTION)-2,left=GetSystemMetrics(SM_CXFRAME);
	browser = g_handler->GetBrowser();
	RECT winRect;
	GetWindowRect(renderWindow, &winRect);
	BLENDFUNCTION bf;
	bf.BlendOp = AC_SRC_OVER;
	bf.AlphaFormat = AC_SRC_ALPHA;
	bf.BlendFlags = 0;
	bf.SourceConstantAlpha = 255;
	x=winRect.left;
	y=winRect.top;
	browser->GetSize(PET_VIEW, width, height);
	POINT p = {x,y};
	POINT p2 = {0, 0};
	SIZE sz = {width,height};
	HDC renderDC=GetWindowDC(renderWindow);
	HDC hdcMemory=CreateCompatibleDC(renderDC);
	if(hBitMap){
		DeleteObject(hBitMap);
	}
	hBitMap = CreateCompatibleBitmap(renderDC, width, height);
	SelectObject(hdcMemory, hBitMap);
	SetBitmapBits(hBitMap, size, buffer);
	BOOL bRet = UpdateLayeredWindow(renderWindow, renderDC, &p, &sz, hdcMemory, &p2, RGB(0,0,0), &bf, ULW_ALPHA);
	DeleteObject(hdcMemory);
	ReleaseDC(renderWindow, renderDC);
}
void TransparentWnd::GetSize(int& w, int& h){
	RECT rect;
	GetClientRect(hWnd, &rect);
	w=rect.right-rect.left;
	h=rect.bottom-rect.top;
}
void TransparentWnd::CreateBrowser(CefString url, CefString param){
    HANDLE m_hRead;
	HANDLE m_hWrite;
	SECURITY_ATTRIBUTES sa;   // 父进程传递给子进程的一些信息
    sa.bInheritHandle = TRUE; // 还记得我上面的提醒吧，这个来允许子进程继承父进程的管道句柄
    sa.lpSecurityDescriptor = NULL;
    sa.nLength = sizeof(SECURITY_ATTRIBUTES);
    if (!CreatePipe(&m_hRead, &m_hWrite, &sa, 0))
    {
        return;
    }

    STARTUPINFO sui;    
    PROCESS_INFORMATION pi; // 保存了所创建子进程的信息
    ZeroMemory(&sui, sizeof(STARTUPINFO)); // 对一个内存区清零，最好用ZeroMemory, 它的速度要快于memset
    sui.cb = sizeof(STARTUPINFO);
    sui.dwFlags = STARTF_USESTDHANDLES;
        
    sui.hStdInput = m_hRead;
	sui.hStdOutput = m_hWrite;
    /* 以上两行也许大家要有些疑问，为什么把管道读句柄（m_hRead）赋值给了hStdInput, 因为管道是双向的，对于父进程写的一端正好是子进程读的一端，而m_hRead就是父进程中对管道读的一端， 自然要把这个句柄给子进程让它来写数据了(sui是父进程传给子进程的数据结构，里面包含了一些父进程要告诉子进程的一些信息)，反之一样*/
    sui.hStdError = GetStdHandle(STD_ERROR_HANDLE);
	TCHAR   szPath[1000];   
    GetModuleFileName(NULL,szPath,MAX_PATH);
	DWORD d;
	if(::WriteFile(m_hWrite,param.ToString().c_str(),strlen(param.ToString().c_str()),&d,NULL)==FALSE){
		MessageBox(NULL,L"写入失败",L"错误",0);
	}
	if (!CreateProcess(szPath, (LPWSTR)url.ToWString().c_str(), NULL, NULL, TRUE, 0, NULL, NULL, &sui, &pi))
    {
        CloseHandle(m_hRead);
        CloseHandle(m_hWrite);
    }
    else
    {
        CloseHandle(pi.hProcess); // 子进程的进程句柄
        CloseHandle(pi.hThread); // 子进程的线程句柄，windows中进程就是一个线程的容器，每个进程至少有一个线程在执行
    }
}
void TransparentWnd::SetReadyHandler(CefString s){
	readyHandler=s;
}
void TransparentWnd::SetTitle(CefString title){
	SetWindowText(renderWindow, title.ToWString().data());
}
void TransparentWnd::Drag(){
	SendMessage(hWnd, WM_SYSCOMMAND, SC_MOVE+HTCAPTION, 0);
}
void TransparentWnd::StartDrag(){
	POINT pt;
	GetCursorPos(&pt);
	if(isTransparent){
		isDrag=true;
		dragX=pt.x-x;
		dragY=pt.y-y;
	}
	else{
		SendMessage(hWnd, WM_NCLBUTTONDOWN, HTCAPTION, MAKELPARAM(pt.x, pt.y));
	}
}
void TransparentWnd::EnableDrag(){
	isEnableDrag=true;
}
void TransparentWnd::Download(CefString url,CefString filename){
	std::stringstream ss;
	string s=url.ToString();
	ss<<"var img = new Image();"
		<<"img.src='"<<url.ToString();
	if(s.find("?")==string::npos){
		ss<<"?t='+Date.now()+'&webtop_download=";
	}
	else{
		ss<<"&webtop_download=";
	}
	ss<<filename.ToString()<<"';";
	this->ExecJS(ss.str());
}
void TransparentWnd::SetSize(int w, int h){
	this->width=w;
	this->height=h;
	g_handler->SetSize(w, h);
	SizeHandler();
	if(isTransparent){
		MoveWindow(renderWindow, x, y, w, h, false);
	}
	else{
		MoveWindow(hWnd, x, y, w, h, false);
	}
}
void TransparentWnd::GetUsers(){
	p2p.GetU();
}
void TransparentWnd::RecieveMessage(int type, char* message, char* ip, unsigned short port){
	switch(type){
	case P2PMESSAGE:
		{
			std::stringstream ss;
			string msg(message);
			replace_all(msg,"'","\'");
			ss<<"var e = new CustomEvent('webtopP2PRecieveMessage',{"
				"	detail:{"
				"		ip:'"<<p2p.IP<<"',"
				"		port:"<<p2p.port<<","
				"		message:'"<<msg<<"'"
				"	}"
				"});"<<
				"dispatchEvent(e);";
			ExecJS(ss.str());
			break;
		}
	case IPANDPORT:
		{
			std::stringstream ss;
			ss<<"var e = new CustomEvent('webtopP2PInitInfo',{"
				"	detail:{"
				"		ip:'"<<p2p.IP<<"',"
				"		port:"<<p2p.port<<
				"	}"
				"});"<<
				"dispatchEvent(e);";
			ExecJS(ss.str());
		}
		break;
	case GETALLUSER:
		{
 			std::stringstream ss;
			UserList ClientList=p2p.ClientList;
			ss<<"var e = new CustomEvent('webtopP2PUpdateUserList',{"
				"	detail:{"
				"		list:[";
			UserList::iterator UserIterator=ClientList.begin();
			if(UserIterator!=ClientList.end()){
				stUserListNode* node= *UserIterator;
				in_addr tmp;
				tmp.S_un.S_addr = htonl(node->ip);
				ss<<"'"<<inet_ntoa(tmp)<<":"<<node->port<<"_"<<node->userName<<"'";
				++UserIterator;
				for(;UserIterator!=ClientList.end();++UserIterator)
				{
					stUserListNode* node= *UserIterator;
					in_addr tmp;
					tmp.S_un.S_addr = htonl(node->ip);
					ss<<",'"<<inet_ntoa(tmp)<<":"<<node->port<<"_"<<node->userName<<"'";
				}
			}
			ss<<"		],"
				"		ip:'"<<ip<<"',"
				"		port:"<<port<<
				"	}"
				"});"
				"dispatchEvent(e);";
			ExecJS(ss.str());
		}
		break;
	}
}
