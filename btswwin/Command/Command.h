#pragma once

#include <memory>
#include <thread>

class CCommand
{
public:
	CCommand();

	struct ExecResult {
		explicit ExecResult() : exitCode(0) {}
		DWORD exitCode;
		CString cmd;
	};
	HRESULT exec(HWND hwnd, UINT messageId, LPCTSTR cmd);

protected:
	HWND m_hwnd;
	UINT m_messageId;

	std::unique_ptr<std::thread> m_execThread;
};
