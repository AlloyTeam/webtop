// Copyright (c) 2011 The Chromium Embedded Framework Authors. All rights
// reserved. Use of this source code is governed by a BSD-style license that
// can be found in the LICENSE file.

#include "res_scheme.h"
#include <algorithm>
#include <string>
#include "include/cef_browser.h"
#include "include/cef_frame.h"
#include "include/cef_response.h"
#include "include/cef_request.h"
#include "include/cef_scheme.h"
#include "cefclient/resource_util.h"
#include "cefclient/string_util.h"
#include "cefclient/util.h"
#include "system.h"

#if defined(OS_WIN)
#include "cefclient/resource.h"
#endif


// Implementation of the schema handler for client:// requests.
class ResSchemeHandler : public CefSchemeHandler {
 public:
  ResSchemeHandler() : offset_(0) {}

  virtual bool ProcessRequest(CefRefPtr<CefRequest> request,
                              CefRefPtr<CefSchemeHandlerCallback> callback)
                              OVERRIDE {
    REQUIRE_IO_THREAD();
	extern HINSTANCE hInst;

    bool handled = false;

    AutoLock lock_scope(this);

    std::string url = request->GetURL();
	std::string res=url.substr(6);
	std::string modulePath="";
	string name="";
	int index=res.find_last_of("/");
	HMODULE hModule=hInst;
	if(index!=-1&&index!=(int)res.length()-1){
		modulePath=res.substr(0,index);
		name=res.substr(index+1);
		hModule=GetModuleHandle(CefString(modulePath).ToWString().data());
	}
	else{
		name=res.substr(0,index);
	}
	std::string ext=GetExt(name);
      // Load the response image
#if defined(OS_WIN)
    DWORD dwSize;
    LPBYTE pBytes;
	if (LoadBinaryResourceByName(CefString(name).ToWString().data(), dwSize, pBytes, hModule)) {
		data_ = std::string(reinterpret_cast<const char*>(pBytes), dwSize);
		handled = true;
		// Set the resulting mime type
		if(ext=="html"||ext=="js"||ext=="css"){
			if(ext=="js"){
				mime_type_ = "text/javascript";
			}
			else{
				mime_type_ = "text/"+ext;
			}
		}
		else{
			mime_type_ = "image/"+ext;
		}
    }
#elif defined(OS_MACOSX) || defined(OS_LINUX)
    if (LoadBinaryResource("logo.png", data_)) {
    handled = true;
    // Set the resulting mime type
    mime_type_ = "image/png";
    }
#else
#error "Unsupported platform"
#endif

    if (handled) {
      // Indicate the headers are available.
      callback->HeadersAvailable();
      return true;
    }

    return false;
  }

  virtual void GetResponseHeaders(CefRefPtr<CefResponse> response,
                                  int64& response_length,
                                  CefString& redirectUrl) OVERRIDE {
    REQUIRE_IO_THREAD();

    ASSERT(!data_.empty());

    response->SetMimeType(mime_type_);
    response->SetStatus(200);

    // Set the resulting response length
    response_length = data_.length();
  }

  virtual void Cancel() OVERRIDE {
    REQUIRE_IO_THREAD();
  }

  virtual bool ReadResponse(void* data_out,
                            int bytes_to_read,
                            int& bytes_read,
                            CefRefPtr<CefSchemeHandlerCallback> callback)
                            OVERRIDE {
    REQUIRE_IO_THREAD();

    bool has_data = false;
    bytes_read = 0;

    AutoLock lock_scope(this);

    if (offset_ < data_.length()) {
      // Copy the next block of data into the buffer.
      int transfer_size =
          std::min(bytes_to_read, static_cast<int>(data_.length() - offset_));
      memcpy(data_out, data_.c_str() + offset_, transfer_size);
      offset_ += transfer_size;

      bytes_read = transfer_size;
      has_data = true;
    }

    return has_data;
  }

 private:
  std::string data_;
  std::string mime_type_;
  size_t offset_;

  IMPLEMENT_REFCOUNTING(ResSchemeHandler);
  IMPLEMENT_LOCKING(ResSchemeHandler);
};

// Implementation of the factory for for creating schema handlers.
class ResSchemeHandlerFactory : public CefSchemeHandlerFactory {
 public:
  // Return a new scheme handler instance to handle the request.
  virtual CefRefPtr<CefSchemeHandler> Create(CefRefPtr<CefBrowser> browser,
                                             const CefString& scheme_name,
                                             CefRefPtr<CefRequest> request)
                                             OVERRIDE {
    REQUIRE_IO_THREAD();
    return new ResSchemeHandler();
  }

  IMPLEMENT_REFCOUNTING(ClientSchemeHandlerFactory);
};

void InitResScheme() {
  CefRegisterCustomScheme("res", true, false, false);
  CefRegisterSchemeHandlerFactory("res", "",
      new ResSchemeHandlerFactory());
}

void RunResScheme(CefRefPtr<CefBrowser> browser) {
  browser->GetMainFrame()->LoadURL("res://AlloyDesktop/logo.png");
}
