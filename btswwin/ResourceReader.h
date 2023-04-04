#pragma once

#include "../Common/Assert.h"

class CResourceReader
{
public:
	CResourceReader();

	CString getFileVersion() const;
	CString getProductVersion() const;
	CString getCompanyName() const { return queryString(_T("CompanyName")); }
	CString getFileDesctiption() const { return queryString(_T("FileDescription")); }
	CString getProductName() const { return queryString(_T("ProductName")); }
	CString getLegalCopyright() const { return queryString(_T("LegalCopyright")); }

	template<typename T>
	CString getCustomString(DWORD id) const;

protected:
	std::unique_ptr<BYTE[]> m_versionInfo;
	VS_FIXEDFILEINFO* m_pFileInfo;
	CString m_subBlockKey;

	CString queryString(LPCTSTR key) const;
};

// Returns string retrieved from custom string resource.
// Type parameter T should:
//    Conform to encoding of the string resource.
//    Be type that can be assigned to CString object.
template<typename T>
CString CResourceReader::getCustomString(DWORD id) const
{
	CString ret;

	auto hResource = FindResource(NULL, MAKEINTRESOURCE(id), _T("RT_CUSTOM_STRING"));
	if(SUCCEEDED(WIN32_EXPECT(hResource))) {
		auto hGlobal = LoadResource(NULL, hResource);
		if(SUCCEEDED(WIN32_EXPECT(hGlobal))) {
			auto p = (T)LockResource(hGlobal);
			if(p) { ret = p; }
		}
	}
	return ret;
}
