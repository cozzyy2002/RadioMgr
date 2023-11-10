// MiscTabItem.cpp : implementation file
//

#include "pch.h"
#include "btswwin.h"
#include "afxdialogex.h"
#include "MiscTabItem.h"


// CMiscTabItem dialog

IMPLEMENT_DYNAMIC(CMiscTabItem, CDialogEx)

CMiscTabItem::CMiscTabItem(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_SETTINGS_MISC, pParent)
{

}

CMiscTabItem::~CMiscTabItem()
{
}

void CMiscTabItem::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CMiscTabItem, CDialogEx)
END_MESSAGE_MAP()


// CMiscTabItem message handlers
