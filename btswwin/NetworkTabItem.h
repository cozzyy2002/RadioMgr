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

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_SETTINGS_NETWORK };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
};
