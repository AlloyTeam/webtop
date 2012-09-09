// Copyright (c) 2010 The Chromium Embedded Framework Authors. All rights
// reserved. Use of this source code is governed by a BSD-style license that
// can be found in the LICENSE file.

#ifndef _CEFCLIENT_DOWNLOAD_HANDLER_H
#define _CEFCLIENT_DOWNLOAD_HANDLER_H

#include "include/cef_app.h"
#include "include/cef_browser.h"
#include "include/cef_frame.h"
#include "include/cef_runnable.h"
#include "include/cef_download_handler.h"

// Implement this interface to receive download notifications.
class DownloadListener : public virtual CefBase
{
public:
  // Called when the download is complete.
  virtual void NotifyDownloadComplete(const CefString& fileName) =0;
  virtual void NotifyDownloadBegin(const CefString& fileName, int64 contentLength)=0;
  virtual void* GetWin()=0;
  // Called if the download fails.
  virtual void NotifyDownloadError(const CefString& fileName) =0;
};

// Create a new download handler to manage download of a single file.
CefRefPtr<CefDownloadHandler> CreateDownloadHandler(
    CefRefPtr<DownloadListener> listener, const CefString& fileName, int64 contentLength);

#endif // _CEFCLIENT_DOWNLOAD_HANDLER_H
