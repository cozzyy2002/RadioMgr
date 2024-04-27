#pragma once

#include <memory>
#include <thread>

class CCommand
{
public:
	CCommand();
	virtual ~CCommand() {}

	HRESULT exec(HWND hwnd, UINT messageId, LPCTSTR cmd);
	DWORD exitCode;
	CString cmd;

	static CCommand* getInstanceFromNotify(WPARAM wParam, LPARAM lParam);

protected:
	HWND m_hwnd;
	UINT m_messageId;

	std::unique_ptr<std::thread> m_execThread;
};
