#include "pch.h"
#include "ResourceReader.h"

#include "../Common/Assert.h"

#pragma comment(lib, "Version.lib")

static auto& logger(log4cxx::Logger::getLogger(_T("btswwin.CResourceReader")));

CResourceReader::CResourceReader()
	: m_pFileInfo(nullptr)
{
	CString fileName;
	AfxGetModuleFileName(NULL, fileName);
	DWORD dwHandle = 0;
	auto versionInfoSize = GetFileVersionInfoSize(fileName.GetString(), &dwHandle);
	if(FAILED(WIN32_EXPECT(0 < versionInfoSize))) { return; }
	m_versionInfo = std::make_unique<BYTE[]>(versionInfoSize);
	if(FAILED(WIN32_EXPECT(GetFileVersionInfo(fileName.GetString(), dwHandle, versionInfoSize, (LPVOID)m_versionInfo.get())))) { return; }
	UINT len = 0;
	BOOL ok;
	ok = VerQueryValue(m_versionInfo.get(), _T("\\"), (LPVOID*)&m_pFileInfo, &len);
	if(!ok || !m_pFileInfo) {
		LOG4CXX_ERROR(logger, _T("VerQueryValue(`\\`) failed"));
	}

	struct Translation {
		WORD language;
		WORD codePage;
	} *pTranslation = nullptr;

	ok = VerQueryValue(m_versionInfo.get(), _T("\\VarFileInfo\\Translation"), (LPVOID*)&pTranslation, &len);
	if(!ok || !pTranslation || (len < sizeof(Translation))) {
		LOG4CXX_ERROR(logger, _T("VerQueryValue(`\\VarFileInfo\\Translation`) failed. len = ") << len);
		return;
	}
	m_subBlockKey.Format(_T("\\StringFileInfo\\%04x%04x\\"), pTranslation->language, pTranslation->codePage);
	LOG4CXX_DEBUG(logger, _T("Sub block key: ") << m_subBlockKey.GetString());
}

// Returs file version in VS_FIXEDFILEINFO as string
CString CResourceReader::getFileVersion() const
{
	if(m_pFileInfo) {
		CString str;
		str.Format(_T("%d.%d.%d.%d"),
			HIWORD(m_pFileInfo->dwFileVersionMS),
			LOWORD(m_pFileInfo->dwFileVersionMS),
			HIWORD(m_pFileInfo->dwFileVersionLS),
			LOWORD(m_pFileInfo->dwFileVersionLS));
		return str;
	} else {
		return _T("?.?.?.?");
	}
}

// Returs product version in VS_FIXEDFILEINFO as string
CString CResourceReader::getProductVersion() const
{
	if(m_pFileInfo) {
		CString str;
		str.Format(_T("%d.%d.%d.%d"),
			HIWORD(m_pFileInfo->dwProductVersionMS),
			LOWORD(m_pFileInfo->dwProductVersionMS),
			HIWORD(m_pFileInfo->dwProductVersionLS),
			LOWORD(m_pFileInfo->dwProductVersionLS));
		return str;
	} else {
		return _T("?.?.?.?");
	}
}


CString CResourceReader::queryString(LPCTSTR key) const
{
	CString keyStr = m_subBlockKey + key;
	if(m_versionInfo) {
		LPCTSTR value = nullptr;
		UINT len = 0;
		auto ok = VerQueryValue(m_versionInfo.get(), keyStr.GetString(), (LPVOID*)&value, &len);
		if(ok && value) {
			LOG4CXX_DEBUG(logger, _T("Key `") << keyStr.GetString() << _T("` = `") << value << _T("`(") << len << _T(" char)"));
			return value;
		} else {
			LOG4CXX_ERROR(logger, _T("VerQueryValue(`") << keyStr.GetString() << _T("` failed. len = ") << len);
		}
	}

	return _T("?");
}
