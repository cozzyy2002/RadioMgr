// BluetoothTabItem.cpp : implementation file
//

#include "pch.h"
#include "btswwin.h"
#include "afxdialogex.h"
#include "BluetoothTabItem.h"


// CBluetoothTabItem dialog

IMPLEMENT_DYNAMIC(CBluetoothTabItem, CDialogEx)

CBluetoothTabItem::CBluetoothTabItem(CMySettings& settings)
	: CTabItem(IDD_SETTINGS_BLUETOOTH, _T("Bluetooth"), settings)
{
	addController(m_settings.switchByLcdState, m_switchByLcdState);
	addController(m_settings.restoreRadioState, m_restoreRadioState);
	addController(m_settings.setRadioStateTimeout, m_setRadioStateTimeout);
	addController(m_settings.setRadioOnDelay, m_setRadioOnDelay);
	addController(m_settings.autoCheckRadioInstance, m_autoCheckRadioInstance);
	addController(m_settings.autoSelectDevice, m_autoSelectDevice);
	addController(m_settings.bluetoothPollingTimer, m_bluetoothPollingTimer);
}

CBluetoothTabItem::~CBluetoothTabItem()
{
}

void CBluetoothTabItem::updateUIState()
{
	// Set enabled state of the controls depending on whether switchByLcdState is checked.
	static const int ids[] = {
		IDC_CHECK_RESTORE_RADIO_STATE,
		IDC_STATIC_SET_RADIO_ON_DELAY, IDC_EDIT_SET_RADIO_ON_DELAY, IDC_SPIN_SET_RADIO_ON_DELAY
	};
	auto enable = isButtonChecked(m_switchByLcdState);
	for(auto id : ids) {
		GetDlgItem(id)->EnableWindow(enable);
	}
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
	DDX_Control(pDX, IDC_EDIT_BLUETOOTH_POLLING_TIMER, m_bluetoothPollingTimer);
	DDX_Control(pDX, IDC_SPIN_BLUETOOTH_POLLING_TIMER, m_bluetoothPollingTimerSpin);
}


BEGIN_MESSAGE_MAP(CBluetoothTabItem, CDialogEx)
	ON_BN_CLICKED(IDC_CHECK_SWITCH_BY_LCD_STATE, &CBluetoothTabItem::OnClickedCheckButtonSwitchByLcdState)
	ON_BN_CLICKED(IDC_CHECK_RESTORE_RADIO_STATE, &CBluetoothTabItem::OnClickedCheckButton)
	ON_BN_CLICKED(IDC_CHECK_AUTO_CHECK_RADIO_INSTANCE, &CBluetoothTabItem::OnClickedCheckButton)
	ON_BN_CLICKED(IDC_CHECK_AUTO_SELECT_DEVICE, &CBluetoothTabItem::OnClickedCheckButton)
	ON_EN_CHANGE(IDC_EDIT_SET_RADIO_STATE_TIMEOUT, &CBluetoothTabItem::OnEnChangeEdit)
	ON_EN_CHANGE(IDC_EDIT_SET_RADIO_ON_DELAY, &CBluetoothTabItem::OnEnChangeEdit)
	ON_EN_CHANGE(IDC_EDIT_BLUETOOTH_POLLING_TIMER, &CBluetoothTabItem::OnEnChangeEdit)
END_MESSAGE_MAP()


// CBluetoothTabItem message handlers

BOOL CBluetoothTabItem::OnInitDialog()
{
	CTabItem::OnInitDialog();

	m_setRadioOnDelaySpin.SetRange(0, 60);

	// Set timeout range of IRadioInstance::SetRadioState() method.
	// See "https://learn.microsoft.com/en-us/previous-versions/windows/hardware/radio/hh406610(v=vs.85)"
	m_setRadioStateTimeoutSpin.SetRange(1, 5);

	m_bluetoothPollingTimerSpin.SetRange(1, 60);

	updateUIState();

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}


void CBluetoothTabItem::OnClickedCheckButton()
{
	notifyValueChanged();
}

void CBluetoothTabItem::OnClickedCheckButtonSwitchByLcdState()
{
	updateUIState();
	notifyValueChanged();
}


void CBluetoothTabItem::OnEnChangeEdit()
{
	// Confirm that window handle of all controls are available
	// to not call updateUIState() method inside constructor.
	if(!areAllControlsAvailable()) { return; }

	notifyValueChanged();
}
