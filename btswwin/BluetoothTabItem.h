#pragma once
#include "afxdialogex.h"
#include "TabItem.h"

// CBluetoothTabItem dialog

class CBluetoothTabItem : public CTabItem
{
	DECLARE_DYNAMIC(CBluetoothTabItem)

public:
	CBluetoothTabItem();
	virtual ~CBluetoothTabItem();

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_SETTINGS_BLUETOOTH };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	CButton m_switchByLcdState;
	CButton m_restoreRadioState;
	CEdit m_setRadioStateTimeout;
	CSpinButtonCtrl m_setRadioStateTimeoutSpin;
	CButton m_autoSelectDevice;
	CEdit m_setRadioOnDelay;
	CSpinButtonCtrl m_setRadioOnDelaySpin;
	CButton m_autoCheckRadioInstance;
//	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	virtual BOOL OnInitDialog();
};
