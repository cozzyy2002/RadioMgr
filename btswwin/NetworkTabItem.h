#pragma once
#include "afxdialogex.h"
#include "TabItem.h"

// CNetworkTabItem dialog

class CNetworkTabItem : public CTabItem
{
	DECLARE_DYNAMIC(CNetworkTabItem)

public:
	CNetworkTabItem(CMySettings& settings);
	virtual ~CNetworkTabItem();

protected:
	void updateUIState();

public:
// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_SETTINGS_NETWORK };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	CButton m_connectVpn;
	CEdit m_vpnName;
	CButton m_vpnConnection;
	afx_msg void OnChangeEdit();
	afx_msg void OnClickedCheck();
	afx_msg void OnClickedRadio();
};
