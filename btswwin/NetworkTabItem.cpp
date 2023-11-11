// NetworkTabItem.cpp : implementation file
//

#include "pch.h"
#include "btswwin.h"
#include "afxdialogex.h"
#include "NetworkTabItem.h"


// CNetworkTabItem dialog

IMPLEMENT_DYNAMIC(CNetworkTabItem, CDialogEx)

CNetworkTabItem::CNetworkTabItem(CMySettings& settings)
	: CTabItem(IDD_SETTINGS_NETWORK, _T("Network"), settings)
{
}

CNetworkTabItem::~CNetworkTabItem()
{
}

void CNetworkTabItem::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BOOL CNetworkTabItem::OnInitDialog()
{
	CTabItem::onInitDialog();

	return 0;
}


BEGIN_MESSAGE_MAP(CNetworkTabItem, CDialogEx)
END_MESSAGE_MAP()


// CNetworkTabItem message handlers
