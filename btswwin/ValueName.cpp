#include "pch.h"
#include "ValueName.h"

CString guidToString(REFGUID guid)
{
	OLECHAR strGuid[50] = _T("");
	HR_EXPECT(0 < StringFromGUID2(guid, strGuid, ARRAYSIZE(strGuid)), HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER));
	return strGuid;
}

// Returns a string that represents the GUID value.
template<>
CString ValueName<GUID>::valueToString() const
{
	return guidToString(value);
}
