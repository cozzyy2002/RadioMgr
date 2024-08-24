// MiscTabItem.cpp : implementation file
//

#include "pch.h"
#include "btswwin.h"
#include "afxdialogex.h"
#include "MiscTabItem.h"

#include <sstream>

// Logger used to log all setting value.
static auto& settingsLogger(log4cxx::Logger::getLogger(_T("btswwin")));

// CMiscTabItem dialog

IMPLEMENT_DYNAMIC(CMiscTabItem, CDialogEx)

CMiscTabItem::CMiscTabItem(CMySettings& settings)
	: CTabItem(IDD_SETTINGS_MISC, _T("Miscellaneous"), settings)
{
	addController(m_settings.saveWindowPlacement, m_saveWindowPlacement);
}

void CMiscTabItem::updateUIState()
{
}

void CMiscTabItem::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_CHECK_SAVE_WINDOW_PLACEMENT, m_saveWindowPlacement);
}


BEGIN_MESSAGE_MAP(CMiscTabItem, CDialogEx)
	ON_BN_CLICKED(IDC_CHECK_SAVE_WINDOW_PLACEMENT, &CMiscTabItem::OnClickedCheckButton)
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
