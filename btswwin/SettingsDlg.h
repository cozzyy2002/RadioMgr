#pragma once
#include "afxdialogex.h"

#include "MySettings.h"

class CTabItem;

// CSettingsDlg dialog

class CSettingsDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CSettingsDlg)

public:
	CSettingsDlg(CMySettings& settings, CWnd* pParent = nullptr);   // standard constructor
	virtual ~CSettingsDlg();

protected:
	CMySettings& m_settings;
	std::vector<std::unique_ptr<CTabItem>> m_tabItems;

	void updateUIState();
	void applyChanges();

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_SETTINGS };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	afx_msg void OnBnClickedOk();
	afx_msg void OnClickedSaveSettings();
	CTabCtrl m_tabCtrl;
	afx_msg void OnTcnSelchangeTabSettings(NMHDR* pNMHDR, LRESULT* pResult);
};
