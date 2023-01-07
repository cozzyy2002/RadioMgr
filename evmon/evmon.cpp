// evmon.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <Windows.h>
#include <tchar.h>
#include <locale.h>

#include "../Common/SafeHandle.h"
#include "../Common/Assert.h"

static void AssertFailedProc(HRESULT hr, LPCTSTR exp, LPCTSTR sourceFile, int line);
static LPTSTR trim(LPTSTR str, int len);

int main()
{
	_tsetlocale(LC_ALL, _T(""));
	tsm::Assert::onAssertFailedProc = AssertFailedProc;
	tsm::Assert::onAssertFailedWriter = [](LPCTSTR msg) { _putts(msg); };

	auto hwnd = CreateWindow(NULL, NULL, 0, 0, 0, 0, 0, HWND_MESSAGE, NULL, NULL, NULL);
	WIN32_ASSERT(hwnd);

	WIN32_ASSERT(DestroyWindow(hwnd));
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
