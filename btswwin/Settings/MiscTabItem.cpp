// MiscTabItem.cpp : implementation file
//

#include "pch.h"
#include "btswwin.h"
#include "afxdialogex.h"
#include "MiscTabItem.h"

#include <sstream>

// Logger used to log all setting value.
static auto& settingsLogger(log4cxx::Logger::getLogger(_T("btswwin")));


class LogPowerRaminController : public IController
{
public:
	LogPowerRaminController(
		CMiscTabItem* dlg,
		CSettings::EnumValue<CMySettings::Trigger>& value
	) : m_dlg(dlg), m_value(value) {}

	virtual void setValueToCtrl() override;
	virtual void getValueFromCtrl() override;
	virtual bool isChanged() const override;

protected:
	CMiscTabItem* m_dlg;
	CSettings::EnumValue<CMySettings::Trigger>& m_value;

	// Returns button states for Log Battery Ramain.
	CMySettings::Trigger getButtonStates() const;
};


// CMiscTabItem dialog

IMPLEMENT_DYNAMIC(CMiscTabItem, CDialogEx)

CMiscTabItem::CMiscTabItem(CMySettings& settings)
	: CTabItem(IDD_SETTINGS_MISC, _T("Miscellaneous"), settings)
{
	addController(m_settings.saveWindowPlacement, m_saveWindowPlacement);
	addController(new LogPowerRaminController(this, settings.batteryLogTrigger));
}

void CMiscTabItem::updateUIState()
{
}

void CMiscTabItem::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_CHECK_SAVE_WINDOW_PLACEMENT, m_saveWindowPlacement);
	DDX_Control(pDX, IDC_CHECK_LOG_BATTERY_REMAIN_LID_OPEN_CLOSE, m_batteryRemainLidOpenClose);
	DDX_Control(pDX, IDC_CHECK_LOG_BATTERY_REMAIN_POWER_SOURCE_CHANGED, m_batteryRemainPowerSourceChanged);
}


BEGIN_MESSAGE_MAP(CMiscTabItem, CDialogEx)
	ON_BN_CLICKED(IDC_CHECK_SAVE_WINDOW_PLACEMENT, &CMiscTabItem::OnClickedCheckButton)
	ON_BN_CLICKED(IDC_CHECK_LOG_BATTERY_REMAIN_LID_OPEN_CLOSE, &CMiscTabItem::OnClickedCheckButton)
	ON_BN_CLICKED(IDC_CHECK_LOG_BATTERY_REMAIN_POWER_SOURCE_CHANGED, &CMiscTabItem::OnClickedCheckButton)
	ON_BN_CLICKED(IDC_BUTTON_LOG_SETTINGS, &CMiscTabItem::OnBnClickedButtonLogSettings)
END_MESSAGE_MAP()


// CMiscTabItem message handlers


BOOL CMiscTabItem::OnInitDialog()
{
	CTabItem::OnInitDialog();

	return 0;
}

void CMiscTabItem::OnClickedCheckButton()
{
	notifyValueChanged();
}

// Writes setting value to the log file.
void CMiscTabItem::OnBnClickedButtonLogSettings()
{
	std::tostringstream msg;
	msg << _T("All setting values in ") << m_settings.getRegistryKeyName().GetString();

	for(auto& value : m_settings.getValueList()) {
		msg << _T("\n")
			<< (value->isChanged() ? _T(" * ") : _T("   "))
			<< value->toString(true).GetString();
	}
	settingsLogger->forcedLog(settingsLogger->getEffectiveLevel(), msg.str());
}

void LogPowerRaminController::setValueToCtrl()
{
	setButtonCheck(
		m_dlg->m_batteryRemainLidOpenClose,
		checkFlag(*m_value, CMySettings::Trigger::LidOpenClose)
	);
	setButtonCheck(
		m_dlg->m_batteryRemainPowerSourceChanged,
		checkFlag(*m_value, CMySettings::Trigger::PowerSourceChanged)
	);
}

void LogPowerRaminController::getValueFromCtrl()
{
	m_value = getButtonStates();
}

bool LogPowerRaminController::isChanged() const
{
	return (m_value != getButtonStates()) || m_value.isChanged();
}

CMySettings::Trigger LogPowerRaminController::getButtonStates() const
{
	auto ret(CMySettings::Trigger::None);
	setFlag(ret, CMySettings::Trigger::LidOpenClose, m_dlg->m_batteryRemainLidOpenClose.GetCheck());
	setFlag(ret, CMySettings::Trigger::PowerSourceChanged, m_dlg->m_batteryRemainPowerSourceChanged.GetCheck());
	return ret;
}
