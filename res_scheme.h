// Copyright (c) 2009 The Chromium Embedded Framework Authors. All rights
// reserved. Use of this source code is governed by a BSD-style license that
// can be found in the LICENSE file.

#ifndef RES_SCHEME_H_
#define RES_SCHEME_H_
#pragma once

#include "include/cef_base.h"

class CefBrowser;

// Register the scheme handler.
void InitResScheme();

// Run the test.
void RunResScheme(CefRefPtr<CefBrowser> browser);

#endif  // CEF_TESTS_CEFCLIENT_SCHEME_TEST_H_
