#pragma once

#include <memory>

template<typename H>
using SafeHandleDeleteFunc = void (*)(H);

template<typename H, SafeHandleDeleteFunc<H> delFunc>
class SafeHandle {
public:
	SafeHandle(H h = (H)0) : m_handle(h) {}
	SafeHandle& operator=(H h) {
		m_handle.reset(h);
		return *this;
	}
	operator H() const { return m_handle.get(); }
	operator bool() const { return m_handle.operator bool(); }

	SafeHandle(SafeHandle&&) = delete;
	SafeHandle& operator=(SafeHandle&&) = delete;
	SafeHandle(const SafeHandle&) = delete;
	SafeHandle& operator=(const SafeHandle&) = delete;

protected:
	struct Deleter {
		using pointer = H;
		void operator()(H h) { delFunc(h); }
	};

	std::unique_ptr<H, Deleter> m_handle;
};
