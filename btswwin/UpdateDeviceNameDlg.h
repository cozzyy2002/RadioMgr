#pragma once
#include "afxdialogex.h"


// CUpdateDeviceNameDlg dialog

class CUpdateDeviceNameDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CUpdateDeviceNameDlg)

public:
	CUpdateDeviceNameDlg(
		CString& deviceName,
		CWnd* pParent = nullptr
	);   // standard constructor
	virtual ~CUpdateDeviceNameDlg();

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_UPDATE_DEVICE_NAME };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	CString m_deviceName;

	DECLARE_MESSAGE_MAP()
};
