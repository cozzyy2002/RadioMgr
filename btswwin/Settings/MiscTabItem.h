#pragma once
#include "afxdialogex.h"
#include "TabItem.h"

// CMiscTabItem dialog

class CMiscTabItem : public CTabItem
{
	DECLARE_DYNAMIC(CMiscTabItem)

public:
	CMiscTabItem(CMySettings& settings);

protected:
	void updateUIState();

public:
// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_SETTINGS_MISC };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	CButton m_saveWindowPlacement;
	afx_msg void OnClickedCheckButton();
	afx_msg void OnBnClickedButtonLogSettings();
	CButton m_batteryRemainLidOpenClose;
	CButton m_batteryRemainPowerSourceChanged;
	CButton m_batteryRemainConsoleDisplayChanged;
	CComboBoxEx m_logBatteryRemainTriggers;
};
