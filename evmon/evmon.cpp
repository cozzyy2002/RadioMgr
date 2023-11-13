// evmon.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <Windows.h>
#include <windowsx.h>
#include <tchar.h>
#include <locale.h>
#include <vector>
#include <memory>

#include "../Common/Assert.h"
#include "ValueName.h"
#include "NetworkEvents.h"
#include "IpAdapterAddresses.h"
#include "WLan.h"

enum {
	WM_NETWORK_CONNECTIVITYCHANGED = WM_USER + 1,	// Notification of NetworkListManagerEvents
	WM_WLAN_NOTIFY,									// Notification of Wlan
};

extern void print(LPCTSTR format, ...);

static LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
static void AssertFailedProc(HRESULT hr, LPCTSTR exp, LPCTSTR sourceFile, int line);
static LPTSTR trim(LPTSTR str, int len);
static BOOL WINAPI CtrlCHandler(DWORD ctrlType);

static HWND g_hwnd(NULL);

using PowerSettingFunc = CString (*)(POWERBROADCAST_SETTING*);

static CString AcDcPowerSource(POWERBROADCAST_SETTING* setting)
{
	static const ValueName<SYSTEM_POWER_CONDITION> values[] = {
		VALUE_NAME_ITEM(PoAc),
		VALUE_NAME_ITEM(PoDc),
		VALUE_NAME_ITEM(PoHot),
	};
	return ValueToString(values, *(SYSTEM_POWER_CONDITION*)setting->Data);
}

static CString BatteryPercentageRemaining(POWERBROADCAST_SETTING* setting)
{
	CString str;
	str.Format(_T("%d%%"), *(DWORD*)setting->Data);
	return str;
}

static CString ConsoleDisplayState(POWERBROADCAST_SETTING* setting)
{
	static const ValueName<DWORD> values[] = {
		{0x0, _T("The display is off")},
		{0x1, _T("The display is on")},
		{0x2, _T("The display is dimmed")},
	};
	return ValueToString(values, *(DWORD*)setting->Data);
}

static CString IdleBackgroundTask(POWERBROADCAST_SETTING* setting)
{
	// setting->Data has no infomation.
	return _T("No data");
}

static CString MonitorPowerOn(POWERBROADCAST_SETTING* setting)
{
	static const ValueName<DWORD> values[] = {
		{0x0, _T("The monitor is off")},
		{0x1, _T("The monitor is on")},
	};
	return ValueToString(values, *(DWORD*)setting->Data);
}

static CString PowerSavingStatus(POWERBROADCAST_SETTING* setting)
{
	static const ValueName<DWORD> values[] = {
		{0x0, _T("The Battery saver is off")},
		{0x1, _T("The Battery saver is on. Save energy where possible")},
	};
	return ValueToString(values, *(DWORD*)setting->Data);
}

static CString PowerSchemePersonality(POWERBROADCAST_SETTING* setting)
{
	static const ValueName<GUID> values[] = {
		VALUE_NAME_ITEM(GUID_MIN_POWER_SAVINGS),
		VALUE_NAME_ITEM(GUID_MAX_POWER_SAVINGS),
		VALUE_NAME_ITEM(GUID_TYPICAL_POWER_SAVINGS),
	};
	return ValueToString(values, *(GUID*)setting->Data);
}

static CString SessionDisplayStatus(POWERBROADCAST_SETTING* setting)
{
	return ConsoleDisplayState(setting);
}

static CString SessionUserPresence(POWERBROADCAST_SETTING* setting)
{
	static const ValueName<USER_ACTIVITY_PRESENCE> values[] = {
		VALUE_NAME_ITEM(PowerUserPresent),
		VALUE_NAME_ITEM(PowerUserNotPresent),
		VALUE_NAME_ITEM(PowerUserInactive),
	};
	return ValueToString(values, *(USER_ACTIVITY_PRESENCE*)setting->Data);
}

//static CString LidCloseAction(POWERBROADCAST_SETTING* setting);
//static CString LidOpenPowerState(POWERBROADCAST_SETTING* setting);
//static CString LidSwitchStateReliability(POWERBROADCAST_SETTING* setting);

static CString LidSwitchStateChange(POWERBROADCAST_SETTING* setting)
{
	static const ValueName<DWORD> values[] = {
		{0x0, _T("The lid is closed")},
		{0x1, _T("The lid is opened")},
	};
	return ValueToString(values, *(DWORD*)setting->Data);
}
static CString SystmAwayMode(POWERBROADCAST_SETTING* setting)
{
	static const ValueName<DWORD> values[] = {
		{0x0, _T("The computer is exiting away mode")},
		{0x1, _T("The computer is entering away mode")},
	};
	return ValueToString(values, *(DWORD*)setting->Data);
}

CString UnknownPowerStateData(POWERBROADCAST_SETTING* setting)
{
	DWORD data = 0;
	switch(setting->DataLength) {
	default:
	case sizeof(BYTE) : data = setting->Data[0]; break;
	case sizeof(WORD) : data = *(WORD*)(setting->Data); break;
	case sizeof(DWORD) : data = *(DWORD*)(setting->Data); break;
	}
	CString str;
	str.Format(_T("DataLength=%d, Data=%d"), setting->DataLength, data);
	return str;
}

// Power Setting GUIDs.
// See https://learn.microsoft.com/en-us/windows/win32/power/power-setting-guids
static const ValueName<GUID> PowerSettings[] = {
	VALUE_NAME_ITEM(GUID_ACDC_POWER_SOURCE, AcDcPowerSource),
	VALUE_NAME_ITEM(GUID_BATTERY_PERCENTAGE_REMAINING, BatteryPercentageRemaining),
	VALUE_NAME_ITEM(GUID_CONSOLE_DISPLAY_STATE, ConsoleDisplayState),
	VALUE_NAME_ITEM(GUID_IDLE_BACKGROUND_TASK, IdleBackgroundTask),
	VALUE_NAME_ITEM(GUID_MONITOR_POWER_ON, MonitorPowerOn),
	VALUE_NAME_ITEM(GUID_POWER_SAVING_STATUS, PowerSavingStatus),
	VALUE_NAME_ITEM(GUID_POWERSCHEME_PERSONALITY, PowerSchemePersonality),
	VALUE_NAME_ITEM(GUID_SESSION_DISPLAY_STATUS, SessionDisplayStatus),
	VALUE_NAME_ITEM(GUID_SESSION_USER_PRESENCE, SessionUserPresence),
	VALUE_NAME_ITEM(GUID_LIDCLOSE_ACTION, UnknownPowerStateData),
	VALUE_NAME_ITEM(GUID_LIDOPEN_POWERSTATE, UnknownPowerStateData),
	VALUE_NAME_ITEM(GUID_LIDSWITCH_STATE_RELIABILITY, UnknownPowerStateData),
	VALUE_NAME_ITEM(GUID_LIDSWITCH_STATE_CHANGE, LidSwitchStateChange),
	VALUE_NAME_ITEM(GUID_SYSTEM_AWAYMODE, SystmAwayMode),
};

struct PowerNotifyHandleDeleter {
	using pointer = HPOWERNOTIFY;
	void operator()(HPOWERNOTIFY handle)
	{
		WIN32_EXPECT(UnregisterPowerSettingNotification(handle));
	}
};

int _tmain(int argc, TCHAR** argv)
{
	_tsetlocale(LC_ALL, _T(""));
	tsm::Assert::onAssertFailedProc = AssertFailedProc;
	tsm::Assert::onAssertFailedWriter = [](LPCTSTR msg) { _putts(msg); };

	print(_T("---- IpAdapterAddresses ----"));
	IpAdapterAddresses ipa;
	ipa.update();

	WNDCLASS wc = {0};
	wc.lpfnWndProc = WndProc;
	wc.lpszClassName = _T("Message-Only Window");
	wc.hInstance = GetModuleHandle(NULL);
	auto atom = RegisterClass(&wc);
	WIN32_ASSERT(atom != 0);
	g_hwnd = CreateWindow((LPCTSTR)atom, NULL, 0, 0, 0, 0, 0, HWND_MESSAGE, NULL, NULL, NULL);
	WIN32_ASSERT(g_hwnd != NULL);

	print(_T("---- WLan ----"));
	WLan wlan;
	wlan.start(g_hwnd, WM_WLAN_NOTIFY);
	wlan.update();

	// Register power setting notification.
	using PowerNotifyHandle = std::unique_ptr<HPOWERNOTIFY, PowerNotifyHandleDeleter>;
	std::vector<PowerNotifyHandle> handles(ARRAYSIZE(PowerSettings));
	for(auto& s : PowerSettings) {
		auto h = RegisterPowerSettingNotification(g_hwnd, &s.value, DEVICE_NOTIFY_WINDOW_HANDLE);
		WIN32_ASSERT(h != NULL);
		handles.push_back(PowerNotifyHandle(h));
	}

	HR_ASSERT_OK(CoInitializeEx(NULL, COINIT_MULTITHREADED));
	{
		// This block ensures that all IUnknown objects held by CComPtr<> are released before calling CoUninitialize().

		// Start listening events of INetworkListManager.
		CComPtr<NetworkListManagerEvents> netEvents(new NetworkListManagerEvents());
		HR_ASSERT_OK(netEvents->start(g_hwnd, WM_NETWORK_CONNECTIVITYCHANGED));

		WIN32_ASSERT(SetConsoleCtrlHandler(CtrlCHandler, TRUE));
		_putts(_T("Press CTRL+C to exit."));
		MSG msg;
		BOOL getMessageResult;
		while((getMessageResult = GetMessage(&msg, g_hwnd, 0, 0)) != 0) {
			if(FAILED(WIN32_EXPECT(getMessageResult != -1))) { break; }
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		HR_EXPECT_OK(netEvents->stop());
	}
	CoUninitialize();

	WIN32_EXPECT(DestroyWindow(g_hwnd));

	return 0;
}

static BOOL WINAPI CtrlCHandler(DWORD ctrlType)
{
	ValueName<DWORD>::StringFormat = _T("%d");
	static const ValueName<DWORD> types[] = {
		VALUE_NAME_ITEM(CTRL_C_EVENT),
		VALUE_NAME_ITEM(CTRL_BREAK_EVENT),
		VALUE_NAME_ITEM(CTRL_CLOSE_EVENT),
		VALUE_NAME_ITEM(CTRL_LOGOFF_EVENT),
		VALUE_NAME_ITEM(CTRL_SHUTDOWN_EVENT),
	};

	print(_T(__FUNCTION__ ": %s"), ValueToString(types, ctrlType).GetString());

	auto hr = WIN32_EXPECT(PostMessage(g_hwnd, WM_CLOSE, 0, 0L));
	return (SUCCEEDED(hr) ? TRUE : FALSE);
}

static void PowerSettingChangePrameFunc(LPVOID param)
{
	auto setting = (POWERBROADCAST_SETTING*)param;
	auto valueName = FindValueName(PowerSettings, setting->PowerSetting);
	auto func = (PowerSettingFunc)valueName.param;
	print(_T("%s: %s"), valueName.name, (func ? func(setting) : UnknownPowerStateData(setting)).GetString());
}

#define HANDLE_WM_POWERBROADCAST(hwnd, wParam, lParam, fn) (LRESULT)(fn)((hwnd), (UINT)(wParam), (LPVOID)(lParam))
static LRESULT OnWmPowerBroadCast(HWND hwnd, UINT ev, LPVOID param)
{
	ValueName<UINT>::StringFormat = _T("0x%x");
	static const ValueName<UINT> events[] = {
		VALUE_NAME_ITEM(PBT_APMPOWERSTATUSCHANGE),
		VALUE_NAME_ITEM(PBT_APMRESUMEAUTOMATIC),
		VALUE_NAME_ITEM(PBT_APMRESUMESUSPEND),
		VALUE_NAME_ITEM(PBT_APMSUSPEND),
		VALUE_NAME_ITEM(PBT_POWERSETTINGCHANGE, PowerSettingChangePrameFunc),
	};

	auto& eventValueName = FindValueName(events, ev);
	print(_T("WM_POWERBROADCAST Event=%s"), eventValueName.toString().GetString());

	using ParamFunc = void (*)(LPVOID);
	auto paramFunc = (ParamFunc)eventValueName.param;
	if(paramFunc) {
		paramFunc(param);
	}

	return FALSE;
}

static void OnWmPower(HWND, int code)
{
	ValueName<int>::StringFormat = _T("%d");
	static const ValueName<int> codes[] = {
		VALUE_NAME_ITEM(PWR_CRITICALRESUME),
		VALUE_NAME_ITEM(PWR_SUSPENDREQUEST),
		VALUE_NAME_ITEM(PWR_SUSPENDRESUME),
	};

	print(_T("WM_POWER %s"), ValueToString(codes, code).GetString());
}

static void OnWmNetworkConnectivityChanged(HWND, NLM_CONNECTIVITY conn)
{
	ValueName<NLM_CONNECTIVITY>::StringFormat = _T("0x%x");
	static const ValueName<NLM_CONNECTIVITY> values[] = {
		VALUE_NAME_ITEM(NLM_CONNECTIVITY_DISCONNECTED),
		VALUE_NAME_ITEM(NLM_CONNECTIVITY_IPV4_NOTRAFFIC),
		VALUE_NAME_ITEM(NLM_CONNECTIVITY_IPV6_NOTRAFFIC),
		VALUE_NAME_ITEM(NLM_CONNECTIVITY_IPV4_SUBNET),
		VALUE_NAME_ITEM(NLM_CONNECTIVITY_IPV4_LOCALNETWORK),
		VALUE_NAME_ITEM(NLM_CONNECTIVITY_IPV4_INTERNET),
		VALUE_NAME_ITEM(NLM_CONNECTIVITY_IPV6_SUBNET),
		VALUE_NAME_ITEM(NLM_CONNECTIVITY_IPV6_LOCALNETWORK),
		VALUE_NAME_ITEM(NLM_CONNECTIVITY_IPV6_INTERNET),
	};

	print(_T("WM_NETWORK_CONNECTIVITYCHANGED %s"), FlagValueToString(values, conn).GetString());
}

static void OnWmClose(HWND)
{
	PostQuitMessage(0);
}

static LRESULT CALLBACK WndProc(HWND hwnd, UINT id, WPARAM wParam, LPARAM lParam)
{
	ValueName<UINT>::StringFormat = _T("0x%x");
	static const ValueName<UINT> msgs[] = {
		VALUE_NAME_ITEM(WM_POWERBROADCAST),
		VALUE_NAME_ITEM(WM_POWER),

		VALUE_NAME_ITEM(WM_CREATE),
		VALUE_NAME_ITEM(WM_GETMINMAXINFO),
		VALUE_NAME_ITEM(WM_NCCREATE),
		VALUE_NAME_ITEM(WM_NCCALCSIZE),
		VALUE_NAME_ITEM(WM_DESTROY),
		VALUE_NAME_ITEM(WM_NCDESTROY),
		VALUE_NAME_ITEM(WM_CLOSE),
		VALUE_NAME_ITEM(WM_QUIT),
		{0x90, _T("WM_UAHDESTROYWINDOW")},
		VALUE_NAME_ITEM(WM_NETWORK_CONNECTIVITYCHANGED),
	};

	print(_T(__FUNCTION__ " %s"), ValueToString(msgs, id).GetString());
	switch(id) {
		HANDLE_MSG(hwnd, WM_POWERBROADCAST, OnWmPowerBroadCast);
		HANDLE_MSG(hwnd, WM_POWER, OnWmPower);
		HANDLE_MSG(hwnd, WM_CLOSE, OnWmClose);
		HANDLE_MSG(hwnd, WM_NETWORK_CONNECTIVITYCHANGED, OnWmNetworkConnectivityChanged);
	default:
		return DefWindowProc(hwnd, id, wParam, lParam);
	}
}

// Print current date/time, thread ID and formatted text.
void print(LPCTSTR format, ...)
{
	CString text;
	va_list args;
	va_start(args, format);
	text.FormatV(format, args);
	va_end(args);

	_tprintf_s(_T("%s [%d] %s\n"), CTime::CurrentTime().Format(_T("%F %T")).GetString(), GetCurrentThreadId(), text.GetString());
}

// Output failed message with formatted HRESULT to the console.
static void AssertFailedProc(HRESULT hr, LPCTSTR exp, LPCTSTR sourceFile, int line)
{
	LPTSTR msg;
	DWORD flags = FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS;
	auto formatResult = FormatMessage(flags, NULL, hr, MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL), (LPTSTR)&msg, 100, NULL);
	if(formatResult) {
		_tprintf_s(_T("`%s` failed: %s(0x%08x)\n%s(Line %d)\n"), exp, trim(msg, formatResult - 1), hr, sourceFile, line);
		LocalFree(msg);
	} else {
		// Call default proc if CbtswwinDlg is not created yet or FormatMessage() failed.
		tsm::Assert::defaultAssertFailedProc(hr, exp, sourceFile, line);
	}
}

// Remove trailing CR/LF
static LPTSTR trim(LPTSTR str, int len)
{
	for(int i = len; 0 <= i; i--) {
		switch(str[i]) {
		case _T('\r'):
		case _T('\n'):
			str[i] = _T('\0');
			break;
		default:
			i = 0;
			break;
		}
	}
	return str;
}
