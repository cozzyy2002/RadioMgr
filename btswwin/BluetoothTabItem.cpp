// BluetoothTabItem.cpp : implementation file
//

#include "pch.h"
#include "btswwin.h"
#include "afxdialogex.h"
#include "BluetoothTabItem.h"


// CBluetoothTabItem dialog

IMPLEMENT_DYNAMIC(CBluetoothTabItem, CDialogEx)

CBluetoothTabItem::CBluetoothTabItem()
	: CTabItem(IDD_SETTINGS_BLUETOOTH)
{
}

CBluetoothTabItem::~CBluetoothTabItem()
{
}

void CBluetoothTabItem::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_CHECK_SWITCH_BY_LCD_STATE, m_switchByLcdState);
	DDX_Control(pDX, IDC_CHECK_RESTORE_RADIO_STATE, m_restoreRadioState);
	DDX_Control(pDX, IDC_EDIT_SET_RADIO_STATE_TIMEOUT, m_setRadioStateTimeout);
	DDX_Control(pDX, IDC_SPIN_SET_RADIO_STATE_TIMEOUT, m_setRadioStateTimeoutSpin);
	DDX_Control(pDX, IDC_CHECK_AUTO_SELECT_DEVICE, m_autoSelectDevice);
	DDX_Control(pDX, IDC_EDIT_SET_RADIO_ON_DELAY, m_setRadioOnDelay);
	DDX_Control(pDX, IDC_SPIN_SET_RADIO_ON_DELAY, m_setRadioOnDelaySpin);
	DDX_Control(pDX, IDC_CHECK_AUTO_CHECK_RADIO_INSTANCE, m_autoCheckRadioInstance);
}


BEGIN_MESSAGE_MAP(CBluetoothTabItem, CDialogEx)
//	ON_WM_CREATE()
END_MESSAGE_MAP()


// CBluetoothTabItem message handlers

BOOL CBluetoothTabItem::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	m_setRadioOnDelaySpin.SetRange(0, 60);

	// Set timeout range of IRadioInstance::SetRadioState() method.
	// See "https://learn.microsoft.com/en-us/previous-versions/windows/hardware/radio/hh406610(v=vs.85)"
	m_setRadioStateTimeoutSpin.SetRange(1, 5);

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}
