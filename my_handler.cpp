#include "my_handler.h"
#include "cefclient/string_util.h"
#include <sstream>
#include <stdio.h>
#include <string>
#include "cefclient/resource.h"
#include "cefclient/resource_util.h"
#include "transparent_wnd.h"
#include "include/cef_download_handler.h"
#include "system.h"
#include "base64.h"
extern HINSTANCE hInst;
extern int CDECL MessageBoxPrintf (TCHAR * szCaption, TCHAR * szFormat, ...)  ;

MyHandler::MyHandler():ClientHandler(){
	win=NULL;
}
MyHandler::~MyHandler(){
}

CefRefPtr<CefDisplayHandler> MyHandler::GetDisplayHandler()
{
	return this;
}

CefRefPtr<CefRenderHandler> MyHandler::GetRenderHandler()
{
	return this; 
}
bool MyHandler::OnBeforeMenu(CefRefPtr<CefBrowser> browser, const CefMenuInfo& menuInfo) {
	if(menuInfo.typeFlags==1){
		return true;
	}
	else{
		return false;
	}
}
CefRefPtr<CefMenuHandler> MyHandler::GetMenuHandler() {
	return this;
}

bool MyHandler::OnKeyEvent(CefRefPtr<CefBrowser> browser,
                               KeyEventType type,
                               int code,
                               int modifiers,
                               bool isSystemKey,
                               bool isAfterJavaScript) {
  REQUIRE_UI_THREAD();
  if(!win)return false;
	TransparentWnd* winHandler=(TransparentWnd*)win;
  if ((code == 123 || code==120)&&type == KEYEVENT_RAWKEYDOWN) {
    // Special handling for the space character if a form element does not have
    // focus.
    if (type == KEYEVENT_RAWKEYDOWN&&winHandler->enableDevelop) {
		GetBrowser()->ShowDevTools();
		return true;
    }
	else{
		string s="var e = new CustomEvent('webtopShowDev');dispatchEvent(e);";
		winHandler->ExecJS(s);
	}
  }
  if(code == 116&&type == KEYEVENT_RAWKEYDOWN){
	if(winHandler->enableRefresh) {
		winHandler->ReloadIgnoreCache();
		return true;
	}
	else{
		string s="var e = new CustomEvent('webtopRefresh');dispatchEvent(e);";
		winHandler->ExecJS(s);
	}
  }
  return false;
}

bool MyHandler::GetDownloadHandler(CefRefPtr<CefBrowser> browser,
                                       const CefString& mimeType,
                                       const CefString& fileName,
                                       int64 contentLength,
                                       CefRefPtr<CefDownloadHandler>& handler)
{
  REQUIRE_UI_THREAD();

  // Create the handler for the file download.
  handler = CreateDownloadHandler(this, fileName, contentLength);

  // Close the browser window if it is a popup with no other document contents.
  if (!browser->HasDocument()){
	if(win){
		TransparentWnd* winHandler=(TransparentWnd*)win;
		//delete winHandler;
	}
    //browser->CloseBrowser();
  }

  return true;
}

void* MyHandler::GetWin(){
	return (void *)this->win;
}

void MyHandler::OnLoadStart(CefRefPtr<CefBrowser> browser,
                                CefRefPtr<CefFrame> frame)
{
}

void MyHandler::OnLoadEnd(CefRefPtr<CefBrowser> browser,
                              CefRefPtr<CefFrame> frame,
                              int httpStatusCode)
{
	std::stringstream ss;
	string s="var e = new CustomEvent('webtopReady');"
	"setTimeout('dispatchEvent(e);',0);";
	ss << "var handler="<<(long)this->win<<";"<<s;
	frame->ExecuteJavaScript(ss.str(), "", 0);
}

bool MyHandler::OnBeforePopup(CefRefPtr<CefBrowser> parentBrowser,
                            const CefPopupFeatures& popupFeatures,
                            CefWindowInfo& windowInfo,
                            const CefString& url,
                            CefRefPtr<CefClient>& client,
                            CefBrowserSettings& settings,
							CefRefPtr<CefBrowser>& newBrowser)
{
	std::string urlStr = url;
	if(urlStr.find("chrome-devtools:") == std::string::npos) {
		TransparentWnd* tp=(TransparentWnd*)this->win;
		tp->RunApp("browser/index.app",url);
	}
	else{
		return false;
	}
	return true;
}

bool MyHandler::OnLoadError(CefRefPtr<CefBrowser> browser,
                                CefRefPtr<CefFrame> frame,
                                ErrorCode errorCode,
                                const CefString& failedUrl,
                                CefString& errorText)
{
  if(errorCode == ERR_CACHE_MISS) {
    // Usually caused by navigating to a page with POST data via back or
    // forward buttons.
    errorText = "<html><head><title>Expired Form Data</title></head>"
                "<body><h1>Expired Form Data</h1>"
                "<h2>Your form request has expired. "
                "Click reload to re-submit the form data.</h2></body>"
                "</html>";
  } else {
    // All other messages.
    std::stringstream ss;
    ss <<       "<html><head><title>Load Failed</title></head>"
                "<body><h1>Load Failed</h1>"
                "<h2>Load of URL " << std::string(failedUrl) <<
                " failed with error code " << static_cast<int>(errorCode) <<
                ".</h2></body>"
                "</html>";
    errorText = ss.str();
  }
  return true;
}

void MyHandler::OnAfterCreated(CefRefPtr<CefBrowser> browser)
{
  REQUIRE_UI_THREAD();

  AutoLock lock_scope(this);
  if(!m_Browser.get())
  {
    // We need to keep the main child window, but not popup windows
    m_Browser = browser;
    m_BrowserHwnd = browser->GetWindowHandle();
  }
}
bool MyHandler::GetViewRect(CefRefPtr<CefBrowser> browser,
                           CefRect& rect)
  {
    REQUIRE_UI_THREAD();

    // The simulated screen and view rectangle are the same. This is necessary
    // for popup menus to be located and sized inside the view.
	if(win){
		TransparentWnd* winHandler=(TransparentWnd*)win;
		rect.x = winHandler->x;
		rect.y = winHandler->y;
		rect.width = width;
		rect.height = height;
	}
    return true;
  }
bool MyHandler::GetScreenPoint(CefRefPtr<CefBrowser> browser,
                            int viewX,
                            int viewY,
                            int& screenX,
                            int& screenY)
{
	REQUIRE_UI_THREAD();

	// Convert the point from view coordinates to actual screen coordinates.
	POINT screen_pt = {viewX, viewY};
	if(win){
		TransparentWnd* winHandler=(TransparentWnd*)win;
		screen_pt.x+=winHandler->x;
		screen_pt.y+=winHandler->y;
	}	
	screenX = screen_pt.x;
	screenY = screen_pt.y;
	return true;
}

void MyHandler::SetSize(int width, int height)
{
  m_Browser->SetSize(PET_VIEW, width, height);
  this->width=width;
  this->height=height;
}

void MyHandler::OnPaint(CefRefPtr<CefBrowser> browser,
                       PaintElementType type,
                       const RectList& dirtyRects,
                       const void* buffer)
{
	if(win){
		TransparentWnd* winHandler=(TransparentWnd*)win;
		winHandler->Render(buffer);
	}
	return;
}
bool MyHandler::OnTooltip(CefRefPtr<CefBrowser> browser, CefString& text) {
	if(win){
		TransparentWnd* winHandler=(TransparentWnd *)win;
		winHandler->ShowTip(text);
		//winHandler->ExecJS(ss.str());
		return true;
	}
	return false;
}

void MyHandler::OnHttpResponse(const CefString& url, 
	const CefString& mimeType, 
	int responseCode, 
	int64 contentLength, 
	int64 startTime, 
	int64 endTime) {
	string s=url.ToString();
	if(s.find("download=true")!=string::npos){
		
	}
}

void MyHandler::OnResourceResponse(CefRefPtr<CefBrowser> browser,
                                  const CefString& url,
                                  CefRefPtr<CefResponse> response,
                                  CefRefPtr<CefContentFilter>& filter) {
}

bool MyHandler::OnBeforeResourceLoad(CefRefPtr<CefBrowser> browser,
                                     CefRefPtr<CefRequest> request,
                                     CefString& redirectUrl,
                                     CefRefPtr<CefStreamReader>& resourceStream,
                                     CefRefPtr<CefResponse> response,
                                     int loadFlags)
{
  REQUIRE_IO_THREAD();
  std::string url = request->GetURL();

  if(url == "http://tests/test") {
    // Show the uiapp contents
    //resourceStream = GetBinaryResourceReader(IDS_TEST);
    response->SetMimeType("text/html");
    response->SetStatus(200);
  } 
  if(url == "http://tests/test1") {
    // Show the uiapp contents
    //resourceStream = GetBinaryResourceReader(IDS_TEST1);
    response->SetMimeType("text/html");
    response->SetStatus(200);
  } 
  else if(strstr(url.c_str(), "/book512.png") != NULL) {
    // Any time we find "ps_logo2.png" in the URL substitute in our own image
    //resourceStream = GetBinaryResourceReader(IDS_BG);
    response->SetMimeType("image/png");
    response->SetStatus(200);
  }
  else if(url == "http://tests/uiapp") {
    resourceStream = GetBinaryResourceReader(IDS_UIPLUGIN);
    response->SetMimeType("text/html");
    response->SetStatus(200);
  } else if(url == "http://tests/osrapp") {
    // Show the osrapp contents
    resourceStream = GetBinaryResourceReader(IDS_OSRPLUGIN);
    response->SetMimeType("text/html");
    response->SetStatus(200);
  } else if(url == "http://tests/localstorage") {
    // Show the localstorage contents
    resourceStream = GetBinaryResourceReader(IDS_LOCALSTORAGE);
    response->SetMimeType("text/html");
    response->SetStatus(200);
  } else if(url == "http://tests/xmlhttprequest") {
    // Show the xmlhttprequest HTML contents
    resourceStream = GetBinaryResourceReader(IDS_XMLHTTPREQUEST);
    response->SetMimeType("text/html");
    response->SetStatus(200);
  } else if(url == "http://tests/domaccess") {
    // Show the domaccess HTML contents
    resourceStream = GetBinaryResourceReader(IDS_DOMACCESS);
    response->SetMimeType("text/html");
    response->SetStatus(200);
  } else if(strstr(url.c_str(), "/logoball.png") != NULL) {
    // Load the "logoball.png" image resource.
    resourceStream = GetBinaryResourceReader(IDS_LOGOBALL);
    response->SetMimeType("image/png");
    response->SetStatus(200);
  } else if(url == "http://tests/modalmain") {
    resourceStream = GetBinaryResourceReader(IDS_MODALMAIN);
    response->SetMimeType("text/html");
    response->SetStatus(200);
  } else if(url == "http://tests/modaldialog") {
    resourceStream = GetBinaryResourceReader(IDS_MODALDIALOG);
    response->SetMimeType("text/html");
    response->SetStatus(200);
  } else if(url == "http://tests/transparency") {
    resourceStream = GetBinaryResourceReader(IDS_TRANSPARENCY);
    response->SetMimeType("text/html");
    response->SetStatus(200);
  } else if(strstr(url.c_str(), ".app") != NULL){
    response->SetMimeType("text/html");
    response->SetStatus(200);
  } else if(url == "http://tests/plugin") {
    std::string html =
        "<html><body>\n"
        "Client Plugin loaded by Mime Type:<br>\n"
        "<embed type=\"application/x-client-plugin\" width=600 height=40>\n"
        "<br><br>Client Plugin loaded by File Extension:<br>\n"
        "<embed src=\"test.xcp\" width=600 height=40>\n"
        // Add some extra space below the plugin to allow scrolling.
        "<div style=\"height:1000px;\">&nbsp;</div>\n"
        "</body></html>";
  
    resourceStream =
        CefStreamReader::CreateForData((void*)html.c_str(), html.size());
    response->SetMimeType("text/html");
    response->SetStatus(200);
  }
  //以下为特殊请求
  {
	  CefRequest::HeaderMap hm;
	  request->GetHeaderMap(hm);
	  CefRequest::HeaderMap::iterator it=hm.find("Referers");
	  //CefString s("");
	  if(it!=hm.end()){
		  CefRequest::HeaderMap::iterator it2=hm.find("Referrer");
		  it2->second=it->second;
		  hm.erase(it);
	  }
	  request->SetHeaderMap(hm);
  }
  return false;
}

void MyHandler::SetWin(long win){
	this->win=win;
}

void MyHandler::NotifyDownloadBegin(const CefString& fileName, int64 contentLength)
{
  SetLastDownloadFile(fileName);
  //SendNotification(NOTIFY_DOWNLOAD_COMPLETE);
}

void MyHandler::NotifyDownloadComplete(const CefString& fileName)
{
  SetLastDownloadFile(fileName);
  SendNotification(NOTIFY_DOWNLOAD_COMPLETE);
}

void MyHandler::NotifyDownloadError(const CefString& fileName)
{
  SetLastDownloadFile(fileName);
  SendNotification(NOTIFY_DOWNLOAD_ERROR);
}

void MyHandler::SendNotification(NotificationType type)
{
  UINT id;
  switch(type)
  {
  case NOTIFY_CONSOLE_MESSAGE:
    id = ID_WARN_CONSOLEMESSAGE;
    break;
  case NOTIFY_DOWNLOAD_COMPLETE:
    id = ID_WARN_DOWNLOADCOMPLETE;
    break;
  case NOTIFY_DOWNLOAD_ERROR:
    id = ID_WARN_DOWNLOADERROR;
    break;
  default:
    return;
  }
  PostMessage(m_MainHwnd, WM_COMMAND, id, 0);
}

bool MyHandler::OnDragStart(CefRefPtr<CefBrowser> browser,
                                CefRefPtr<CefDragData> dragData,
                                DragOperationsMask mask)
{
  REQUIRE_UI_THREAD();

  // Forbid dragging of image files.
  if (dragData->IsFile()) {
    std::string fileExt = dragData->GetFileExtension();
    //if (fileExt == ".png" || fileExt == ".jpg" || fileExt == ".gif")
      //return true;
  }

  return false;
}

bool MyHandler::OnDragEnter(CefRefPtr<CefBrowser> browser,
                                CefRefPtr<CefDragData> dragData,
                                DragOperationsMask mask)
{
  REQUIRE_UI_THREAD();

  // Forbid dragging of link URLs.
  if (dragData->IsLink())
    return true;

  return false;
}


void MyHandler::OnCursorChange(CefRefPtr<CefBrowser> browser,CefCursorHandle cursor)
{
	REQUIRE_UI_THREAD();

	// Change the plugin window's cursor.
	if(win){
		SetClassLong(((TransparentWnd *)win)->renderWindow, GCL_HCURSOR,
			static_cast<LONG>(reinterpret_cast<LONG_PTR>(cursor)));
		SetCursor(cursor);
	}
}

void MyHandler::OnFocusedNodeChanged(CefRefPtr<CefBrowser> browser,
                                         CefRefPtr<CefFrame> frame,
                                         CefRefPtr<CefDOMNode> node)
{
  REQUIRE_UI_THREAD();

  // Set to true if a form element has focus.
  m_bFormElementHasFocus = (node.get() && node->IsFormControlElement());
}

// Execute with the specified argument list and return value.  Return true if
// the method was handled.
bool MyHandler::Execute(const CefString& name,
	CefRefPtr<CefV8Value> object,
	const CefV8ValueList& arguments,
	CefRefPtr<CefV8Value>& retval,
	CefString& exception)
{
	if(arguments.size() < 1)
		return false;

	TransparentWnd* winHandler=(TransparentWnd*)static_cast<long>(arguments[0]->GetIntValue());
	if(name == "close")
	{
		if(winHandler){
			if(winHandler->downloadHandler){
				//winHandler->downloadHandler->SetWinHandler(NULL);
				winHandler->downloadHandler=NULL;
			}
		}
		delete winHandler;
		return true;
	}
	else if(name == "drag")
	{
		winHandler->StartDrag();
		/*winHandler->Drag();*/
		return true;
	}
	else if(name == "enableDrag")
	{
		winHandler->EnableDrag();
		return true;
	}
	else if(name == "render")
	{
		winHandler->Render();
		return true;
	}
	else if(name == "stopDrag")
	{
		winHandler->isDrag=false;
		return true;
	}
	else if(name == "loadUrl")
	{
		CefString url = arguments[1]->GetStringValue();
		winHandler->SetUrl(url);
		return true;
	}
	else if(name == "setSize")
	{
		int w = static_cast<int>(arguments[1]->GetIntValue());
		int h = static_cast<int>(arguments[2]->GetIntValue());
		winHandler->SetSize(w, h);
		return true;
	}
	else if(name=="getSaveName"){
		CefString s = arguments[1]->GetStringValue();
		retval = CefV8Value::CreateString(winHandler->GetSaveName(s));
		return true;
	}
	else if(name=="getOpenName"){
		CefString s = arguments[1]->GetStringValue();
		retval = CefV8Value::CreateString(winHandler->GetOpenName(s));
		return true;
	}
	else if(name=="getOpenNames"){
		CefString s = arguments[1]->GetStringValue();
		retval = CefV8Value::CreateString(winHandler->GetOpenNames(s));
		return true;
	}
	else if(name=="getFileSize"){
		CefString s = arguments[1]->GetStringValue();
		retval = CefV8Value::CreateDouble(GetFileSize(s.ToWString().data()));
		return true;
	}
	else if(name=="getFolder"){
		retval = CefV8Value::CreateString(winHandler->GetFolder());
		return true;
	}
	else if(name == "move")
	{
		int x = static_cast<int>(arguments[1]->GetIntValue());
		int y = static_cast<int>(arguments[2]->GetIntValue());
		winHandler->Move(x, y);
		return true;
	}
	else if(name == "quit")
	{
		PostQuitMessage(0);
		return true;
	}
	else if(name == "max")
	{
		winHandler->Max();
		return true;
	}
	else if(name == "mini")
	{
		winHandler->Mini();
		return true;
	}
	else if(name == "hide")
	{
		winHandler->Hide();
		return true;
	}
	else if(name == "restore")
	{
		winHandler->Restore();
		return true;
	}
	else if(name == "setTopMost")
	{
		winHandler->SetTopMost();
		return true;
	}
	else if(name == "setWindowStyle")
	{
		UINT exStyle = static_cast<int>(arguments[1]->GetIntValue());
		winHandler->SetWindowStyle(exStyle);
		return true;
	}
	else if(name == "createWindow")
	{
		CefString url = arguments[1]->GetStringValue();
		UINT exStyle = static_cast<int>(arguments[2]->GetIntValue());
		TransparentWnd* win=new TransparentWnd();
		bool isTransparent = arguments[3]->GetBoolValue();
		CefString s = arguments[4]->GetStringValue();
		win->SetReadyHandler(s);
		HINSTANCE hInstance = GetModuleHandle(0);
		win->CreateBrowserWindow(url, exStyle, isTransparent);
		retval = CefV8Value::CreateInt((long)(win));
		//winHandler->CreateBrowser(url);
		return true;
	}
	else if(name == "createWindowBase")
	{
		CefString url = arguments[1]->GetStringValue();
		UINT exStyle = static_cast<int>(arguments[2]->GetIntValue());
		TransparentWnd* win=new TransparentWnd();
		bool isTransparent = arguments[3]->GetBoolValue();
		CefString s = arguments[4]->GetStringValue();
		win->SetReadyHandler(s);
		HINSTANCE hInstance = GetModuleHandle(0);
		win->CreateBrowserWindowBase(url, exStyle, isTransparent);
		retval = CefV8Value::CreateInt((long)(win));
		//winHandler->CreateBrowser(url);
		return true;
	}
	else if(name == "createBrowser")
	{
		CefString url = arguments[1]->GetStringValue();
		winHandler->CreateBrowser(url);
	}
	else if(name == "browse")
	{
		CefString url = arguments[1]->GetStringValue();
		winHandler->Browse(url);
	}
	else if(name == "toImage")
	{
		CefString path = arguments[1]->GetStringValue();
		winHandler->ToImage(path);
	}
	else if(name == "toImageEx")
	{
		CefString path = arguments[1]->GetStringValue();
		int x = static_cast<int>(arguments[2]->GetIntValue());
		int y = static_cast<int>(arguments[3]->GetIntValue());
		int width = static_cast<int>(arguments[4]->GetIntValue());
		int height = static_cast<int>(arguments[5]->GetIntValue());
		winHandler->ToImageEx(path,x,y,width,height);
	}
	else if(name == "bringToTop")
	{
		winHandler->BringToTop();
	}
	else if(name == "focus")
	{
		winHandler->Focus();
	}
	else if(name == "getPos")
	{
		retval = CefV8Value::CreateObject(NULL,NULL);
		// Add a string parameter to the new V8 object.
		retval->SetValue("x", CefV8Value::CreateInt(winHandler->x),V8_PROPERTY_ATTRIBUTE_NONE);
		// Add a function to the new V8 object.
		retval->SetValue("y", CefV8Value::CreateInt(winHandler->y),V8_PROPERTY_ATTRIBUTE_NONE);
		return true;
	}
	else if(name == "getScreenSize")
	{
		retval = CefV8Value::CreateObject(NULL,NULL);
		// Add a string parameter to the new V8 object.
		retval->SetValue("width", CefV8Value::CreateInt(GetSystemMetrics(SM_CXFULLSCREEN)),V8_PROPERTY_ATTRIBUTE_NONE);
		// Add a function to the new V8 object.
		retval->SetValue("height", CefV8Value::CreateInt(GetSystemMetrics(SM_CYFULLSCREEN)+GetSystemMetrics(SM_CYCAPTION)),V8_PROPERTY_ATTRIBUTE_NONE);
		return true;
	}
	else if(name == "readFile")
	{
		CefString s = arguments[1]->GetStringValue().ToString();
		retval = CefV8Value::CreateString(winHandler->ReadFile(s));
		// Add a string parameter to the new V8 object.
		return true;
	}
	else if(name == "writeFile")
	{
		CefString path = arguments[1]->GetStringValue().ToString();
		CefString s = arguments[2]->GetStringValue().ToString();
		retval = CefV8Value::CreateBool(winHandler->WriteFile(path,s));
		// Add a string parameter to the new V8 object.
		return true;
	}
	else if(name=="runApp"){
		CefString appName = arguments[1]->GetStringValue();
		CefString param = arguments[2]->GetStringValue();
		TransparentWnd* win=new TransparentWnd();
		win->RunAppIn(appName,param,winHandler->GetUrl());
		retval = CefV8Value::CreateInt((long)(win));
		return true;
	}
	else if(name=="runAppEx"){
		CefString appName = arguments[1]->GetStringValue();
		CefString param = arguments[2]->GetStringValue();
		winHandler->RunApp(appName,param,winHandler->GetUrl());
	}
	else if(name=="ready"){
		winHandler->Ready();
	}
	else if(name=="reload"){
		winHandler->Reload();
	}
	else if(name=="reloadIgnoreCache"){
		winHandler->ReloadIgnoreCache();
	}
	else if(name=="readStringEx"){
		int id = static_cast<int>(arguments[1]->GetIntValue());
		int l = static_cast<int>(arguments[2]->GetIntValue());
		CHAR *s1=new CHAR[l+1];
		AmfStream* pStream=winHandler->pStream;
		if(id){
			pStream=((AmfStream*)id);
		}
		pStream->ReadString(s1,NULL,l);
		CefString s(s1);
		retval=CefV8Value::CreateString(s);
		delete s1;
		return true;
	}
	else if(name=="writeStringEx"){
		int id = static_cast<int>(arguments[1]->GetIntValue());
		CefString s = arguments[2]->GetStringValue();
		AmfStream* pStream=winHandler->pStream;
		if(id){
			pStream=((AmfStream*)id);
		}
		pStream->WriteString(s.ToString().c_str());
	}
	else if(name=="readWStringEx"){
		int id = static_cast<int>(arguments[1]->GetIntValue());
		int l = static_cast<int>(arguments[2]->GetIntValue());
		TCHAR *s1=new TCHAR[l+1];
		AmfStream* pStream=winHandler->pStream;
		if(id){
			pStream=((AmfStream*)id);
		}
		pStream->ReadWString(s1,NULL,l*2);
		CefString s(s1);
		retval=CefV8Value::CreateString(s);
		delete s1;
		return true;
	}
	else if(name=="writeWStringEx"){
		int id = static_cast<int>(arguments[1]->GetIntValue());
		CefString s = arguments[2]->GetStringValue();
		AmfStream* pStream=winHandler->pStream;
		if(id){
			pStream=((AmfStream*)id);
		}
		pStream->WriteWString(s.ToWString().data());
	}
	else if(name=="readString"){
		int id = static_cast<int>(arguments[1]->GetIntValue());
		int l = static_cast<int>(arguments[2]->GetIntValue());
		CHAR *s1=new CHAR[l+1];
		AmfStream* pStream=winHandler->pStream;
		if(id){
			pStream=((AmfStream*)id);
		}
		pStream->ReadStringSimple(s1,l);
		CefString s(s1);
		retval=CefV8Value::CreateString(s);
		delete s1;
		return true;
	}
	else if(name=="writeString"){
		int id = static_cast<int>(arguments[1]->GetIntValue());
		CefString s = arguments[2]->GetStringValue();
		AmfStream* pStream=winHandler->pStream;
		if(id){
			pStream=((AmfStream*)id);
		}
		pStream->WriteStringSimple(s.ToString().c_str());
	}
	else if(name=="readWString"){
		int id = static_cast<int>(arguments[1]->GetIntValue());
		int l = static_cast<int>(arguments[2]->GetIntValue());
		TCHAR *s1=new TCHAR[l+1];
		AmfStream* pStream=winHandler->pStream;
		if(id){
			pStream=((AmfStream*)id);
		}
		pStream->ReadWStringSimple(s1,l*2);
		CefString s(s1);
		retval=CefV8Value::CreateString(s);
		delete s1;
		return true;
	}
	else if(name=="writeWString"){
		int id = static_cast<int>(arguments[1]->GetIntValue());
		CefString s = arguments[2]->GetStringValue();
		AmfStream* pStream=winHandler->pStream;
		if(id){
			pStream=((AmfStream*)id);
		}
		pStream->WriteWStringSimple(s.ToWString().data());
	}
	else if(name=="setStreamPos"){
		int id = static_cast<int>(arguments[1]->GetIntValue());
		AmfStream* pStream=winHandler->pStream;
		if(id){
			pStream=((AmfStream*)id);
		}
		int i = static_cast<int>(arguments[2]->GetIntValue());
		pStream->SetPosition(i);
	}
	else if(name=="getStreamPos"){
		int id = static_cast<int>(arguments[1]->GetIntValue());
		AmfStream* pStream=winHandler->pStream;
		if(id){
			pStream=((AmfStream*)id);
		}
		int b=pStream->GetPosition();
		retval=CefV8Value::CreateInt(b);
		return true;
	}
	else if(name=="readInt"){
		int id = static_cast<int>(arguments[1]->GetIntValue());
		AmfStream* pStream=winHandler->pStream;
		if(id){
			pStream=((AmfStream*)id);
		}
		int b=pStream->ReadULong();
		retval=CefV8Value::CreateInt(b);
		return true;
	}
	else if(name=="writeInt"){
		int id = static_cast<int>(arguments[1]->GetIntValue());
		int i = static_cast<int>(arguments[2]->GetIntValue());
		AmfStream* pStream=winHandler->pStream;
		if(id){
			pStream=((AmfStream*)id);
		}
		retval=CefV8Value::CreateBool(pStream->WriteULong(i)>0);
		return true;
	}
	else if(name=="readByte"){
		int id = static_cast<int>(arguments[1]->GetIntValue());
		AmfStream* pStream=winHandler->pStream;
		if(id){
			pStream=((AmfStream*)id);
		}
		BYTE b=pStream->ReadByte();
		retval=CefV8Value::CreateInt(b);
		return true;
	}
	else if(name=="writeByte"){
		int id = static_cast<int>(arguments[1]->GetIntValue());
		BYTE i = static_cast<BYTE>(arguments[2]->GetIntValue());
		AmfStream* pStream=winHandler->pStream;
		if(id){
			pStream=((AmfStream*)id);
		}
		retval=CefV8Value::CreateBool(pStream->WriteByte(i)>0);
		return true;
	}
	else if(name=="writeBytes"){
		int id = static_cast<int>(arguments[1]->GetIntValue());
		CefRefPtr<CefV8Value> value=arguments[2];
		if(value->IsArray()){
			int len=value->GetArrayLength();
			BYTE* p=new BYTE[len];
			for (int i = 0; i < len; ++i) {
				p[i]=value->GetValue(i)->GetIntValue();
			}
			AmfStream* pStream=winHandler->pStream;
			if(id){
				pStream=((AmfStream*)id);
			}
			retval=CefV8Value::CreateBool(pStream->WriteBytes(p,len)>0);
			delete []p;
		}
		else if (value->IsObject()) {
			int len=value->GetValue("length")->GetIntValue();
			BYTE* p=new BYTE[len];
			for (int i = 0; i < len; ++i) {
				p[i]=value->GetValue(i)->GetIntValue();
			}
			AmfStream* pStream=winHandler->pStream;
			if(id){
				pStream=((AmfStream*)id);
			}
			retval=CefV8Value::CreateBool(pStream->WriteBytes(p,len)>0);
			delete []p;
		}
		return true;
	}
	else if(name=="readBytes"){
		int id = static_cast<int>(arguments[1]->GetIntValue());
		int len=arguments[2]->GetIntValue();
		AmfStream* pStream=winHandler->pStream;
		if(id){
			pStream=((AmfStream*)id);
		}
		retval=CefV8Value::CreateArray();
		BYTE* p=new BYTE[len];
		pStream->ReadBytes(p,len);
		for (int i = 0; i < len; ++i) {
			retval->SetValue(i,CefV8Value::CreateInt(p[i]));	
		}
		delete []p;
		return true;
	}
	else if(name=="enableTransparent"){
		UINT exStyle = static_cast<int>(arguments[1]->GetIntValue());
		winHandler->EnableTransparent(exStyle);
	}
	else if(name == "getSize")
	{
		retval = CefV8Value::CreateObject(NULL,NULL);
		// Add a string parameter to the new V8 object.
		retval->SetValue("width", CefV8Value::CreateInt(winHandler->width),V8_PROPERTY_ATTRIBUTE_NONE);
		// Add a function to the new V8 object.
		retval->SetValue("height", CefV8Value::CreateInt(winHandler->height),V8_PROPERTY_ATTRIBUTE_NONE);
		return true;
	}
	else if(name == "setTitle")
	{
		CefString title = arguments[1]->GetStringValue();
		winHandler->SetTitle(title);
	}
	else if(name == "showDev"){
		CefBrowser* browser=winHandler->g_handler->GetBrowser();
		browser->ShowDevTools();
	}
	else if(name == "shutdown"){
		bool flag=arguments[1]->GetBoolValue();
		Shutdown(flag);
	}
	else if(name == "reboot"){
		bool flag=arguments[1]->GetBoolValue();
		Reboot(flag);
	}
	else if(name == "logoff"){
		bool flag=arguments[1]->GetBoolValue();
		Logoff(flag);
	}
	else if(name == "connect"){
		CefString ip=arguments[1]->GetStringValue();
		CefString uid=arguments[2]->GetStringValue();
		winHandler->p2p.Connect(ip.ToString().c_str(),uid.ToString().c_str());
	}
	else if(name == "connectByHost"){
		CefString hostName=arguments[1]->GetStringValue();
		CefString uid=arguments[2]->GetStringValue();
		winHandler->p2p.ConnectByHost(hostName.ToString().c_str(),uid.ToString().c_str());
	}
	else if(name == "getUsers"){
		winHandler->GetUsers();
	}
	else if(name == "sendMessage"){
		CefString userName=arguments[1]->GetStringValue();
		CefString msg=arguments[2]->GetStringValue();
		winHandler->p2p.SendMessageTo(userName.ToString().c_str(),msg.ToString().c_str());
	}
	else if(name == "sendMsgToServer"){
		CefString msg=arguments[1]->GetStringValue();
		winHandler->p2p.SendMessageToServer(msg.ToString().c_str());
	}
	else if(name == "sendMsgToIP"){
		CefString ip=arguments[1]->GetStringValue();
		short port=arguments[2]->GetIntValue();
		CefString msg=arguments[3]->GetStringValue();
		winHandler->p2p.SendMessageToEx(ip.ToString().c_str(),port,msg.ToString().c_str());
	}
	else if(name == "setTaskIcon"){
		int id = static_cast<int>(arguments[1]->GetIntValue());
		CefString path=arguments[2]->GetStringValue();
		CefString title=arguments[3]->GetStringValue();
		winHandler->SetTaskIcon(id,path,title);
	}
	else if(name == "findFiles"){
		CefString path=arguments[1]->GetStringValue();
		bool flag=arguments[2]->GetBoolValue();
		retval=CefV8Value::CreateString(find(path.ToWString(),flag));
		return true;
	}
	else if(name == "createMemory"){
		CefString name=arguments[1]->GetStringValue();
		CefString filename=arguments[2]->GetStringValue();
		int size = static_cast<int>(arguments[3]->GetIntValue());
		retval=CefV8Value::CreateInt((int)winHandler->CreateMemory(name,filename,size));
		return true;
	}
	else if(name == "deleteMemory"){
		int id = static_cast<int>(arguments[1]->GetIntValue());
		winHandler->DeleteMemory((CSFMServer*)id);
		return true;
	}
	else if(name == "createStream"){
		int id = static_cast<int>(arguments[1]->GetIntValue());
		retval=CefV8Value::CreateInt((int)winHandler->CreateStream((CSFMServer*)id));
		return true;
	}
	else if(name == "deleteStream"){
		int id = static_cast<int>(arguments[1]->GetIntValue());
		winHandler->DeleteStream((AmfStream*)id);
		return true;
	}
	else if(name == "delTaskIcon"){
		int id = static_cast<int>(arguments[1]->GetIntValue());
		winHandler->DelTaskIcon(id);
	}
	else if(name == "saveImageFromStream"){
		int id = static_cast<int>(arguments[1]->GetIntValue());
		int width = static_cast<int>(arguments[2]->GetIntValue());
		int height = static_cast<int>(arguments[3]->GetIntValue());
		CefString path=arguments[4]->GetStringValue();
		AmfStream* pStream=(AmfStream*)id;
		winHandler->SaveImageFromStream(path,pStream,width,height);
	}
	else if(name == "saveImageFromBase64"){
		CefString s=arguments[1]->GetStringValue();
		CefString path=arguments[2]->GetStringValue();
		string s1=s.ToString();
		int index=s1.find(',');
		s1=s1.substr(index+1);
		int imageSize = int((s1.length()/3)+1)*4;
		char* t=new char[imageSize];
		base64_decode(s1.c_str(),s1.length(), t, &imageSize); // using the base64
		CSFMServer *ps=winHandler->CreateMemory("test",path,imageSize);
		AmfStream *ps1=winHandler->CreateStream(ps);
		ps1->WriteBytes((PBYTE)t,imageSize);
		delete ps1;
		delete ps;
		delete t;
		/*Bitmap* pbm=GetImageFromBase64(s1);
		SaveBitmap(pbm,path);
		delete pbm;*/
	}
	else if(name == "getIPAndPort"){
		CefString ip=winHandler->p2p.IP;
		unsigned short port=winHandler->p2p.port;
		retval = CefV8Value::CreateObject(NULL,NULL);
		// Add a string parameter to the new V8 object.
		retval->SetValue("ip", CefV8Value::CreateString(ip),V8_PROPERTY_ATTRIBUTE_NONE);
		// Add a function to the new V8 object.
		retval->SetValue("port", CefV8Value::CreateInt(port),V8_PROPERTY_ATTRIBUTE_NONE);
		return true;
	}
	return false;
}

void InitCallback()
{
  // Register a V8 extension with the below JavaScript code that calls native
  // methods implemented in ClientV8ExtensionHandler.
  std::string code = "var webtop;"
    "if (!webtop)"
    "  webtop = {};"
    "(function() {"
    "  webtop.close = function(handler) {"
	"    handler=handler?handler:window['handler'];"
    "    native function close(handler);"
    "    return close(handler);"
    "  };"
    "  webtop.stopDrag = function(handler) {"
	"    handler=handler?handler:window['handler'];"
    "    native function stopDrag(handler);"
	"    return stopDrag(handler);"
    "  };"
    "  webtop.setSize = function(w,h,handler) {"
	"    handler=handler?handler:window['handler'];"
    "    native function setSize(handler,w,h);"
	"    return setSize(handler,w,h);"
    "  };"
    "  webtop.move = function(x,y,handler) {"
	"    handler=handler?handler:window['handler'];"
    "    native function move(handler,x,y);"
	"    return move(handler,x,y);"
    "  };"
    "  webtop.max = function(handler) {"
	"    handler=handler?handler:window['handler'];"
    "    native function max(handler);"
	"    return max(handler);"
    "  };"
    "  webtop.hide = function(handler) {"
	"    handler=handler?handler:window['handler'];"
    "    native function hide(handler);"
	"    return hide(handler);"
    "  };"
    "  webtop.mini = function(handler) {"
	"    handler=handler?handler:window['handler'];"
    "    native function mini(handler);"
	"    return mini(handler);"
    "  };"
    "  webtop.restore = function(handler) {"
	"    handler=handler?handler:window['handler'];"
    "    native function restore(handler);"
	"    return restore(handler);"
    "  };"
    "  webtop.drag = function(handler) {"
	"    handler=handler?handler:window['handler'];"
    "    native function drag(handler);"
	"    return drag(handler);"
    "  };"
    "  webtop.render = function(handler) {"
	"    handler=handler?handler:window['handler'];"
    "    native function render(handler);"
	"    return render(handler);"
    "  };"
    "  webtop.bringToTop = function(handler) {"
	"    handler=handler?handler:window['handler'];"
    "    native function bringToTop(handler);"
	"    return bringToTop(handler);"
    "  };"
    "  webtop.focus = function(handler) {"
	"    handler=handler?handler:window['handler'];"
    "    native function focus(handler);"
	"    return focus(handler);"
    "  };"
    "  webtop.loadUrl = function(url,handler) {"
	"    handler=handler?handler:window['handler'];"
    "    native function loadUrl(handler,url);"
	"    return loadUrl(handler,url);"
    "  };"
    "  webtop.getPos = function(handler) {"
	"    handler=handler?handler:window['handler'];"
    "    native function getPos(handler);"
	"    return getPos(handler);"
    "  };"
    "  webtop.getSize = function(handler) {"
	"    handler=handler?handler:window['handler'];"
    "    native function getSize(handler);"
	"    return getSize(handler);"
    "  };"
    "  webtop.enableDrag = function(handler) {"
	"    handler=handler?handler:window['handler'];"
    "    native function enableDrag(handler);"
	"    return enableDrag(handler);"
    "  };"
    "  webtop.quit = function(handler) {"
	"    handler=handler?handler:window['handler'];"
    "    native function quit(handler);"
	"    return quit(handler);"
    "  };"
    "  webtop.ready = function(handler) {"
	"    handler=handler?handler:window['handler'];"
    "    native function ready(handler);"
	"    return ready(handler);"
    "  };"
    "  webtop.createWindow = function(url,exStyle,isTransparent,readyHandler,handler) {"
	"    handler=handler?handler:window['handler'];"
	"    readyHandler=readyHandler||'';"
    "    native function createWindow(handler,url,exStyle,isTransparent,readyHandler);"
	"    return createWindow(handler,url,exStyle,isTransparent,readyHandler);"
    "  };"
    "  webtop.createWindowBase = function(url,exStyle,isTransparent,readyHandler,handler) {"
	"    handler=handler?handler:window['handler'];"
	"    readyHandler=readyHandler||'';"
    "    native function createWindowBase(handler,url,exStyle,isTransparent,readyHandler);"
	"    return createWindowBase(handler,url,exStyle,isTransparent,readyHandler);"
    "  };"
    "  webtop.createBrowser = function(url,handler) {"
	"    handler=handler?handler:window['handler'];"
    "    native function createBrowser(handler,url);"
	"    return createBrowser(handler,url);"
    "  };"
    "  webtop.browse = function(url,handler) {"
	"    handler=handler?handler:window['handler'];"
    "    native function browse(handler,url);"
	"    return browse(handler,url);"
    "  };"
    "  webtop.setTitle = function(title,handler) {"
	"    handler=handler?handler:window['handler'];"
    "    native function setTitle(handler,title);"
	"    return setTitle(handler,title);"
    "  };"
    "  webtop.getImage = function(handler) {"
	"    handler=handler?handler:window['handler'];"
    "    native function getImage(handler);"
	"    return getImage(handler);"
    "  };"
    "  webtop.showDev = function(handler) {"
	"    handler=handler?handler:window['handler'];"
    "    native function showDev(handler);"
	"    return showDev(handler);"
    "  };"
    "  webtop.readFile = function(path,handler) {"
	"    handler=handler?handler:window['handler'];"
    "    native function readFile(handler,path);"
	"    return readFile(handler,path);"
    "  };"
    "  webtop.writeFile = function(path,s,handler) {"
	"    handler=handler?handler:window['handler'];"
    "    native function writeFile(handler,path,s);"
	"    return writeFile(handler,path,s);"
    "  };"
    "  webtop.getSaveName = function(filename,handler) {"
	"    handler=handler?handler:window['handler'];"
    "    native function getSaveName(handler,filename);"
	"    return getSaveName(handler,filename);"
    "  };"
    "  webtop.runApp = function(appName,param,handler) {"
	"    handler=handler?handler:window['handler'];"
	"	 param=param?param:'';"
    "    native function runApp(handler,appName,param);"
	"    return runApp(handler,appName,param);"
    "  };"
    "  webtop.runAppEx = function(appName,param,handler) {"
	"    handler=handler?handler:window['handler'];"
    "    native function runAppEx(handler,appName,param);"
	"    return runAppEx(handler,appName,param);"
    "  };"
    "  webtop.getOpenName = function(filename,handler) {"
	"    handler=handler?handler:window['handler'];"
    "    native function getOpenName(handler,filename);"
	"    return getOpenName(handler,filename);"
    "  };"
    "  webtop.getOpenNames = function(filename,handler) {"
	"    handler=handler?handler:window['handler'];"
    "    native function getOpenNames(handler,filename);"
	"    return JSON.parse(getOpenNames(handler,filename).replace(/\\\\/g,'\\\\\\\\'));"
    "  };"
    "  webtop.getFileSize = function(filename,handler) {"
	"    handler=handler?handler:window['handler'];"
    "    native function getFileSize(handler,filename);"
	"    return getFileSize(handler,filename);"
    "  };"
    "  webtop.getFolder = function(handler) {"
	"    handler=handler?handler:window['handler'];"
    "    native function getFolder(handler);"
	"    return getFolder(handler);"
    "  };"
    "  webtop.reload = function(handler) {"
	"    handler=handler?handler:window['handler'];"
    "    native function reload(handler);"
	"    return reload(handler);"
    "  };"
    "  webtop.reloadIgnoreCache = function(handler) {"
	"    handler=handler?handler:window['handler'];"
    "    native function reloadIgnoreCache(handler);"
	"    return reloadIgnoreCache(handler);"
    "  };"
    "  webtop.setTopMost = function(handler) {"
	"    handler=handler?handler:window['handler'];"
    "    native function setTopMost(handler);"
	"    return setTopMost(handler);"
    "  };"
    "  webtop.setWindowStyle = function(exStyle,handler) {"
	"    handler=handler?handler:window['handler'];"
    "    native function setWindowStyle(handler,exStyle);"
	"    return setWindowStyle(handler,exStyle);"
    "  };"
    "  webtop.getSharePos = function(handler) {"
	"    handler=handler?handler:window['handler'];"
    "    native function getSharePos(handler);"
	"    return getSharePos(handler);"
    "  };"
    "  webtop.setSharePos = function(i) {"
	"    handler=handler?handler:window['handler'];"
    "    native function setSharePos(handler,i);"
	"    return setSharePos(handler,i);"
    "  };"
    "  webtop.toImageEx = function(path,x,y,width,height,handler) {"
	"    handler=handler?handler:window['handler'];"
    "    native function toImageEx(handler,path,x,y,width,height);"
	"    return toImageEx(handler,path,x,y,width,height);"
    "  };"
    "  webtop.toImage = function(path,handler) {"
	"    handler=handler?handler:window['handler'];"
    "    native function toImage(handler,path);"
	"    return toImage(handler,path);"
    "  };"
    "  webtop.shutdown = function(flag,handler) {"
	"    handler=handler?handler:window['handler'];"
    "    native function shutdown(handler,flag);"
	"    return shutdown(handler,flag);"
    "  };"
    "  webtop.reboot = function(flag,handler) {"
	"    handler=handler?handler:window['handler'];"
    "    native function reboot(handler,flag);"
	"    return reboot(handler,flag);"
    "  };"
    "  webtop.logoff = function(flag,handler) {"
	"    handler=handler?handler:window['handler'];"
    "    native function logoff(handler,flag);"
	"    return logoff(handler,flag);"
    "  };"
    "  webtop.connect = function(ip,uid,handler) {"
	"    handler=handler?handler:window['handler'];"
    "    native function connect(handler,ip,uid);"
	"    return connect(handler,ip,uid);"
    "  };"
    "  webtop.connectByHost = function(hostName,uid,handler) {"
	"    handler=handler?handler:window['handler'];"
    "    native function connectByHost(handler,hostName,uid);"
	"    return connectByHost(handler,hostName,uid);"
    "  };"
    "  webtop.getUsers = function(handler) {"
	"    handler=handler?handler:window['handler'];"
    "    native function getUsers(handler);"
	"    return getUsers(handler);"
    "  };"
    "  webtop.sendMessage = function(userName,msg,handler) {"
	"    handler=handler?handler:window['handler'];"
    "    native function sendMessage(handler,userName,msg);"
	"    return sendMessage(handler,userName,msg);"
    "  };"
    "  webtop.sendMsgToIP = function(ip,port,msg,handler) {"
	"    handler=handler?handler:window['handler'];"
    "    native function sendMsgToIP(handler,ip,port,msg);"
	"    return sendMsgToIP(handler,ip,port,msg);"
    "  };"
    "  webtop.sendMsgToServer = function(msg,handler) {"
	"    handler=handler?handler:window['handler'];"
    "    native function sendMsgToServer(handler,msg);"
	"    return sendMsgToServer(handler,msg);"
    "  };"
    "  webtop.getIPAndPort = function(handler) {"
	"    handler=handler?handler:window['handler'];"
    "    native function getIPAndPort(handler);"
	"    return getIPAndPort(handler);"
    "  };"
    "  webtop.setTaskIcon = function(id,path,title,handler) {"
	"    handler=handler?handler:window['handler'];"
    "    native function setTaskIcon(handler,id,path,title);"
	"    return setTaskIcon(handler,id,path,title);"
    "  };"
    "  webtop.delTaskIcon = function(id,handler) {"
	"    handler=handler?handler:window['handler'];"
    "    native function delTaskIcon(handler,id);"
	"    return delTaskIcon(handler,id);"
    "  };"
    "  webtop.findFiles = function(path,flag,handler) {"
	"    handler=handler?handler:window['handler'];"
    "    native function findFiles(handler,path,flag);"
	"    return JSON.parse(findFiles(handler,path,flag));"
    "  };"
    "  webtop.createMemory = function(name,filename,size,handler) {"
	"    handler=handler?handler:window['handler'];"
	"	 filename=filename?filename:'';"
    "    native function createMemory(handler,name,filename,size);"
	"    return createMemory(handler,name,filename,size);"
    "  };"
    "  webtop.deleteMemory = function(id,handler) {"
	"    handler=handler?handler:window['handler'];"
    "    native function deleteMemory(handler,id);"
	"    return deleteMemory(handler,id);"
    "  };"
    "  webtop.createStream = function(id,handler) {"
	"    handler=handler?handler:window['handler'];"
    "    native function createStream(handler,id);"
	"    return createStream(handler,id);"
    "  };"
    "  webtop.deleteStream = function(id,handler) {"
	"    handler=handler?handler:window['handler'];"
    "    native function deleteStream(handler,id);"
	"    return deleteStream(handler,id);"
    "  };"
    "  webtop.readString = function(l,id,handler) {"
	"    handler=handler?handler:window['handler'];"
    "    native function readString(handler,id,l);"
	"    return readString(handler,id,l);"
    "  };"
    "  webtop.writeString = function(s,id,handler) {"
	"    handler=handler?handler:window['handler'];"
    "    native function writeString(handler,id,s);"
	"    return writeString(handler,id,s);"
    "  };"
    "  webtop.readWString = function(l,id,handler) {"
	"    handler=handler?handler:window['handler'];"
    "    native function readWString(handler,id,l);"
	"    return readWString(handler,id,l);"
    "  };"
    "  webtop.writeWString = function(s,id,handler) {"
	"    handler=handler?handler:window['handler'];"
    "    native function writeWString(handler,id,s);"
	"    return writeWString(handler,id,s);"
    "  };"
    "  webtop.readStringEx = function(l,id,handler) {"
	"    handler=handler?handler:window['handler'];"
    "    native function readStringEx(handler,id,l);"
	"    return readStringEx(handler,id,l);"
    "  };"
    "  webtop.writeStringEx = function(s,id,handler) {"
	"    handler=handler?handler:window['handler'];"
    "    native function writeStringEx(handler,id,s);"
	"    return writeStringEx(handler,id,s);"
    "  };"
    "  webtop.readWStringEx = function(l,id,handler) {"
	"    handler=handler?handler:window['handler'];"
    "    native function readWStringEx(handler,id,l);"
	"    return readWStringEx(handler,id,l);"
    "  };"
    "  webtop.writeWStringEx = function(s,id,handler) {"
	"    handler=handler?handler:window['handler'];"
    "    native function writeWStringEx(handler,id,s);"
	"    return writeWStringEx(handler,id,s);"
    "  };"
    "  webtop.readInt = function(id,handler) {"
	"    handler=handler?handler:window['handler'];"
    "    native function readInt(handler,id);"
	"    return readInt(handler,id);"
    "  };"
    "  webtop.writeInt = function(i,id,handler) {"
	"    handler=handler?handler:window['handler'];"
    "    native function writeInt(handler,id,i);"
	"    return writeInt(handler,id,i);"
    "  };"
    "  webtop.readByte = function(id,handler) {"
	"    handler=handler?handler:window['handler'];"
    "    native function readByte(handler,id);"
	"    return readByte(handler,id);"
    "  };"
    "  webtop.writeByte = function(b,id,handler) {"
	"    handler=handler?handler:window['handler'];"
    "    native function writeByte(handler,id,b);"
	"    return writeByte(handler,id,b);"
    "  };"
    "  webtop.readBytes = function(l,id,handler) {"
	"    handler=handler?handler:window['handler'];"
    "    native function readBytes(handler,id,l);"
	"    return readBytes(handler,id,l);"
    "  };"
    "  webtop.writeBytes = function(arr,id,handler) {"
	"    handler=handler?handler:window['handler'];"
    "    native function writeBytes(handler,id,arr);"
	"    return writeBytes(handler,id,arr);"
    "  };"
    "  webtop.setStreamPos = function(pos,id,handler) {"
	"    handler=handler?handler:window['handler'];"
    "    native function setStreamPos(handler,id,pos);"
	"    return setStreamPos(handler,id,pos);"
    "  };"
    "  webtop.getStreamPos = function(id,handler) {"
	"    handler=handler?handler:window['handler'];"
    "    native function getStreamPos(handler,id);"
	"    return getStreamPos(handler,id);"
    "  };"
    "  webtop.getStreamPos = function(id,handler) {"
	"    handler=handler?handler:window['handler'];"
    "    native function getStreamPos(handler,id);"
	"    return getStreamPos(handler,id);"
    "  };"
    "  webtop.saveImageFromStream = function(id,width,height,path,handler) {"
	"    handler=handler?handler:window['handler'];"
    "    native function saveImageFromStream(handler,id,width,height,path);"
	"    return saveImageFromStream(handler,id,width,height,path);"
    "  };"
    "  webtop.saveImageFromBase64 = function(s,path,handler) {"
	"    handler=handler?handler:window['handler'];"
    "    native function saveImageFromBase64(handler,s,path);"
	"    return saveImageFromBase64(handler,s,path);"
    "  };"
    "  webtop.getScreenSize = function(handler) {"
	"    handler=handler?handler:window['handler'];"
    "    native function getScreenSize(handler);"
	"    return getScreenSize(handler);"
    "  };"
	"})();";
	CefRegisterExtension("callback/test", code, new MyHandler());
}
