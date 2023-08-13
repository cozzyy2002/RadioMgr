#include "pch.h"
#include "MySettings.h"

// CMySettings members

CMySettings::CMySettings(LPCTSTR companyName, LPCTSTR applicationName)
	: settings(companyName, applicationName)
	, switchByLcdState(_T("SwitchByLcdState"), TRUE)
	, restoreRadioState(_T("RestoreRadioState"), TRUE)
	, setRadioOnDelay(_T("SetRadioOnDelay"))
	, setRadioStateTimeout(_T("SetRadioStateTimeout"), 1)
	, autoCheckRadioInstance(_T("AutoCheckRadioInstance"))
	, autoSelectDevice(_T("AudoSelectDevice"), TRUE)
	, saveWindowPlacement(_T("SaveWindowPlacement"))
	, windowPlacement(_T("WindowPlacement"), this)
	, debugSwitches(_T("DebugSwitches"))
{
}

void CMySettings::load()
{
	CSettings::IValue* valueList[] = {
		&switchByLcdState,
		&restoreRadioState,
		&setRadioOnDelay,
		&setRadioStateTimeout,
		&autoCheckRadioInstance,
		&autoSelectDevice,
		&saveWindowPlacement,
		&windowPlacement,
	};
	HR_EXPECT_OK(settings.load(valueList));
}

void CMySettings::save()
{
	HR_EXPECT_OK(settings.save());
}

bool CMySettings::isChanged(const WINDOWPLACEMENT& a, const WINDOWPLACEMENT& b)
{
	// If saving window position is not necessary, return as unchnged.
	if(!saveWindowPlacement) { return false; }

	return
		(a.showCmd != b.showCmd)
		|| (a.rcNormalPosition.left != b.rcNormalPosition.left)
		|| (a.rcNormalPosition.top != b.rcNormalPosition.top)
		|| (a.rcNormalPosition.right != b.rcNormalPosition.right)
		|| (a.rcNormalPosition.bottom != b.rcNormalPosition.bottom)
		;
}

CString CMySettings::valueToString(const CSettings::BinaryValue<WINDOWPLACEMENT>& value) const
{
	auto& wp = (const WINDOWPLACEMENT&)value;
	auto& rc = wp.rcNormalPosition;
	CString str;
	str.Format(_T("Position=%d:%d, Size=%dx%d, showCmd=%d"), rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, wp.showCmd);
	return str;
}

template<>
const DWORD CSettings::BinaryValue<WINDOWPLACEMENT>::RegType = REG_BINARY;

bool CMySettings::isEnabled(DebugSwitch flag)
{
	debugSwitches.read(&settings);
	return (flag & debugSwitches);
}
