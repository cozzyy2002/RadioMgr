#pragma once

#include "../Common/Assert.h"

extern CString guidToString(REFGUID);

//
// Template class that hold a value and it's name and description(optional).
//
template<typename T>
struct ValueName {
	T value;
	LPCTSTR name;
	LPCTSTR description;
	LPVOID param;

	static LPCTSTR StringFormat;
	static LPCTSTR UnknownValueName;

	CString toString(LPCTSTR stringFormat = nullptr) const;
	CString valueToString(LPCTSTR stringFormat = nullptr) const;
	static CString valueToString(const T& v, LPCTSTR stringFormat = nullptr);
};

// Macro for declaring ValueName table that passed to ValueToString() function.
// Optiional parameter can be specified to assign description string.
#define VALUE_NAME_ITEM(x, ...) {x, _T(#x), __VA_ARGS__}

// Default format for scalar type value used to convert to string.
template<typename T> LPCTSTR ValueName<T>::StringFormat = _T("%d");

template<typename T> LPCTSTR ValueName<T>::UnknownValueName = _T("Unknown");

template<typename T, size_t size>
const ValueName<T>& FindValueName(const ValueName<T>(&list)[size], const T& v)
{
	for(auto& i : list) {
		if(i.value == v) {
			return i;
		}
	}
	static ValueName<T> unknown = {T(), ValueName<T>::UnknownValueName, nullptr};
	unknown.value = v;
	return unknown;
}

// Searches a value in the list and returns string generated by toString() method of found ValueName object.
template<typename T, size_t size>
CString ValueToString(const ValueName<T>(&list)[size], const T& v)
{
	return FindValueName(list, v).toString();
}

template<typename T, size_t size>
CString FlagValueToString(const ValueName<T>(&list)[size], const T& v)
{
	// Flag should be shown as hex value.
	LPCTSTR stringFormat = _T("0x%x");
	CString str;
	static const CString plus = _T("+");
	for(auto& i : list) {
		if(i.value == v) {
			return i.toString(stringFormat);
		}
		if(i.value & v) {
			if(0 < str.GetLength()) { str += plus; }
			str += i.toString(stringFormat);
		}
	}

	CString ret;
	ret.Format(_T("%s=0x%s"),
		(0 < str.GetLength()) ? str.GetString() : ValueName<T>::UnknownValueName,
		ValueName<T>::valueToString(v, stringFormat).GetString());
	return ret;
}

// Returns string `name(value):description`.
template<typename T>
CString ValueName<T>::toString(LPCTSTR stringFormat /*= nullptr*/) const
{
	CString desc;
	if(description) { desc.Format(_T(":%s"), description); }
	CString ret;
	ret.Format(_T("%s(%s)%s"), (name ? name : _T("No name")), valueToString(stringFormat).GetString(), desc.GetString());
	return ret;
}

// Returns a string that represents the value.
// ValueName<>::StringFormat is used to format a string.
template<typename T>
CString ValueName<T>::valueToString(LPCTSTR stringFormat /*= nullptr*/) const
{
	return valueToString(value, stringFormat);
}

template<typename T>
CString ValueName<T>::valueToString(const T& v, LPCTSTR stringFormat /*= nullptr*/)
{
	CString ret;
	ret.Format(stringFormat ? stringFormat : StringFormat, v);
	return ret;
}
