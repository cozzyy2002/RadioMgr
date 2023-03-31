#include "pch.h"
#include "Settings.h"
#include "../Common/Assert.h"

CSettings::CSettings(LPCTSTR sectionName)
	: m_sectionName(sectionName)
{
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
			value->write(this);
		}
	}
	return S_OK;
}


template<>
bool CSettings::read(Value<bool>* value)
{
	return AfxGetApp()->GetProfileInt(m_sectionName, value->getName(), value->getDefault()) ?
		true : false;
}

template<>
void CSettings::write(Value<bool>* value)
{
	AfxGetApp()->WriteProfileInt(m_sectionName.GetString(), value->getName(), *value ? TRUE : FALSE);
}

template<>
int CSettings::read(Value<int>* value)
{
	return AfxGetApp()->GetProfileInt(m_sectionName.GetString(), value->getName(), value->getDefault());
}

template<>
void CSettings::write(Value<int>* value)
{
	AfxGetApp()->WriteProfileInt(m_sectionName.GetString(), value->getName(), *value);
}

template<>
CString CSettings::read(Value<CString>* value)
{
	return AfxGetApp()->GetProfileString(m_sectionName.GetString(), value->getName(), value->getDefault().GetString());
}

template<>
void CSettings::write(Value<CString>* value)
{
	AfxGetApp()->WriteProfileString(m_sectionName.GetString(), value->getName(), (*value)->GetString());
}
