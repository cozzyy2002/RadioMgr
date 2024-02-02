
// btswwin.cpp : Defines the class behaviors for the application.
//

#include "pch.h"
#include "framework.h"
#include "btswwin.h"
#include "btswwinDlg.h"

#include <pathcch.h>
#pragma comment(lib, "pathcch.lib")

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CbtswwinApp

BEGIN_MESSAGE_MAP(CbtswwinApp, CWinApp)
	ON_COMMAND(ID_HELP, &CWinApp::OnHelp)
END_MESSAGE_MAP()


// CbtswwinApp construction

CbtswwinApp::CbtswwinApp()
{
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}


// The one and only CbtswwinApp object

CbtswwinApp theApp;
static auto logger(log4cxx::Logger::getLogger(_T("btswwin.CbtswwinApp")));

// CbtswwinApp initialization

BOOL CbtswwinApp::InitInstance()
{
	// Configure log4cxx using XML file in the folder of exe file.
	WCHAR path[MAX_PATH];
	static const auto pathLength = ARRAYSIZE(path);
	GetModuleFileNameW(NULL, path, pathLength);

	// Prevent multiple app from running.
	// Absolute path of app is used as the mutex name in global namespace.
	CString mutexName(_T("Global\\"));
	{
		CString _path(path);
		_path.Replace(_T('\\'), _T('/'));
		mutexName += _path;
	}
	CHandle appMutex(CreateMutex(NULL, TRUE, mutexName.GetString()));
	auto err = GetLastError();
	CString errMsg;
	switch(err) {
	case ERROR_SUCCESS:
		break;
	case ERROR_ALREADY_EXISTS:
		errMsg.Format(_T("`%s` is already running."), path);
		break;
	default:
		errMsg.Format(_T("CreateMutex failed.\nError = %d"), err);
		break;
	}
	if(!errMsg.IsEmpty()) {
		MessageBox(NULL, errMsg.GetString(), _T("Error"), MB_ICONEXCLAMATION | MB_OK);
		return FALSE;
	}

	PathCchRemoveFileSpec(path, pathLength);
	WCHAR configFileName[MAX_PATH];
	PathCchCombine(configFileName, ARRAYSIZE(configFileName), path, L"log4cxx.config.xml");
	log4cxx::xml::DOMConfigurator::configure(configFileName);
	LOG4CXX_INFO(logger, L"Logger is configured: " << configFileName);
	LOG4CXX_DEBUG(logger, L"Mutex is created: " << mutexName.GetString());

	CWinApp::InitInstance();


	// Create the shell manager, in case the dialog contains
	// any shell tree view or shell list view controls.
	CShellManager *pShellManager = new CShellManager;

	// Activate "Windows Native" visual manager for enabling themes in MFC controls
	CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerWindows));

	CResourceReader resourceReader;
	LOG4CXX_INFO(logger, resourceReader.getProductName().GetString() << _T(" V") << resourceReader.getFileVersion().GetString());

	// Standard initialization
	// If you are not using these features and wish to reduce the size
	// of your final executable, you should remove from the following
	// the specific initialization routines you do not need
	// Change the registry key under which our settings are stored
	// TODO: You should modify this string to be something appropriate
	// such as the name of your company or organization
	SetRegistryKey(resourceReader.getCompanyName().GetString());

	// Initialize COM library and Uninitialize it when this method exits.
	struct CoInitializeDeleter {
		using pointer = bool;
		void operator()(bool) { CoUninitialize(); }
	};
	std::unique_ptr<bool, CoInitializeDeleter> coinit(SUCCEEDED(CoInitializeEx(NULL, COINIT_MULTITHREADED)));

	CbtswwinDlg dlg(resourceReader);
	m_pMainWnd = &dlg;
	INT_PTR nResponse = dlg.DoModal();
	if (nResponse == IDOK)
	{
		// TODO: Place code here to handle when the dialog is
		//  dismissed with OK
	}
	else if (nResponse == IDCANCEL)
	{
		// TODO: Place code here to handle when the dialog is
		//  dismissed with Cancel
	}
	else if (nResponse == -1)
	{
		TRACE(traceAppMsg, 0, "Warning: dialog creation failed, so application is terminating unexpectedly.\n");
		TRACE(traceAppMsg, 0, "Warning: if you are using MFC controls on the dialog, you cannot #define _AFX_NO_MFC_CONTROLS_IN_DIALOGS.\n");
	}

	// Delete the shell manager created above.
	if (pShellManager != nullptr)
	{
		delete pShellManager;
	}

#if !defined(_AFXDLL) && !defined(_AFX_NO_MFC_CONTROLS_IN_DIALOGS)
	ControlBarCleanUp();
#endif

	// Since the dialog has been closed, return FALSE so that we exit the
	//  application, rather than start the application's message pump.
	return FALSE;
}
