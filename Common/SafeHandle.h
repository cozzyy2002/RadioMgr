#pragma once

#include <memory>

// To enable logging of SafeHandle<T> class:
//	Include header files of log4cxx prior to this file.
//	Declare `logger` satic member as follows:
//		/*static*/ log4cxx::LoggerPtr& SafeHandle<T>::logger(log4cxx::Logger::getLogger("Logger name")));
#if !defined(SAFE_HANDLE_LOG)
#if defined(LOG4CXX_LOG4CXX_H)
#define SAFE_HANDLE_LOG 1
#else
#define SAFE_HANDLE_LOG 0
#endif
#endif

template<typename H>
using SafeHandleDeleteFunc = void (*)(H);

template<typename H, SafeHandleDeleteFunc<H> delFunc>
class SafeHandle {
public:
	SafeHandle(H h = (H)0) : m_handle(h)
	{
#if SAFE_HANDLE_LOG
		LOG4CXX_DEBUG(logger, _T("SafeHandle created: HANDLE = ") << h);
#endif
	}
	SafeHandle(SafeHandle&& src) noexcept : m_handle(src.m_handle.release())
	{
#if SAFE_HANDLE_LOG
		LOG4CXX_DEBUG(logger, _T("SafeHandle moved: HANDLE = ") << m_handle.get());
#endif
	}
	SafeHandle& operator=(H h) {
#if SAFE_HANDLE_LOG
		LOG4CXX_DEBUG(logger, _T("SafeHandle substitutd: HANDLE = ") << h);
#endif
		m_handle.reset(h);
		return *this;
	}
	operator H() const { return m_handle.get(); }
	operator bool() const { return m_handle.operator bool(); }

	SafeHandle& operator=(SafeHandle&&) = delete;
	SafeHandle(const SafeHandle&) = delete;
	SafeHandle& operator=(const SafeHandle&) = delete;

protected:
#if SAFE_HANDLE_LOG
	static log4cxx::LoggerPtr& logger;
#endif

	struct Deleter {
		using pointer = H;
		void operator()(H h) {
#if SAFE_HANDLE_LOG
			LOG4CXX_DEBUG(logger, _T("SafeHandle deleted: HANDLE = ") << h);
#endif
			delFunc(h);
		}
	};

	std::unique_ptr<H, Deleter> m_handle;
};
