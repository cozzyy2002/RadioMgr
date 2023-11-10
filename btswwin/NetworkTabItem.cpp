// NetworkTabItem.cpp : implementation file
//

#include "pch.h"
#include "btswwin.h"
#include "afxdialogex.h"
#include "NetworkTabItem.h"


// CNetworkTabItem dialog

IMPLEMENT_DYNAMIC(CNetworkTabItem, CDialogEx)

CNetworkTabItem::CNetworkTabItem(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_SETTINGS_NETWORK, pParent)
{

}

CNetworkTabItem::~CNetworkTabItem()
{
}

void CNetworkTabItem::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CNetworkTabItem, CDialogEx)
END_MESSAGE_MAP()


// CNetworkTabItem message handlers
