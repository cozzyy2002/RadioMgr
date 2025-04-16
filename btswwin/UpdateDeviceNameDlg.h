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
	CString& m_deviceNameOrg;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	CString m_deviceName;
	afx_msg void OnEnChangeEditDeviceName();
	virtual void OnOK();
};
