
// btswwin.h : main header file for the PROJECT_NAME application
//

#pragma once

#ifndef __AFXWIN_H__
	#error "include 'pch.h' before including this file for PCH"
#endif

#include "resource.h"		// main symbols


// CbtswwinApp:
// See btswwin.cpp for the implementation of this class
//

class CbtswwinApp : public CWinApp
{
public:
	CbtswwinApp();

	// Overrides
public:
	virtual BOOL InitInstance();

// Implementation

	DECLARE_MESSAGE_MAP()
};

extern CbtswwinApp theApp;

namespace std
{
#ifdef _UNICODE
	using tostringstream = wostringstream;
#else
	using tostringstream = ostringstream;
#endif
}
