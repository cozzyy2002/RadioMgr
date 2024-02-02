#include "pch.h"
#include "ValueName.h"

CString guidToString(REFGUID guid)
{
	OLECHAR strGuid[50] = L"";
	HR_EXPECT(0 < StringFromGUID2(guid, strGuid, ARRAYSIZE(strGuid)), HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER));
	return CString(strGuid);
}

// Returns a string that represents the GUID value.
template<>
CString ValueName<GUID>::valueToString(const GUID& v, LPCTSTR)
{
	return guidToString(v);
}
