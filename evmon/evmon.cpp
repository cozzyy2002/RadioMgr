// evmon.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <Windows.h>
#include <windowsx.h>
#include <tchar.h>
#include <locale.h>

#include "../Common/SafeHandle.h"
#include "../Common/Assert.h"
#include "ValueName.h"

static LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
static void AssertFailedProc(HRESULT hr, LPCTSTR exp, LPCTSTR sourceFile, int line);
static LPTSTR trim(LPTSTR str, int len);
static void print(LPCTSTR format, ...);
static BOOL WINAPI CtrlCHandler(DWORD ctrlType);

static HWND g_hwnd(NULL);

int _tmain(int argc, TCHAR** argv)
{
	_tsetlocale(LC_ALL, _T(""));
	tsm::Assert::onAssertFailedProc = AssertFailedProc;
	tsm::Assert::onAssertFailedWriter = [](LPCTSTR msg) { _putts(msg); };

	WIN32_ASSERT(SetConsoleCtrlHandler(CtrlCHandler, TRUE));

	WNDCLASS wc = {0};
	wc.lpfnWndProc = WndProc;
	wc.lpszClassName = _T("Message-Only Window");
	wc.hInstance = GetModuleHandle(NULL);
	auto atom = RegisterClass(&wc);
	WIN32_ASSERT(atom != 0);
	g_hwnd = CreateWindow((LPCTSTR)atom, NULL, 0, 0, 0, 0, 0, HWND_MESSAGE, NULL, NULL, NULL);
	WIN32_ASSERT(g_hwnd != NULL);

	_putts(_T("Press CTRL+C to exit."));
	MSG msg;
	BOOL getMessageResult;
	while((getMessageResult = GetMessage(&msg, g_hwnd, 0, 0)) != 0) {
		if(FAILED(WIN32_EXPECT(getMessageResult != -1))) { break; }
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	WIN32_ASSERT(DestroyWindow(g_hwnd));
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

template<typename T>
using ParamFunc = void (*)(T*);

void PowerSettingChangePrameFunc(POWERBROADCAST_SETTING* setting)
{
	static const ValueName<GUID> PowerSettings[] = {
		VALUE_NAME_ITEM(GUID_ACDC_POWER_SOURCE),
		VALUE_NAME_ITEM(GUID_BATTERY_PERCENTAGE_REMAINING),
		VALUE_NAME_ITEM(GUID_CONSOLE_DISPLAY_STATE),
		VALUE_NAME_ITEM(GUID_IDLE_BACKGROUND_TASK),
		VALUE_NAME_ITEM(GUID_MONITOR_POWER_ON),
		VALUE_NAME_ITEM(GUID_POWER_SAVING_STATUS),
		VALUE_NAME_ITEM(GUID_POWERSCHEME_PERSONALITY),
		VALUE_NAME_ITEM(GUID_MIN_POWER_SAVINGS),
		VALUE_NAME_ITEM(GUID_MAX_POWER_SAVINGS),
		VALUE_NAME_ITEM(GUID_TYPICAL_POWER_SAVINGS),
		VALUE_NAME_ITEM(GUID_SESSION_DISPLAY_STATUS),
		VALUE_NAME_ITEM(GUID_SESSION_USER_PRESENCE),
		VALUE_NAME_ITEM(GUID_SYSTEM_AWAYMODE),
	};

	DWORD data = 0;
	switch(setting->DataLength) {
	case sizeof(BYTE) : data = setting->Data[0]; break;
	case sizeof(WORD) : data = *(WORD*)(setting->Data); break;
	case sizeof(DWORD): data = *(DWORD*)(setting->Data); break;
	}
	print(_T("%s: DataLength=%d, Data=%d"), ValueToString(PowerSettings, setting->PowerSetting).GetString(), setting->DataLength, data);
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
		VALUE_NAME_ITEM(PBT_POWERSETTINGCHANGE, nullptr, PowerSettingChangePrameFunc),
	};

	auto& eventValueName = FindValueName(events, ev);
	print(_T("WM_POWERBROADCAST Event=%s"), eventValueName.toString().GetString());
	auto paramFunc = (ParamFunc<POWERBROADCAST_SETTING>)eventValueName.param;
	if(paramFunc) {
		auto setting = (POWERBROADCAST_SETTING*)param;
		paramFunc(setting);
	}

	return FALSE;
}

void OnWmPower(HWND, int code)
{
	ValueName<int>::StringFormat = _T("%d");
	static const ValueName<int> codes[] = {
		VALUE_NAME_ITEM(PWR_CRITICALRESUME, _T("Resuming after suspended mode without broadcasting PWR_SUSPENDREQUEST.")),
		VALUE_NAME_ITEM(PWR_SUSPENDREQUEST, _T("About to enter suspended mode.")),
		VALUE_NAME_ITEM(PWR_SUSPENDRESUME, _T("Resuming after normal suspended mode.")),
	};

	print(_T("WM_POWER %s"), ValueToString(codes, code).GetString());
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
	};

	print(_T(__FUNCTION__ " %s"), ValueToString(msgs, id).GetString());
	switch(id) {
		HANDLE_MSG(hwnd, WM_POWERBROADCAST, OnWmPowerBroadCast);
		HANDLE_MSG(hwnd, WM_POWER, OnWmPower);
	case WM_CLOSE:
		PostQuitMessage(0);
		return 0;
	default:
		return DefWindowProc(hwnd, id, wParam, lParam);
	}
}

// Print current date/time, thread ID and formatted text.
static void print(LPCTSTR format, ...)
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
		_tprintf_s(_T("`%s` failed: %s(0x%08x)\n%s(Line %d)"), exp, trim(msg, formatResult - 1), hr, sourceFile, line);
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
