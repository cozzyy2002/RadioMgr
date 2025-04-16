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
	, m_deviceName(deviceName), m_deviceNameOrg(deviceName)
{

}

CUpdateDeviceNameDlg::~CUpdateDeviceNameDlg()
{
}

void CUpdateDeviceNameDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT_DEVICE_NAME, m_deviceName);
}


BEGIN_MESSAGE_MAP(CUpdateDeviceNameDlg, CDialogEx)
	ON_EN_CHANGE(IDC_EDIT_DEVICE_NAME, &CUpdateDeviceNameDlg::OnEnChangeEditDeviceName)
END_MESSAGE_MAP()


// CUpdateDeviceNameDlg message handlers


void CUpdateDeviceNameDlg::OnEnChangeEditDeviceName()
{
	// TODO:  If this is a RICHEDIT control, the control will not
	// send this notification unless you override the CDialogEx::OnInitDialog()
	// function and call CRichEditCtrl().SetEventMask()
	// with the ENM_CHANGE flag ORed into the mask.

	// TODO:  Add your control notification handler code here
}


void CUpdateDeviceNameDlg::OnOK()
{
	UpdateData();
	m_deviceNameOrg = m_deviceName;

	CDialogEx::OnOK();
}
