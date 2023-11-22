#include "pch.h"
#include "Settings.h"
#include "../Common/Assert.h"

static auto& logger(log4cxx::Logger::getLogger(_T("btswwin.CSettings")));

CSettings::CSettings(LPCTSTR companyName, LPCTSTR applicationName)
{
	static const auto subKeyFormat = _T("Software\\%s\\%s")
#ifdef _DEBUG
		_T(".debug");
#endif
	;

	CString subKey;
	subKey.Format(subKeyFormat, companyName, applicationName);

	HKEY hKey;
	if(SUCCEEDED(HR_EXPECT_OK(HRESULT_FROM_WIN32(
		RegCreateKeyEx(HKEY_CURRENT_USER, subKey.GetString(), 0, NULL, 0, KEY_ALL_ACCESS, NULL, &hKey, NULL))
	))) {
		m_hKey.reset(hKey);
	}
}

void CSettings::HKEYDeleter::operator()(HKEY h)
{
	HR_EXPECT_OK(HRESULT_FROM_WIN32(
		RegCloseKey(h)
	));
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
		auto errorRegGetValue = RegGetValue(m_hKey.get(), NULL, valueName, RRF_RT_ANY, &type, NULL, &size);
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
	auto errorRegGetValue = RegGetValue(m_hKey.get(), NULL, valueName, RRF_RT_ANY, &type, data.get(), &size);
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
	return HR_EXPECT_OK(HRESULT_FROM_WIN32(RegSetValueEx(m_hKey.get(), valueName, 0, type, data, size)));
}


template<>
bool CSettings::read(Value<bool>& value)
{
	std::unique_ptr<BYTE[]> data;
	if(SUCCEEDED(read(value.getName(), value.RegType, data, sizeof(int)))) {
		return *(int*)data.get() ? true : false;
	} else {
		return value.getDefault();
	}
}

template<>
void CSettings::write(Value<bool>& value)
{
	int data = (value ? TRUE : FALSE);
	write(value.getName(), value.RegType, (const BYTE*)&data, sizeof(data));
}

template<>
int CSettings::read(Value<int>& value)
{
	std::unique_ptr<BYTE[]> data;
	if(SUCCEEDED(read(value.getName(), value.RegType, data, sizeof(int)))) {
		return *(int*)data.get();
	} else {
		return value.getDefault();
	}
}

template<>
void CSettings::write(Value<int>& value)
{
	int data = value;
	write(value.getName(), value.RegType, (const BYTE*)&data, sizeof(data));
}

template<>
CString CSettings::read(Value<CString>& value)
{
	std::unique_ptr<BYTE[]> data;
	if(SUCCEEDED(read(value.getName(), value.RegType, data))) {
		return (LPCTSTR)data.get();
	} else {
		return value.getDefault();
	}
}

template<>
void CSettings::write(Value<CString>& value)
{
	write(value.getName(), value.RegType, (const BYTE*)value->GetString(), (value->GetLength() + 1) * sizeof(TCHAR));
}


template<>
const DWORD CSettings::Value<bool>::RegType = REG_DWORD;
template<>
const DWORD CSettings::Value<int>::RegType = REG_DWORD;
template<>
const DWORD CSettings::Value<CString>::RegType = REG_SZ;

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
