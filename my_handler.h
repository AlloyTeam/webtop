#ifndef _MY_HANDLER_H
#define _MY_HANDLER_H
#include "cefclient/client_handler.h"

class MyHandler : public ClientHandler,public CefRenderHandler, public CefV8Handler,public CefMenuHandler
{
public:
  typedef cef_paint_element_type_t PaintElementType;
  typedef std::vector<CefRect> RectList;
  long win;
  int width;
  int height;
  int contentLength;
  MyHandler();
  virtual ~MyHandler();
  // CefLoadHandler methods
  virtual void OnLoadStart(CefRefPtr<CefBrowser> browser,
                           CefRefPtr<CefFrame> frame) OVERRIDE;
  virtual void OnLoadEnd(CefRefPtr<CefBrowser> browser,
                         CefRefPtr<CefFrame> frame,
                         int httpStatusCode) OVERRIDE;
  virtual bool OnLoadError(CefRefPtr<CefBrowser> browser,
                           CefRefPtr<CefFrame> frame,
                           ErrorCode errorCode,
                           const CefString& failedUrl,
                           CefString& errorText) OVERRIDE;
  virtual void OnPaint(CefRefPtr<CefBrowser> browser,
                       PaintElementType type,
                       const RectList& dirtyRects,
                       const void* buffer) OVERRIDE;
  virtual void OnResourceResponse(CefRefPtr<CefBrowser> browser,
                                  const CefString& url,
                                  CefRefPtr<CefResponse> response,
                                  CefRefPtr<CefContentFilter>& filter) OVERRIDE;
  bool OnBeforeResourceLoad(CefRefPtr<CefBrowser> browser,
										 CefRefPtr<CefRequest> request,
										 CefString& redirectUrl,
										 CefRefPtr<CefStreamReader>& resourceStream,
										 CefRefPtr<CefResponse> response,
										 int loadFlags) OVERRIDE;
  virtual CefRefPtr<CefRenderHandler> GetRenderHandler() OVERRIDE;
  virtual CefRefPtr<CefDisplayHandler> GetDisplayHandler() OVERRIDE;
  virtual bool OnBeforePopup(CefRefPtr<CefBrowser> parentBrowser,
								const CefPopupFeatures& popupFeatures,
								CefWindowInfo& windowInfo,
								const CefString& url,
								CefRefPtr<CefClient>& client,
								CefBrowserSettings& settings,
							 CefRefPtr<CefBrowser>& newBrowser) OVERRIDE;
  virtual void OnAfterCreated(CefRefPtr<CefBrowser> browser) OVERRIDE;
  void SetSize(int width, int height);
  void SetWin(long win);
  virtual void OnCursorChange(CefRefPtr<CefBrowser> browser,
								CefCursorHandle cursor) OVERRIDE;
  // CefRenderHandler methods
  bool OnKeyEvent(CefRefPtr<CefBrowser> browser,
                               KeyEventType type,
                               int code,
                               int modifiers,
                               bool isSystemKey,
                               bool isAfterJavaScript);
  virtual bool GetViewRect(CefRefPtr<CefBrowser> browser,
                           CefRect& rect) OVERRIDE;

  virtual bool GetScreenRect(CefRefPtr<CefBrowser> browser,
                             CefRect& rect) OVERRIDE
  {
    return GetViewRect(browser, rect);
  }
  virtual void OnFocusedNodeChanged(CefRefPtr<CefBrowser> browser,
                                    CefRefPtr<CefFrame> frame,
                                    CefRefPtr<CefDOMNode> node) OVERRIDE;
  virtual CefRefPtr<CefDragHandler> GetDragHandler() OVERRIDE
      { return this; }


  virtual bool OnDragStart(CefRefPtr<CefBrowser> browser,
                           CefRefPtr<CefDragData> dragData,
                           DragOperationsMask mask) OVERRIDE;
  virtual bool OnDragEnter(CefRefPtr<CefBrowser> browser,
                           CefRefPtr<CefDragData> dragData,
                           DragOperationsMask mask) OVERRIDE;

  // DownloadListener methods
  virtual void NotifyDownloadBegin(const CefString& fileName, int64 contentLength);
  virtual void NotifyDownloadComplete(const CefString& fileName) OVERRIDE;
  virtual void NotifyDownloadError(const CefString& fileName) OVERRIDE;
  virtual void* GetWin();
  virtual void OnHttpResponse(const CefString& url, 
	  const CefString& mimeType, 
	  int responseCode, 
	  int64 contentLength, 
	  int64 startTime, 
	  int64 endTime);
  virtual bool GetDownloadHandler(CefRefPtr<CefBrowser> browser,
                                  const CefString& mimeType,
                                  const CefString& fileName,
                                  int64 contentLength,
                                  CefRefPtr<CefDownloadHandler>& handler)
                                  OVERRIDE;
  CefRefPtr<CefMenuHandler> GetMenuHandler();
  virtual bool OnTooltip(CefRefPtr<CefBrowser> browser,
                         CefString& text) OVERRIDE;

  virtual bool Execute(const CefString& name,
                       CefRefPtr<CefV8Value> object,
                       const CefV8ValueList& arguments,
                       CefRefPtr<CefV8Value>& retval,
                       CefString& exception);
  virtual bool OnBeforeMenu(CefRefPtr<CefBrowser> browser,
	  const CefMenuInfo& menuInfo);

  enum NotificationType
  {
    NOTIFY_CONSOLE_MESSAGE,
    NOTIFY_DOWNLOAD_COMPLETE,
    NOTIFY_DOWNLOAD_ERROR,
  };
  void SendNotification(NotificationType type);

  virtual bool GetScreenPoint(CefRefPtr<CefBrowser> browser,
                              int viewX,
                              int viewY,
                              int& screenX,
                              int& screenY) OVERRIDE;
  // Include the default reference counting implementation.
  IMPLEMENT_REFCOUNTING(MyHandler);
  // Include the default locking implementation.
  IMPLEMENT_LOCKING(MyHandler);
};


#endif