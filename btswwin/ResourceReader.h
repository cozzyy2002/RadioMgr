#pragma once

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

protected:
	std::unique_ptr<BYTE[]> m_versionInfo;
	VS_FIXEDFILEINFO* m_pFileInfo;
	CString m_subBlockKey;

	CString queryString(LPCTSTR key) const;
};
