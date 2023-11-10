#pragma once
#include "afxdialogex.h"


// CMiscTabItem dialog

class CMiscTabItem : public CDialogEx
{
	DECLARE_DYNAMIC(CMiscTabItem)

public:
	CMiscTabItem(CWnd* pParent = nullptr);   // standard constructor
	virtual ~CMiscTabItem();

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_SETTINGS_MISC };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
};
