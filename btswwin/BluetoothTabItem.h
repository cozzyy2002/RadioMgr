#pragma once
#include "afxdialogex.h"


// CBluetoothTabItem dialog

class CBluetoothTabItem : public CDialogEx
{
	DECLARE_DYNAMIC(CBluetoothTabItem)

public:
	CBluetoothTabItem(CWnd* pParent = nullptr);   // standard constructor
	virtual ~CBluetoothTabItem();

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_SETTINGS_BLUETOOTH };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
};
