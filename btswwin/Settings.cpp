#include "pch.h"
#include "Settings.h"
#include "../Common/Assert.h"

static auto& logger(log4cxx::Logger::getLogger(_T("btswwin.CSettings")));

CSettings::CSettings(LPCTSTR companyName, LPCTSTR applicationName)
{
	CString subKey;
	subKey.Format(_T("Software\\%s\\%s"), companyName, applicationName);
#ifdef _DEBUG
	subKey += _T(".debug");
#endif

	HR_EXPECT_OK(HRESULT_FROM_WIN32(
		RegCreateKeyEx(HKEY_CURRENT_USER, subKey.GetString(), 0, NULL, 0, KEY_ALL_ACCESS, NULL, &m_hKey, NULL))
	);
}

HRESULT CSettings::load(IValue** valueList, size_t size)
{
	HR_ASSERT(valueList, E_POINTER);
	HR_ASSERT(size, E_INVALIDARG);

	m_valueList.assign(valueList, &valueList[size]);

	for(auto value : m_valueList) {
		value->read(this);
	}
	return S_OK;
}

HRESULT CSettings::save(bool force /*= false*/)
{
	HR_ASSERT(!m_valueList.empty(), E_ILLEGAL_METHOD_CALL);

	for(auto value : m_valueList) {
		if(force || value->isChanged()) {
			LOG4CXX_DEBUG(logger, _T("Writing ") << value->toString().GetString());
			value->write(this);
		}
	}
	return S_OK;
}

bool CSettings::isChanged() const
{
	for(auto value : m_valueList) {
		if(value->isChanged()) { return true; }
	}
	return false;
}

HRESULT CSettings::read(LPCTSTR valueName, DWORD expectedType, std::unique_ptr<BYTE[]>& data, DWORD expectedSize /*= 0*/)
{
	auto size = expectedSize;
	DWORD type;
	if(0 == expectedSize) {
		// Retrieve size of the value in registory.
		auto errorRegGetValue = RegGetValue(m_hKey, NULL, valueName, RRF_RT_ANY, &type, NULL, &size);
		// If the value does not exist, return error.
		if(errorRegGetValue == ERROR_FILE_NOT_FOUND) { return HRESULT_FROM_WIN32(errorRegGetValue); }
		// Check error.
		HR_ASSERT_OK(HRESULT_FROM_WIN32(errorRegGetValue));
		if(type != expectedType) {
			LOG4CXX_WARN(logger, valueName << _T(": Unexpected type ") << type << _T("(size = ") << size << _T(")"));
			return E_UNEXPECTED;
		}
	}

	data = std::make_unique<BYTE[]>(size);
	auto errorRegGetValue = RegGetValue(m_hKey, NULL, valueName, RRF_RT_ANY, &type, data.get(), &size);
	// If the value does not exist, return error.
	if(errorRegGetValue == ERROR_FILE_NOT_FOUND) { return HRESULT_FROM_WIN32(errorRegGetValue); }
	// Check another error.
	HR_ASSERT_OK(HRESULT_FROM_WIN32(errorRegGetValue));
	if((type == expectedType) && ((expectedSize ? (expectedSize == size) : true))) {
		return S_OK;
	} else {
		LOG4CXX_WARN(logger, valueName << _T(": Unexpected type ") << type << _T(" or size ") << size);
		data.reset();
		return E_UNEXPECTED;
	}
}

HRESULT CSettings::write(LPCTSTR valueName, DWORD type, const BYTE* data, DWORD size)
{
	return HR_EXPECT_OK(HRESULT_FROM_WIN32(RegSetValueEx(m_hKey, valueName, 0, type, data, size)));
}


template<>
bool CSettings::read(Value<bool>& value)
{
	std::unique_ptr<BYTE[]> data;
	if(SUCCEEDED(read(value.getName(), value.getRegistryType(), data, sizeof(int)))) {
		return *(int*)data.get() ? true : false;
	} else {
		return value.getDefault();
	}
}

template<>
void CSettings::write(Value<bool>& value)
{
	int data = (value ? TRUE : FALSE);
	write(value.getName(), value.getRegistryType(), (const BYTE*)&data, sizeof(data));
}

template<>
int CSettings::read(Value<int>& value)
{
	std::unique_ptr<BYTE[]> data;
	if(SUCCEEDED(read(value.getName(), value.getRegistryType(), data, sizeof(int)))) {
		return *(int*)data.get();
	} else {
		return value.getDefault();
	}
}

template<>
void CSettings::write(Value<int>& value)
{
	int data = value;
	write(value.getName(), value.getRegistryType(), (const BYTE*)&data, sizeof(data));
}

template<>
CString CSettings::read(Value<CString>& value)
{
	std::unique_ptr<BYTE[]> data;
	if(SUCCEEDED(read(value.getName(), value.getRegistryType(), data))) {
		return (LPCTSTR)data.get();
	} else {
		return value.getDefault();
	}
}

template<>
void CSettings::write(Value<CString>& value)
{
	write(value.getName(), value.getRegistryType(), (const BYTE*)value->GetString(), (value->GetLength() + 1) * sizeof(TCHAR));
}


template<>
DWORD CSettings::Value<bool>::getRegistryType() const { return REG_DWORD; }
template<>
DWORD CSettings::Value<int>::getRegistryType() const { return REG_DWORD; }
template<>
DWORD CSettings::Value<CString>::getRegistryType() const { return REG_SZ; }
template<>
DWORD CSettings::BinaryValue<WINDOWPLACEMENT>::getRegistryType() const { return REG_BINARY; }

DWORD CSettings::BinaryValue<CStringArray>::getRegistryType() const { return REG_MULTI_SZ; }

CString CSettings::Value<bool>::toString() const
{
	CString str;
	str.Format(_T("%s: %s"), m_name.GetString(), m_value ? _T("true") : _T("false"));
	return str;
}

template<>
CString CSettings::Value<int>::toString() const
{
	CString str;
	str.Format(_T("%s: %d"), m_name.GetString(), m_value);
	return str;
}

template<>
CString CSettings::Value<CString>::toString() const
{
	CString str;
	str.Format(_T("%s: %s"), m_name.GetString(), m_value.GetString());
	return str;
}

void CSettings::CStringArrayValue::ValueHandler::copy(CStringArray& dest, const CStringArray& source)
{
	dest.RemoveAll();
	dest.Copy(source);
}

bool CSettings::CStringArrayValue::ValueHandler::isChanged(const CStringArray& a, const CStringArray& b)
{
	if(a.GetCount() != b.GetCount()) return true;

	for(auto i = 0; i < a.GetCount(); i++) {
		if(a[i] != b[i]) return true;
	}
	return false;
}

CString CSettings::CStringArrayValue::ValueHandler::valueToString(const BinaryValue<CStringArray>& value) const
{
	CString str;
	str.Format(_T("Count=%d"), (int)((const CStringArray&)value).GetCount());
	return str;
}

// Reads REG_MULTI_SZ type value and add the string(s) to CStringArray object.
// See about REG_MULTI_SZ data format: https://learn.microsoft.com/en-us/windows/win32/sysinfo/registry-value-types
void CSettings::CStringArrayValue::read(CSettings* settings)
{
	std::unique_ptr<BYTE[]> data;
	if(SUCCEEDED(settings->read(m_name.GetString(), getRegistryType(), data))) {
		auto p = (LPCTSTR)data.get();
		while(*p) {
			auto index = m_value.Add(p);
			p += (m_value[index].GetLength() + 1);
		}
		m_valueHandler->copy(m_savedValue, m_value);
	}
}

// Copy string(s) contained in CStringArray object to the buffer and write it to REG_MULTI_SZ type registry value.
void CSettings::CStringArrayValue::write(CSettings* settings)
{
	DWORD size = 0;
	for(int i = 0; i < m_value.GetCount(); i++) {
		size += (m_value[i].GetLength() + 1);
	}
	size++;

	auto buff = std::make_unique<TCHAR[]>(size);
	auto p = buff.get();
	auto remainSize = size;
	for(int i = 0; i < m_value.GetCount(); i++) {
		auto& str = m_value[i];
		auto len = str.GetLength() + 1;
		_tcscpy_s(p, remainSize, str.GetString());
		p += len;
		remainSize -= len;
	}
	*p = _T('\0');

	settings->write(m_name, getRegistryType(), (const BYTE*)buff.get(), size * sizeof(TCHAR));
	m_valueHandler->copy(m_savedValue, m_value);
}
