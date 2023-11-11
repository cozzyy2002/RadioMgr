// MiscTabItem.cpp : implementation file
//

#include "pch.h"
#include "btswwin.h"
#include "afxdialogex.h"
#include "MiscTabItem.h"


// CMiscTabItem dialog

IMPLEMENT_DYNAMIC(CMiscTabItem, CDialogEx)

CMiscTabItem::CMiscTabItem()
	: CTabItem(IDD_SETTINGS_MISC)
{
}

CMiscTabItem::~CMiscTabItem()
{
}

void CMiscTabItem::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_CHECK_SAVE_WINDOW_PLACEMENT, m_saveWindowPlacement);
}


BEGIN_MESSAGE_MAP(CMiscTabItem, CDialogEx)
END_MESSAGE_MAP()


// CMiscTabItem message handlers
