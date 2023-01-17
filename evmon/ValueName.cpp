#include "ValueName.h"

#include <memory>
#include <atlbase.h>

/*static*/ size_t CString::MaxFormatLength = 0x100;

CString& CString::operator=(const CString& that)
{
	this->assign(that);
	return *this;
}
CString CString::operator+(const CString& that) const
{
	CString ret(*this);
	ret += that.c_str();
	return ret;
}

CString& CString::operator+=(const CString& that)
{
	this->std::tstring::operator+=(that);
	return *this;
}

void CString::Format(LPCTSTR fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	FormatV(fmt, args);
	va_end(args);
}

void CString::FormatV(LPCTSTR fmt, va_list args)
{
	auto str = std::make_unique<TCHAR[]>(MaxFormatLength + 1);
	_vstprintf_s(str.get(), MaxFormatLength, fmt, args);
	assign(str.get());
}

CString CTime::Format(LPCTSTR fmt)
{
	struct tm localTime;
	localtime_s(&localTime, &m_time);

	TCHAR str[100];
	_tcsftime(str, ARRAYSIZE(str), fmt, &localTime);
	return str;
}

/*static*/ CTime CTime::CurrentTime()
{
	CTime obj;
	time(&obj.m_time);
	return obj;
}


// Returns a string that represents the GUID value.
template<>
/*static*/ CString ValueName<GUID>::valueToString(const GUID& value)
{
	OLECHAR strGuid[50] = _T("");
	HR_EXPECT(0 < StringFromGUID2(value, strGuid, ARRAYSIZE(strGuid)), HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER));
	CW2T ret(strGuid);
	return (LPCTSTR)ret;
}
