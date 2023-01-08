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

int _tmain(int argc, TCHAR** argv)
{
	_tsetlocale(LC_ALL, _T(""));
	tsm::Assert::onAssertFailedProc = AssertFailedProc;
	tsm::Assert::onAssertFailedWriter = [](LPCTSTR msg) { _putts(msg); };

	WNDCLASS wc = {0};
	wc.lpfnWndProc = WndProc;
	wc.lpszClassName = _T("Message-Only Window");
	wc.hInstance = GetModuleHandle(NULL);
	auto atom = RegisterClass(&wc);
	WIN32_ASSERT(atom != 0);
	auto hwnd = CreateWindow((LPCTSTR)atom, NULL, 0, 0, 0, 0, 0, HWND_MESSAGE, NULL, NULL, NULL);
	WIN32_ASSERT(hwnd != NULL);

	_putts(_T("Press CTRL+C to exit."));
	MSG msg;
	while(GetMessage(&msg, hwnd, 0, 0)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
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
		VALUE_NAME_ITEM(PBT_POWERSETTINGCHANGE),
	};

	print(_T("WM_POWERBROADCAST Event=%s"), ValueToString(events, ev).GetString());

	switch(ev) {
	case PBT_POWERSETTINGCHANGE:
		{
			auto setting = (POWERBROADCAST_SETTING*)param;
		}
		break;
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
		VALUE_NAME_ITEM(WM_CREATE),
		VALUE_NAME_ITEM(WM_GETMINMAXINFO),
		VALUE_NAME_ITEM(WM_NCCREATE),
		VALUE_NAME_ITEM(WM_NCCALCSIZE),
		VALUE_NAME_ITEM(WM_POWERBROADCAST),
		VALUE_NAME_ITEM(WM_POWER),
	};

	print(_T(__FUNCTION__ " %s"), ValueToString(msgs, id).GetString());
	switch(id) {
		HANDLE_MSG(hwnd, WM_POWERBROADCAST, OnWmPowerBroadCast);
		HANDLE_MSG(hwnd, WM_POWER, OnWmPower);
	default:
		return DefWindowProc(hwnd, id, wParam, lParam);
	}
}

// Print current date/time and formatted text.
static void print(LPCTSTR format, ...)
{
	CString text;
	va_list args;
	va_start(args, format);
	text.FormatV(format, args);
	va_end(args);

	_tprintf_s(_T("%s %s\n"), CTime::CurrentTime().Format(_T("%F %T")).GetString(), text.GetString());
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
