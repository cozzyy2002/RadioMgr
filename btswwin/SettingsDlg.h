#pragma once
#include "afxdialogex.h"

#include "MySettings.h"

// CSettingsDlg dialog

class CSettingsDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CSettingsDlg)

public:
	CSettingsDlg(CMySettings& settings, CWnd* pParent = nullptr);   // standard constructor
	virtual ~CSettingsDlg();

protected:
	CMySettings& m_settings;

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_SETTINGS };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	CButton m_switchByLcdState;
	CButton m_restoreRadioState;
	CButton m_saveWindowPlacement;
	virtual BOOL OnInitDialog();
	afx_msg void OnBnClickedOk();
};
