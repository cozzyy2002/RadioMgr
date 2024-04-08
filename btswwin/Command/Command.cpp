#include "pch.h"
#include "Command.h"
#include "../Common/Assert.h"

static auto& logger(log4cxx::Logger::getLogger(_T("btswwin.CCommand")));

CCommand::CCommand()
	: m_hwnd(NULL), m_messageId(0)
{
}

// Creates process to execute command specified by the parameter.
// Notifies result code when the process exits. 
HRESULT CCommand::exec(HWND hwnd, UINT messageId, LPCTSTR cmd)
{
	m_hwnd = hwnd;
	m_messageId = messageId;

	LOG4CXX_INFO(logger, _T("Executing command: ") << cmd);
	CString _cmd(cmd);
	auto creationFlags =
		CREATE_DEFAULT_ERROR_MODE
#if defined(UNICODE)
		| CREATE_UNICODE_ENVIRONMENT
#endif
		;

	STARTUPINFO startupInfo{sizeof(startupInfo)};
	startupInfo.lpTitle = _T("btswwin");
	PROCESS_INFORMATION processInfo{0};
#pragma warning(suppress : 6335) 
	WIN32_ASSERT(
		CreateProcess(NULL, _cmd.GetBuffer(), NULL, NULL, FALSE, creationFlags, NULL, NULL, &startupInfo, &processInfo)
	);

	// Wait for created process to exit.
	auto result = new ExecResult();
	result->cmd = cmd;
	m_execThread = std::make_unique<std::thread>([this, processInfo, result]{
		auto wait = WaitForSingleObject(processInfo.hProcess, INFINITE);
		WIN32_EXPECT(wait == WAIT_OBJECT_0);
		WIN32_EXPECT(GetExitCodeProcess(processInfo.hProcess, &result->exitCode));
		LOG4CXX_INFO_FMT(logger, _T("Command exited: Code=%d, %s"), result->exitCode, result->cmd.GetString());
		CloseHandle(processInfo.hThread);
		CloseHandle(processInfo.hProcess);

		// Notify result by sending Window Message.
		if(FAILED(WIN32_EXPECT(PostMessage(m_hwnd, m_messageId, 0, (LPARAM)result)))) {
			delete result;
		}
	});
	return S_OK;
}
