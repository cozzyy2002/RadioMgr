#pragma once

#include "../Common/Assert.h"

template<typename T>
struct ValueName {
	T value;
	LPCTSTR name;
	LPCTSTR description;

	static LPCTSTR StringFormat;
	template<size_t size>
	static CString getName(const ValueName(&list)[size], const T& v);
	static CString valueToString(const T& v);
};


template<typename T> LPCTSTR ValueName<T>::StringFormat = _T("%d");
template<> LPCTSTR ValueName<UINT>::StringFormat = _T("0x%04X");
template<> LPCTSTR ValueName<float>::StringFormat = _T("%f");

template<typename T> template<size_t size>
/*static*/ CString ValueName<T>::getName(const ValueName(&list)[size], const T& v)
{
	auto name = _T("UNKNOWN");
	CString desc;
	for(auto& i : list) {
		if(i.value == v) {
			name = i.name;
			if(i.description) {
				desc.Format(_T(":%s"), i.description);
			}
			break;
		}
	}
	CString ret;
	ret.Format(_T("%s(%s)%s"), name, valueToString(v).GetString(), desc.GetString());
	return ret;
}

template<typename T>
/*static*/ CString ValueName<T>::valueToString(const T& v)
{
	CString ret;
	ret.Format(StringFormat, v);
	return ret;
}

template<>
/*static*/ CString ValueName<GUID>::valueToString(const GUID& v)
{
	OLECHAR strGuid[50] = _T("");
	HR_EXPECT(0 < StringFromGUID2(v, strGuid, ARRAYSIZE(strGuid)), HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER));
	return strGuid;
}

#define VALUE_NAME_ITEM(x, ...) {x, _T(#x), __VA_ARGS__}

