// Copyright (c) 2010 The Chromium Embedded Framework Authors. All rights
// reserved. Use of this source code is governed by a BSD-style license that
// can be found in the LICENSE file.

#include "include/cef_runnable.h"
#include "download_handler.h"
#include "util.h"
#include <sstream>
#include <stdio.h>
#include <vector>
#include "../transparent_wnd.h"

#if defined(OS_WIN)
#include <windows.h>
#include <shlobj.h>
#include <shlwapi.h>
#include <tchar.h>
#include <locale.h>
#include <CommDlg.h>
#endif // OS_WIN
#pragma once

// Implementation of the CefDownloadHandler interface.
class ClientDownloadHandler : public CefDownloadHandler
{
public:
	int64 contentLength_;
  ClientDownloadHandler(CefRefPtr<DownloadListener> listener,
                        const CefString& fileName, int64 contentLength)
    : listener_(listener), filename_(fileName), file_(NULL)
  {
	  contentLength_=contentLength;
  }

  ~ClientDownloadHandler()
  {
    ASSERT(pending_data_.empty());
    ASSERT(file_ == NULL);
    
    if(!pending_data_.empty()) {
      // Delete remaining pending data.
      std::vector<std::vector<char>*>::iterator it = pending_data_.begin();
      for(; it != pending_data_.end(); ++it)
        delete (*it);
    }
    
    if(file_) {
      // Close the dangling file pointer on the FILE thread.
      CefPostTask(TID_FILE,
          NewCefRunnableFunction(&ClientDownloadHandler::CloseDanglingFile,
                                 file_));
      
      // Notify the listener that the download failed.
      listener_->NotifyDownloadError(filename_);
    }
  }

  // --------------------------------------------------
  // The following methods are called on the UI thread.
  // --------------------------------------------------

  void Initialize()
  {
    // Open the file on the FILE thread.
	  winHandler=new TransparentWnd();
	  winHandler->downloadHandler=this;
	  winHandler->CreateBrowserWindowBase(L"browser/download.html");
	  byteCount=0;
	  CefPostTask(TID_FILE,
        NewCefRunnableMethod(this, &ClientDownloadHandler::OnOpen));
      listener_->NotifyDownloadBegin(filename_, contentLength_);
  }

  // A portion of the file contents have been received. This method will be
  // called multiple times until the download is complete. Return |true| to
  // continue receiving data and |false| to cancel.
  virtual bool ReceivedData(void* data, int data_size)
  {
    REQUIRE_UI_THREAD();
	if(!winHandler){
		return false;
	}

    if(data_size == 0)
      return true;

    // Create a new vector for the data.
    std::vector<char>* buffer = new std::vector<char>(data_size);
    memcpy(&(*buffer)[0], data, data_size);

    // Add the new data vector to the pending data queue.
    {
      AutoLock lock_scope(this);
      pending_data_.push_back(buffer);
    }

    // Write data to file on the FILE thread.
    CefPostTask(TID_FILE,
        NewCefRunnableMethod(this, &ClientDownloadHandler::OnReceivedData));
    return true;
  }

  virtual void SetWinHandler(void* handler){
	  winHandler=(TransparentWnd *)handler;
  }
  // The download is complete.
  virtual void Complete()
  {
    REQUIRE_UI_THREAD();

    // Flush and close the file on the FILE thread.
    CefPostTask(TID_FILE,
        NewCefRunnableMethod(this, &ClientDownloadHandler::OnComplete));
  }

  // ----------------------------------------------------
  // The following methods are called on the FILE thread.
  // ----------------------------------------------------
	TCHAR szFile[2048];

	BOOL  OpenFileDialog(HWND hWnd, const TCHAR* fileNameStr)
	{
		//TCHAR szFile[2048];
		OPENFILENAME ofn;
		ZeroMemory(&ofn, sizeof(ofn));
		ofn.lStructSize = sizeof(ofn);
	// must !
		ofn.lpstrFile = szFile;
		ofn.lpstrTitle = TEXT("±£´æÎÄ¼þ");
		ofn.lpstrFileTitle=(LPWSTR)fileNameStr;
		ofn.nMaxFile = sizeof(szFile);
	//
		ofn.lpstrFile[0] = '\0';
		wcscpy(ofn.lpstrFile, (LPWSTR)fileNameStr);
		ofn.Flags = OFN_OVERWRITEPROMPT;
	//no extention file!    ofn.lpstrFilter="Any file(*.*)\0*.*\0ddfs\0ddfs*\0";
		return(GetSaveFileName((LPOPENFILENAME)&ofn));
	}

  void OnOpen()
  {
    REQUIRE_FILE_THREAD();

    if(file_)
      return;
    
#if defined(OS_WIN)

    // Save the file in the user's "My Documents" folder.
    std::wstring fileNameStr = filename_;
    if(winHandler){
		if(OpenFileDialog(winHandler->renderWindow, fileNameStr.data())) {
			if(winHandler){
 				std::stringstream ss;
				ss<<"start("<<contentLength_<<")";
				winHandler->ExecJS(ss.str());
			}
		  int ct = 0;
		  LPWSTR name = PathFindFileName(fileNameStr.c_str());
		  LPWSTR ext = PathFindExtension(fileNameStr.c_str());
		  if(ext) {
			name[ext-name] = 0;
			ext++;
		  }
		  std::wstringstream ss;


		  // Make sure the file name is unique.
		  /*do*/ {
			//if(ct > 0)
			  //ss.str(L"");
			ss << szFile;//szFolderPath << L"\\" << name;
			if(ct > 0)
			  ss << L" (" << ct << L")";
			//if(ext)
			 // ss << L"." << ext;
			ct++;
		  } 
		  //while(PathFileExists(ss.str().c_str()));

		  {
			AutoLock lock_scope(this);
			filename_ = ss.str();
		  }

		  file_ = _wfopen(ss.str().c_str(), L"wb");
		  ASSERT(file_ != NULL);
		}
		else{
			delete winHandler;
			winHandler=NULL;
		}
	}
#else
    // TODO(port): Implement this.
    ASSERT(false); // Not implemented
#endif
  }

  void OnComplete()
  {
    REQUIRE_FILE_THREAD();

    if(!file_)
      return;

    // Make sure any pending data is written.
    OnReceivedData();

    fclose(file_);
    file_ = NULL;

    // Notify the listener that the download completed.
    listener_->NotifyDownloadComplete(filename_);
	if(winHandler){
		winHandler->ExecJS("complete()");
	}
  }

  void OnReceivedData()
  {
    REQUIRE_FILE_THREAD();

    std::vector<std::vector<char>*> data;

    // Remove all data from the pending data queue.
    {
      AutoLock lock_scope(this);
      if(!pending_data_.empty()) {
        data = pending_data_;
        pending_data_.clear();
      }
    }

    if(data.empty())
      return;

    // Write all pending data to file.
	CefString s;
	char s1[100];
	int64 f=0;
    std::vector<std::vector<char>*>::iterator it = data.begin();
    for(; it != data.end(); ++it) {
      std::vector<char>* buffer = *it;
 	  std::stringstream ss;
     if(file_)
        fwrite(&(*buffer)[0], buffer->size(), 1, file_);
	  byteCount+=(int64)buffer->size();
	  f=byteCount*100/contentLength_;
	  sprintf(s1, "%I64d", f);
	  ss<<"setProgress("<<buffer->size()<<")";
	  s=ss.str();
	  if(winHandler){
		  //try{
			winHandler->ExecJS(s);
		  //}
		  //catch(...){
		 // }
	  }
      delete buffer;
    }
    data.clear();
  }

  static void CloseDanglingFile(FILE *file)
  {
    fclose(file);
  }
  TransparentWnd *winHandler;
private:
  CefRefPtr<DownloadListener> listener_;
  CefString filename_;
  FILE* file_;
  std::vector<std::vector<char>*> pending_data_;
  int64 byteCount;

  IMPLEMENT_REFCOUNTING(ClientDownloadHandler);
  IMPLEMENT_LOCKING(ClientDownloadHandler);
};

CefRefPtr<CefDownloadHandler> CreateDownloadHandler(
    CefRefPtr<DownloadListener> listener, const CefString& fileName, int64 contentLength)
{
  CefRefPtr<ClientDownloadHandler> handler =
      new ClientDownloadHandler(listener, fileName, contentLength);
  handler->Initialize();
  return handler.get();
}
