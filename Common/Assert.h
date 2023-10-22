#pragma once

#include <Windows.h>
#include <tchar.h>

#define HR_ERROR(msg, hr) tsm::Assert::checkHResult(hr, msg, _T(__FILE__), __LINE__)
#define HR_ASSERT(exp, hr) do { auto _hr(HR_EXPECT(exp, hr)); if(FAILED(_hr)) return _hr; } while(false)
#define HR_EXPECT(exp, hr) tsm::Assert::checkHResult((exp) ? S_OK : hr, _T(#exp), _T(__FILE__), __LINE__)
#define HR_ASSERT_OK(exp) do { auto _hr(HR_EXPECT_OK(exp)); if(FAILED(_hr)) return _hr; } while(false)
#define HR_EXPECT_OK(exp) tsm::Assert::checkHResult(exp, _T(#exp), _T(__FILE__), __LINE__)
#define WIN32_ASSERT(exp) HR_ASSERT(exp, HRESULT_FROM_WIN32(GetLastError()))
#define WIN32_EXPECT(exp) HR_EXPECT(exp, HRESULT_FROM_WIN32(GetLastError()))

#define tsm_STATE_MACHINE_EXPORT

namespace tsm
{

class tsm_STATE_MACHINE_EXPORT Assert
{
public:
	static HRESULT checkHResult(HRESULT hr, LPCTSTR exp, LPCTSTR sourceFile, int line);

	// Procedure that is called when error occurs in ASSERT/EXPECT macro.
	using OnAssertFailedProc = void (*)(HRESULT hr, LPCTSTR exp, LPCTSTR sourceFile, int line);
	static OnAssertFailedProc onAssertFailedProc;
	static void defaultAssertFailedProc(HRESULT hr, LPCTSTR exp, LPCTSTR sourceFile, int line);

	// Procedure that is called by OnAssertFailedProc to write error message.
	using OnAssertFailedWriter = void (*)(LPCTSTR msg);
	static OnAssertFailedWriter onAssertFailedWriter;
	static void defaultAssertFailedWriter(LPCTSTR msg);
};

}

#define LOG4CXX_WARN_FMT(logger, fmt, ...) do { \
		if (logger->isWarnEnabled()) {\
			CString message; \
			message.Format(fmt, __VA_ARGS__); \
			logger->forcedLog(::log4cxx::Level::getWarn(), message.GetString(), LOG4CXX_LOCATION); }} while (0)

#define LOG4CXX_INFO_FMT(logger, fmt, ...) do { \
		if (logger->isInfoEnabled()) {\
			CString message; \
			message.Format(fmt, __VA_ARGS__); \
			logger->forcedLog(::log4cxx::Level::getInfo(), message.GetString(), LOG4CXX_LOCATION); }} while (0)

#define LOG4CXX_DEBUG_FMT(logger, fmt, ...) do { \
		if (logger->isDebugEnabled()) {\
			CString message; \
			message.Format(fmt, __VA_ARGS__); \
			logger->forcedLog(::log4cxx::Level::getDebug(), message.GetString(), LOG4CXX_LOCATION); }} while (0)
