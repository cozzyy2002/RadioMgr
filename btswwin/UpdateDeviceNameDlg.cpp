// CUpdateDeviceNameDlg.cpp : implementation file
//

#include "pch.h"
#include "btswwin.h"
#include "afxdialogex.h"
#include "UpdateDeviceNameDlg.h"


// CUpdateDeviceNameDlg dialog

IMPLEMENT_DYNAMIC(CUpdateDeviceNameDlg, CDialogEx)

CUpdateDeviceNameDlg::CUpdateDeviceNameDlg(
	CString& deviceName,
	CWnd* pParent
)
	: CDialogEx(IDD_UPDATE_DEVICE_NAME, pParent)
	, m_deviceName(deviceName)
{

}

CUpdateDeviceNameDlg::~CUpdateDeviceNameDlg()
{
}

void CUpdateDeviceNameDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CUpdateDeviceNameDlg, CDialogEx)
END_MESSAGE_MAP()


// CUpdateDeviceNameDlg message handlers
