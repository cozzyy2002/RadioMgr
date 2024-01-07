#include "pch.h"
#include "Settings.h"
#include "../Common/Assert.h"
static auto& logger(log4cxx::Logger::getLogger(_T("btswwin.CSettings")));

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

HRESULT CSettings::read(IValue& value, std::unique_ptr<BYTE[]>& data, DWORD expectedSize /*= 0*/)
{
	auto expectedType = value.getRegType();
	auto size = expectedSize;
	DWORD type;
	auto& regKey = value.getHKey();
	if(0 == expectedSize) {
		// Retrieve size of the value in registory.
		auto errorRegGetValue = regKey.QueryValue(value.getName(), &type, NULL, &size);
		// If the value does not exist, return error without logging.
		if(errorRegGetValue == ERROR_FILE_NOT_FOUND) { return HRESULT_FROM_WIN32(errorRegGetValue); }
		// Check error.
		HR_ASSERT_OK(HRESULT_FROM_WIN32(errorRegGetValue));
		if(type != expectedType) {
			LOG4CXX_WARN(logger, value.getName() << _T(": Unexpected type ") << type << _T("(size = ") << size << _T(")"));
			return E_UNEXPECTED;
		}
	}

	data = std::make_unique<BYTE[]>(size);
	auto errorRegGetValue = regKey.QueryValue(value.getName(), &type, data.get(), &size);
	// If the value does not exist, return error without logging.
	if(errorRegGetValue == ERROR_FILE_NOT_FOUND) { return HRESULT_FROM_WIN32(errorRegGetValue); }
	// Check another error.
	HR_ASSERT_OK(HRESULT_FROM_WIN32(errorRegGetValue));
	if((type == expectedType) && ((expectedSize ? (expectedSize == size) : true))) {
		return S_OK;
	} else {
		LOG4CXX_WARN(logger, value.getName() << _T(": Unexpected type ") << type << _T(" or size ") << size);
		data.reset();
		return E_UNEXPECTED;
	}
}

HRESULT CSettings::write(IValue& value, const BYTE* data, DWORD size)
{
	auto& regKey = value.getHKey();
	return HR_EXPECT_OK(HRESULT_FROM_WIN32(regKey.SetValue(value.getName(), value.getRegType(), data, size)));
}


template<>
bool CSettings::read(Value<bool>& value)
{
	std::unique_ptr<BYTE[]> data;
	if(SUCCEEDED(read(value, data, sizeof(int)))) {
		return *(int*)data.get() ? true : false;
	} else {
		return value.getDefault();
	}
}

template<>
void CSettings::write(Value<bool>& value)
{
	int data = (value ? TRUE : FALSE);
	write(value, (const BYTE*)&data, sizeof(data));
}

template<>
int CSettings::read(Value<int>& value)
{
	std::unique_ptr<BYTE[]> data;
	if(SUCCEEDED(read(value, data, sizeof(int)))) {
		return *(int*)data.get();
	} else {
		return value.getDefault();
	}
}

template<>
void CSettings::write(Value<int>& value)
{
	int data = value;
	write(value, (const BYTE*)&data, sizeof(data));
}

template<>
CString CSettings::read(Value<CString>& value)
{
	std::unique_ptr<BYTE[]> data;
	if(SUCCEEDED(read(value, data))) {
		return (LPCTSTR)data.get();
	} else {
		return value.getDefault();
	}
}

template<>
void CSettings::write(Value<CString>& value)
{
	write(value, (const BYTE*)value->GetString(), (value->GetLength() + 1) * sizeof(TCHAR));
}


template<>
const DWORD CSettings::Value<bool>::RegType = REG_DWORD;
template<>
const DWORD CSettings::Value<int>::RegType = REG_DWORD;
template<>
const DWORD CSettings::Value<CString>::RegType = REG_SZ;

CString CSettings::ValueBase::toString(bool isRelativeKey /*= false*/) const
{
	CString str;
	str.Format(_T("%s\\%s: %s"),
		m_registryKey.getFullKeyName(isRelativeKey).GetString(),
		m_name.GetString(),
		valueToString().GetString()
	);
	return str;
}

CString CSettings::Value<bool>::valueToString() const
{
	return (m_value ? _T("true") : _T("false"));
}

template<>
CString CSettings::Value<int>::valueToString() const
{
	CString str;
	str.Format(_T("%d"), m_value);
	return str;
}

template<>
CString CSettings::Value<CString>::valueToString() const
{
	return m_value.GetString();
}
