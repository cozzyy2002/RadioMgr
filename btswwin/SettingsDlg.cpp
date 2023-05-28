// CSettingsDlg.cpp : implementation file
//

#include "pch.h"
#include "btswwin.h"
#include "afxdialogex.h"
#include "SettingsDlg.h"


static void setButtonCheck(CButton& button, const CMySettings::BoolValue& value) { button.SetCheck(value ? BST_CHECKED : BST_UNCHECKED); }
static BOOL isButtonChecked(const CButton& button) { return (button.GetCheck() == BST_CHECKED) ? TRUE : FALSE; }

#pragma region Controller<BOOL, CButton>
template<>
void Controller<BOOL, CButton>::setValue()
{
	setButtonCheck(ctrl, value);
}

template<>
void Controller<BOOL, CButton>::getValue()
{
	value = isButtonChecked(ctrl);
}

// Returns true if one of following conditions is satisfied.
//   State of the control is not set to the value yet.
//   The value is changed but not saved to setting storage yet.
template<>
bool Controller<BOOL, CButton>::isChanged() const
{
	auto buttonChecked = isButtonChecked(ctrl);
	auto bValue = (value ? TRUE : FALSE);
	return (buttonChecked != bValue) || value.isChanged();
}
#pragma endregion

#pragma region Controller<int, CEdit>
template<>
void Controller<int, CEdit>::setValue()
{
	CString text;
	text.Format(_T("%d"), (int)value);
	ctrl.SetWindowText(text);
}

template<>
void Controller<int, CEdit>::getValue()
{
	CString text;
	ctrl.GetWindowText(text);
	value = _tstoi(text.GetString());
}

// Returns true if one of following conditions is satisfied.
//   State of the control is not set to the value yet.
//   The value is changed but not saved to setting storage yet.
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
	addController(m_settings.switchByLcdState, m_switchByLcdState);
	addController(m_settings.restoreRadioState, m_restoreRadioState);
	addController(m_settings.setRadioStateTimeout, m_setRadioStateTimeout);
	addController(m_settings.setRadioOnDelay, m_setRadioOnDelay);
	addController(m_settings.autoSelectDevice, m_autoSelectDevice);
	addController(m_settings.saveWindowPlacement, m_saveWindowPlacement);
}

CSettingsDlg::~CSettingsDlg()
{
}

void CSettingsDlg::updateUIState()
{
	// restoreRadioState and setRadioOnDelay are used only if switchByLcdState is checked.
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
	bool isChanged = false;
	for(auto& c : m_controllers) {
		if(c->isChanged()) {
			isChanged = true;
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
		c->getValue();
	}
}

void CSettingsDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_CHECK_SWITCH_BY_LCD_STATE, m_switchByLcdState);
	DDX_Control(pDX, IDC_CHECK_RESTORE_RADIO_STATE, m_restoreRadioState);
	DDX_Control(pDX, IDC_CHECK_SAVE_WINDOW_PLACEMENT, m_saveWindowPlacement);
	DDX_Control(pDX, IDC_EDIT_SET_RADIO_STATE_TIMEOUT, m_setRadioStateTimeout);
	DDX_Control(pDX, IDC_SPIN_SET_RADIO_STATE_TIMEOUT, m_setRadioStateTimeoutSpin);
	DDX_Control(pDX, IDC_CHECK_AUTO_SELECT_DEVICE, m_autoSelectDevice);
	DDX_Control(pDX, IDC_EDIT_SET_RADIO_ON_DELAY, m_setRadioOnDelay);
	DDX_Control(pDX, IDC_SPIN_SET_RADIO_ON_DELAY, m_setRadioOnDelaySpin);
}


BEGIN_MESSAGE_MAP(CSettingsDlg, CDialogEx)
	ON_BN_CLICKED(IDOK, &CSettingsDlg::OnBnClickedOk)
	ON_BN_CLICKED(IDC_CHECK_SWITCH_BY_LCD_STATE, &CSettingsDlg::OnClickedCheckButton)
	ON_BN_CLICKED(IDC_CHECK_RESTORE_RADIO_STATE, &CSettingsDlg::OnClickedCheckButton)
	ON_BN_CLICKED(IDC_CHECK_SAVE_WINDOW_PLACEMENT, &CSettingsDlg::OnClickedCheckButton)
	ON_BN_CLICKED(IDC_CHECK_AUTO_SELECT_DEVICE, &CSettingsDlg::OnClickedCheckButton)
	ON_BN_CLICKED(ID_SAVE_SETTINGS, &CSettingsDlg::OnClickedSaveSettings)
	ON_EN_CHANGE(IDC_EDIT_SET_RADIO_STATE_TIMEOUT, &CSettingsDlg::OnEnChangeEdit)
	ON_EN_CHANGE(IDC_EDIT_SET_RADIO_ON_DELAY, &CSettingsDlg::OnEnChangeEdit)
END_MESSAGE_MAP()


// CSettingsDlg message handlers


BOOL CSettingsDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	m_setRadioOnDelaySpin.SetRange(0, 60);

	// Set timeout range of IRadioInstance::SetRadioState() method.
	// See "https://learn.microsoft.com/en-us/previous-versions/windows/hardware/radio/hh406610(v=vs.85)"
	m_setRadioStateTimeoutSpin.SetRange(1, 5);

	// Set all setting values to corresponding control.
	for(auto& c : m_controllers) {
		c->setValue();
	}

	updateUIState();

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
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
	// Note: Confirm that window handle is available
	//       to not call updateUIState() method inside constructor.
	if(m_setRadioStateTimeout.m_hWnd && m_setRadioOnDelay.m_hWnd) {
		updateUIState();
	}
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
