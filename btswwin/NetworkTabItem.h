#pragma once
#include "afxdialogex.h"


// CNetworkTabItem dialog

class CNetworkTabItem : public CDialogEx
{
	DECLARE_DYNAMIC(CNetworkTabItem)

public:
	CNetworkTabItem(CWnd* pParent = nullptr);   // standard constructor
	virtual ~CNetworkTabItem();

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_SETTINGS_NETWORK };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
};
