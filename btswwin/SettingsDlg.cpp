// CSettingsDlg.cpp : implementation file
//

#include "pch.h"
#include "btswwin.h"
#include "afxdialogex.h"
#include "SettingsDlg.h"


static void setButtonCheck(CButton& button, const CMySettings::BoolValue& value) { button.SetCheck(value ? BST_CHECKED : BST_UNCHECKED); }
static BOOL isButtonChecked(const CButton& button) { return (button.GetCheck() == BST_CHECKED) ? TRUE : FALSE; }

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

// CSettingsDlg dialog

IMPLEMENT_DYNAMIC(CSettingsDlg, CDialogEx)

CSettingsDlg::CSettingsDlg(CMySettings& settings, CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_SETTINGS, pParent), m_settings(settings)
{
	addController(m_settings.switchByLcdState, m_switchByLcdState);
	addController(m_settings.restoreRadioState, m_restoreRadioState);
	addController(m_settings.saveWindowPlacement, m_saveWindowPlacement);
}

CSettingsDlg::~CSettingsDlg()
{
}

void CSettingsDlg::updateUIState()
{
	// restoreRadioState is used only if switchByLcdState == TRUE.
	m_restoreRadioState.EnableWindow(isButtonChecked(m_switchByLcdState));

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
}


BEGIN_MESSAGE_MAP(CSettingsDlg, CDialogEx)
	ON_BN_CLICKED(IDOK, &CSettingsDlg::OnBnClickedOk)
	ON_BN_CLICKED(IDC_CHECK_SWITCH_BY_LCD_STATE, &CSettingsDlg::OnClickedCheckButton)
	ON_BN_CLICKED(IDC_CHECK_RESTORE_RADIO_STATE, &CSettingsDlg::OnClickedCheckButton)
	ON_BN_CLICKED(IDC_CHECK_SAVE_WINDOW_PLACEMENT, &CSettingsDlg::OnClickedCheckButton)
	ON_BN_CLICKED(ID_SAVE_SETTINGS, &CSettingsDlg::OnClickedSaveSettings)
END_MESSAGE_MAP()


// CSettingsDlg message handlers


BOOL CSettingsDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

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


void CSettingsDlg::OnClickedSaveSettings()
{
	applyChanges();
	m_settings.save();

	updateUIState();
}
