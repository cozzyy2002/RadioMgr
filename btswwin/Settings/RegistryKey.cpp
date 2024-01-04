#include "pch.h"
#include "RegistryKey.h"
#include "../Common/Assert.h"

static auto& logger(log4cxx::Logger::getLogger(_T("btswwin.CRegistryKey")));

CRegistryKey::CRegistryKey()
    : m_hParent(nullptr), m_parent(nullptr)
{
}

// Attaches itself as root key.
HRESULT CRegistryKey::attach(HKEY hParent, LPCTSTR keyName)
{
    // Make sure that another attach() or open() method has not been called.
    HR_ASSERT(!m_hParent && !m_parent && !m_regKey.m_hKey, E_ILLEGAL_METHOD_CALL);

    m_hParent = hParent;
    m_keyName = keyName;

    LOG4CXX_DEBUG(logger, _T("Attached: ") << getFullKeyName().GetString());
    return S_OK;
}

// Attaches itself as a sub key of the parent key.
HRESULT CRegistryKey::attach(CRegistryKey& parent, LPCTSTR keyName)
{
    // Make sure that another attach() or open() method has not been called.
    HR_ASSERT(!m_hParent && !m_parent && !m_regKey.m_hKey, E_ILLEGAL_METHOD_CALL);

    m_parent = &parent;
    m_keyName = keyName;
    parent.m_subKeys.push_back(this);

    LOG4CXX_DEBUG(logger, _T("Attached: ") << getFullKeyName().GetString());
    return S_OK;
}

HRESULT CRegistryKey::open()
{
    LOG4CXX_DEBUG(logger, _T("Opening: ") << getFullKeyName().GetString());

    // Check if attached.
    HR_ASSERT(!m_keyName.IsEmpty(), E_ILLEGAL_METHOD_CALL);

    auto hParent = m_parent ? m_parent->getHKey() : m_hParent;
    HR_ASSERT_OK(HRESULT_FROM_WIN32(
        m_regKey.Create(hParent, m_keyName.GetString())
    ));

    for(auto subKey : m_subKeys) {
        subKey->open();
    }
    return S_OK;
}

const CString& CRegistryKey::getFullKeyName() const
{
    if(m_fullKeyName.IsEmpty()) {
        if(!m_parent) {
            m_fullKeyName = m_keyName;
        } else {
            m_fullKeyName.Format(_T("%s\\%s"),
                m_parent->getFullKeyName().GetString(), m_keyName.GetString()
            );
        }
    }
    return m_fullKeyName;
}
