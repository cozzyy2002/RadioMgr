#pragma once

#include <memory>

template<typename H>
using SafeHandleDeleteFunc = void (*)(H);

template<typename H, SafeHandleDeleteFunc<H> delFunc>
class SafeHandle {
public:
	SafeHandle(H h = (H)0) : m_handle(h)
	{
		LOG4CXX_DEBUG(logger, _T("SafeHandle created: HANDLE = ") << h);
	}
	SafeHandle(SafeHandle&& src) noexcept : m_handle(src.m_handle.release())
	{
		LOG4CXX_DEBUG(logger, _T("SafeHandle copied: HANDLE = ") << m_handle.get());
	}
	SafeHandle& operator=(H h) {
		LOG4CXX_DEBUG(logger, _T("SafeHandle substitutd: HANDLE = ") << h);
		m_handle.reset(h);
		return *this;
	}
	operator H() const { return m_handle.get(); }
	operator bool() const { return m_handle.operator bool(); }

	SafeHandle& operator=(SafeHandle&&) = delete;
	SafeHandle(const SafeHandle&) = delete;
	SafeHandle& operator=(const SafeHandle&) = delete;

protected:
	static log4cxx::LoggerPtr& logger;

	struct Deleter {
		using pointer = H;
		void operator()(H h) {
			LOG4CXX_DEBUG(logger, _T("SafeHandle deleted: HANDLE = ") << h);
			delFunc(h);
		}
	};

	std::unique_ptr<H, Deleter> m_handle;
};
