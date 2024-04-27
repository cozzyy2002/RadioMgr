#include "pch.h"
#include "Command.h"
#include "../Common/Assert.h"

static auto& logger(log4cxx::Logger::getLogger(_T("btswwin.CCommand")));

CCommand::CCommand()
	: m_hwnd(NULL), m_messageId(0), exitCode(0)
{
}

// Creates process to execute command specified by the parameter.
// Notifies result code when the process exits. 
HRESULT CCommand::exec(HWND hwnd, UINT messageId, LPCTSTR cmd)
{
	HR_ASSERT(!m_execThread, E_ILLEGAL_METHOD_CALL);

	m_hwnd = hwnd;
	m_messageId = messageId;
	this->cmd = cmd;

	LOG4CXX_DEBUG_FMT(logger, _T("Executing command: `%s`"), cmd);
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
		CreateProcess(NULL, this->cmd.GetBuffer(), NULL, NULL, FALSE, creationFlags, NULL, NULL, &startupInfo, &processInfo)
	);

	// Wait for created process to exit on worker thread.
	m_execThread = std::make_unique<std::thread>([this, processInfo]{
		auto wait = WaitForSingleObject(processInfo.hProcess, INFINITE);
		WIN32_EXPECT(wait == WAIT_OBJECT_0);
		WIN32_EXPECT(GetExitCodeProcess(processInfo.hProcess, &this->exitCode));
		LOG4CXX_DEBUG_FMT(logger, _T("Command exited: Code=%d, `%s`"), this->exitCode, this->cmd.GetString());
		CloseHandle(processInfo.hThread);
		CloseHandle(processInfo.hProcess);

		// Notify result by sending Window Message.
		WIN32_EXPECT(PostMessage(m_hwnd, m_messageId, 0, (LPARAM)this));
	});
	return S_OK;
}

// Returns `this` pointer from window message sent by exec() method.
// This method is supposed to be called by the message handler.
CCommand* CCommand::getInstanceFromNotify(WPARAM wParam, LPARAM lParam)
{
	auto _this = (CCommand*)lParam;
	if(_this && _this->m_execThread && _this->m_execThread->joinable()) {
		// Wait for worker thread to exit.
		_this->m_execThread->join();
	}
	return _this;
}
