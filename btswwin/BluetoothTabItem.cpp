// BluetoothTabItem.cpp : implementation file
//

#include "pch.h"
#include "btswwin.h"
#include "afxdialogex.h"
#include "BluetoothTabItem.h"


// CBluetoothTabItem dialog

IMPLEMENT_DYNAMIC(CBluetoothTabItem, CDialogEx)

CBluetoothTabItem::CBluetoothTabItem(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_SETTINGS_BLUETOOTH, pParent)
{

}

CBluetoothTabItem::~CBluetoothTabItem()
{
}

void CBluetoothTabItem::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CBluetoothTabItem, CDialogEx)
END_MESSAGE_MAP()


// CBluetoothTabItem message handlers
