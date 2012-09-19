// Copyright (c) 2008-2009 The Chromium Embedded Framework Authors. All rights
// reserved. Use of this source code is governed by a BSD-style license that
// can be found in the LICENSE file.

#include "cefclient/resource_util.h"
#include "include/cef_stream.h"
#include "include/wrapper/cef_byte_read_handler.h"

#if defined(OS_WIN)

bool LoadBinaryResourceByName(LPCWSTR name, DWORD &dwSize, LPBYTE &pBytes, HMODULE hModule) {
  //extern HINSTANCE hInst;
  HRSRC hRes = FindResource(hModule, name,
                            MAKEINTRESOURCE(256));
  if (hRes) {
    HGLOBAL hGlob = LoadResource(hModule, hRes);
    if (hGlob) {
      dwSize = SizeofResource(hModule, hRes);
      pBytes = (LPBYTE)LockResource(hGlob);
      if (dwSize > 0 && pBytes)
        return true;
    }
  }

  return false;
}

bool LoadBinaryResource(int binaryId, DWORD &dwSize, LPBYTE &pBytes, HMODULE hModule) {
  extern HINSTANCE hInst;
  HRSRC hRes = FindResource(hModule, MAKEINTRESOURCE(binaryId),
                            MAKEINTRESOURCE(256));
  if (hRes) {
    HGLOBAL hGlob = LoadResource(hModule, hRes);
    if (hGlob) {
      dwSize = SizeofResource(hModule, hRes);
      pBytes = (LPBYTE)LockResource(hGlob);
      if (dwSize > 0 && pBytes)
        return true;
    }
  }

  return false;
}

CefRefPtr<CefStreamReader> GetBinaryResourceReader(int binaryId) {
  DWORD dwSize;
  LPBYTE pBytes;

  if (LoadBinaryResource(binaryId, dwSize, pBytes)) {
    return CefStreamReader::CreateForHandler(
        new CefByteReadHandler(pBytes, dwSize, NULL));
  }

  return NULL;
}

#endif  // OS_WIN
