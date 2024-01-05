#pragma once

#include <vector>

class CRegistryKey
{
public:
	explicit CRegistryKey();

	HRESULT attach(HKEY hParent, LPCTSTR keyName);
	HRESULT attach(CRegistryKey& parent, LPCTSTR keyName);

	HRESULT open();
	CString getFullKeyName(bool isRelative = false) const;

	auto& getHKey() { return m_regKey; }
	const auto& getKeyName() const { return m_keyName; }

protected:
	CRegKey m_regKey;
	CString m_keyName;

	HKEY m_hParent;			// Available only root key.
	CRegistryKey* m_parent;	// Available for sub keys.
	std::vector<CRegistryKey*> m_subKeys;
};
