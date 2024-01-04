#pragma once

#include <vector>

class CRegistryKey
{
public:
	explicit CRegistryKey();

	HRESULT attach(HKEY hParent, LPCTSTR keyName);
	HRESULT attach(CRegistryKey& parent, LPCTSTR keyName);

	HRESULT open();
	const CString& getFullKeyName() const;

	const auto& getHKey() const { return m_regKey; }
	const auto& getKeyName() const { return m_keyName; }

protected:
	CRegKey m_regKey;
	CString m_keyName;
	mutable CString m_fullKeyName;

	HKEY m_hParent;			// Available only root key.
	CRegistryKey* m_parent;	// Available for sub keys.
	std::vector<CRegistryKey*> m_subKeys;
};
