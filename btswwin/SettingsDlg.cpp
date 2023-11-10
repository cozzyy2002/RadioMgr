// CSettingsDlg.cpp : implementation file
//

#include "pch.h"
#include "btswwin.h"
#include "afxdialogex.h"
#include "SettingsDlg.h"
#include "BluetoothTabItem.h"
#include "NetworkTabItem.h"
#include "MiscTabItem.h"

// Sets check state of the button.
static void setButtonCheck(CButton& button, const CSettings::Value<bool>& value) { button.SetCheck(value ? BST_CHECKED : BST_UNCHECKED); }

// Returns TRUE if the button is checked, otherwise FALSE.
static BOOL isButtonChecked(const CButton& button) { return (button.GetCheck() == BST_CHECKED) ? TRUE : FALSE; }

#pragma region Controller<bool, CButton>
template<>
void Controller<bool, CButton>::setValueToCtrl()
{
	setButtonCheck(ctrl, value);
}

template<>
void Controller<bool, CButton>::getValueFromCtrl()
{
	value = isButtonChecked(ctrl) ? true : false;
}

template<>
bool Controller<bool, CButton>::isChanged() const
{
	auto buttonChecked = isButtonChecked(ctrl);
	auto bValue = (value ? TRUE : FALSE);
	return (buttonChecked != bValue) || value.isChanged();
}
#pragma endregion

#pragma region Controller<int, CEdit>
template<>
void Controller<int, CEdit>::setValueToCtrl()
{
	CString text;
	text.Format(_T("%d"), (int)value);
	ctrl.SetWindowText(text);
}

template<>
void Controller<int, CEdit>::getValueFromCtrl()
{
	CString text;
	ctrl.GetWindowText(text);
	value = _tstoi(text.GetString());
}

template<>
bool Controller<int, CEdit>::isChanged() const
{
	CString text;
	ctrl.GetWindowText(text);
	auto iValue = _tstoi(text.GetString());
	return (value != iValue) || value.isChanged();
}
#pragma endregion

// CSettingsDlg dialog

IMPLEMENT_DYNAMIC(CSettingsDlg, CDialogEx)

CSettingsDlg::CSettingsDlg(CMySettings& settings, CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_SETTINGS, pParent), m_settings(settings)
{
	//addController(m_settings.switchByLcdState, m_switchByLcdState);
	//addController(m_settings.restoreRadioState, m_restoreRadioState);
	//addController(m_settings.setRadioStateTimeout, m_setRadioStateTimeout);
	//addController(m_settings.setRadioOnDelay, m_setRadioOnDelay);
	//addController(m_settings.autoCheckRadioInstance, m_autoCheckRadioInstance);
	//addController(m_settings.autoSelectDevice, m_autoSelectDevice);
	//addController(m_settings.saveWindowPlacement, m_saveWindowPlacement);
}

CSettingsDlg::~CSettingsDlg()
{
}

void CSettingsDlg::updateUIState()
{
	return;

	// Set enabled state of the controls depending on whether switchByLcdState is checked.
	static const int ids[] = {
		IDC_CHECK_RESTORE_RADIO_STATE,
		IDC_STATIC_SET_RADIO_ON_DELAY, IDC_EDIT_SET_RADIO_ON_DELAY, IDC_SPIN_SET_RADIO_ON_DELAY
	};
	auto enable = isButtonChecked(m_switchByLcdState);
	for(auto id : ids) {
		GetDlgItem(id)->EnableWindow(enable);
	}

	// Set enabled state of [Save] button.
	// The button is enabled if at least one setting value is different from it's setting storage.
	auto isChanged = FALSE;
	for(auto& c : m_controllers) {
		if(c->isChanged()) {
			isChanged = TRUE;
			break;
		}
	}
	auto button = GetDlgItem(ID_SAVE_SETTINGS);
	button->EnableWindow(isChanged);
}

// Sets state of all controls to corresponding settings value.
void CSettingsDlg::applyChanges()
{
	for(auto& c : m_controllers) {
		c->getValueFromCtrl();
	}
}

void CSettingsDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	//DDX_Control(pDX, IDC_CHECK_SWITCH_BY_LCD_STATE, m_switchByLcdState);
	//DDX_Control(pDX, IDC_CHECK_RESTORE_RADIO_STATE, m_restoreRadioState);
	//DDX_Control(pDX, IDC_CHECK_SAVE_WINDOW_PLACEMENT, m_saveWindowPlacement);
	//DDX_Control(pDX, IDC_EDIT_SET_RADIO_STATE_TIMEOUT, m_setRadioStateTimeout);
	//DDX_Control(pDX, IDC_SPIN_SET_RADIO_STATE_TIMEOUT, m_setRadioStateTimeoutSpin);
	//DDX_Control(pDX, IDC_CHECK_AUTO_SELECT_DEVICE, m_autoSelectDevice);
	//DDX_Control(pDX, IDC_EDIT_SET_RADIO_ON_DELAY, m_setRadioOnDelay);
	//DDX_Control(pDX, IDC_SPIN_SET_RADIO_ON_DELAY, m_setRadioOnDelaySpin);
	//DDX_Control(pDX, IDC_CHECK_AUTO_CHECK_RADIO_INSTANCE, m_autoCheckRadioInstance);
	DDX_Control(pDX, IDC_TAB_SETTINGS, m_tabCtrl);
}


BEGIN_MESSAGE_MAP(CSettingsDlg, CDialogEx)
	ON_BN_CLICKED(IDOK, &CSettingsDlg::OnBnClickedOk)
	ON_BN_CLICKED(IDC_CHECK_SWITCH_BY_LCD_STATE, &CSettingsDlg::OnClickedCheckButton)
	ON_BN_CLICKED(IDC_CHECK_RESTORE_RADIO_STATE, &CSettingsDlg::OnClickedCheckButton)
	ON_BN_CLICKED(IDC_CHECK_SAVE_WINDOW_PLACEMENT, &CSettingsDlg::OnClickedCheckButton)
	ON_BN_CLICKED(IDC_CHECK_AUTO_CHECK_RADIO_INSTANCE, &CSettingsDlg::OnClickedCheckButton)
	ON_BN_CLICKED(IDC_CHECK_AUTO_SELECT_DEVICE, &CSettingsDlg::OnClickedCheckButton)
	ON_BN_CLICKED(ID_SAVE_SETTINGS, &CSettingsDlg::OnClickedSaveSettings)
	ON_EN_CHANGE(IDC_EDIT_SET_RADIO_STATE_TIMEOUT, &CSettingsDlg::OnEnChangeEdit)
	ON_EN_CHANGE(IDC_EDIT_SET_RADIO_ON_DELAY, &CSettingsDlg::OnEnChangeEdit)
	ON_NOTIFY(TCN_SELCHANGE, IDC_TAB_SETTINGS, &CSettingsDlg::OnTcnSelchangeTabSettings)
END_MESSAGE_MAP()


// CSettingsDlg message handlers


BOOL CSettingsDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	m_tabs.push_back(std::unique_ptr<CDialogEx>(new CBluetoothTabItem()));
	m_tabs.push_back(std::unique_ptr<CDialogEx>(new CNetworkTabItem()));
	m_tabs.push_back(std::unique_ptr<CDialogEx>(new CMiscTabItem()));
	m_tabs[0]->Create(IDD_SETTINGS_BLUETOOTH, this);
	m_tabs[1]->Create(IDD_SETTINGS_NETWORK, this);
	m_tabs[2]->Create(IDD_SETTINGS_MISC, this);
	m_tabCtrl.InsertItem(0, _T("Bluetooth"));
	m_tabCtrl.InsertItem(1, _T("Network"));
	m_tabCtrl.InsertItem(2, _T("Misc"));
	CRect rect;
	m_tabCtrl.GetWindowRect(&rect);
	m_tabCtrl.AdjustRect(FALSE, &rect);
	m_tabCtrl.ScreenToClient(&rect);
	CRect tabRect;
	m_tabCtrl.GetItemRect(0, &tabRect);
	rect.top += tabRect.Height();
	for each(auto& d in m_tabs) {
		d->MoveWindow(&rect);
	}
	OnTcnSelchangeTabSettings(nullptr, nullptr);

	//m_setRadioOnDelaySpin.SetRange(0, 60);

	// Set timeout range of IRadioInstance::SetRadioState() method.
	// See "https://learn.microsoft.com/en-us/previous-versions/windows/hardware/radio/hh406610(v=vs.85)"
	//m_setRadioStateTimeoutSpin.SetRange(1, 5);

	// Set all setting values to corresponding control.
	for(auto& c : m_controllers) {
		c->setValueToCtrl();
	}

	updateUIState();

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}


void CSettingsDlg::OnTcnSelchangeTabSettings(NMHDR* pNMHDR, LRESULT* pResult)
{
	auto sel = m_tabCtrl.GetCurSel();
	for(int i = 0; i < m_tabs.size(); i++) {
		m_tabs[i]->ShowWindow((i == sel) ? SW_SHOW : SW_HIDE);
	}

	if(pResult) { *pResult = 0; }
}


void CSettingsDlg::OnBnClickedOk()
{
	applyChanges();

	CDialogEx::OnOK();
}


void CSettingsDlg::OnClickedCheckButton()
{
	updateUIState();
}


void CSettingsDlg::OnEnChangeEdit()
{
	// Confirm that window handle of all controls are available
	// to not call updateUIState() method inside constructor.
	for(auto& c : m_controllers) {
		if(!c->getCtrlWnd()->GetSafeHwnd()) { return; }
	}

	updateUIState();
}


void CSettingsDlg::OnClickedSaveSettings()
{
	if(m_saveWindowPlacement.GetCheck())
	{
		m_settings.windowPlacement->length = sizeof(WINDOWPLACEMENT);
		WIN32_EXPECT(GetWindowPlacement(m_settings.windowPlacement));
	}

	applyChanges();
	m_settings.save();

	updateUIState();
}
