#include "IpAdapterAddresses.h"

#include <WinSock2.h>
#include <iphlpapi.h>
#include <memory>
#include "../Common/Assert.h"
#include "ValueName.h"

#pragma comment(lib, "Iphlpapi.lib")

IpAdapterAddresses::IpAdapterAddresses()
{
}

HRESULT IpAdapterAddresses::update()
{
	ULONG size = 0;
	ULONG family = AF_UNSPEC;
	ULONG flags = GAA_FLAG_INCLUDE_PREFIX;
	auto sizeCheckError = GetAdaptersAddresses(family, flags, NULL, NULL, &size);
	HR_ASSERT(sizeCheckError == ERROR_BUFFER_OVERFLOW, HRESULT_FROM_WIN32(sizeCheckError));

	auto buffer = std::make_unique<BYTE[]>(size);
	auto adapterAddresses = (IP_ADAPTER_ADDRESSES*)buffer.get();
	auto getAdapterAddressesError = GetAdaptersAddresses(family, flags, NULL, adapterAddresses, &size);
	HR_ASSERT_OK(HRESULT_FROM_WIN32(getAdapterAddressesError));

	static const ValueName<IF_OPER_STATUS> OperStatuses[] = {
		VALUE_NAME_ITEM(IfOperStatusUp),
		VALUE_NAME_ITEM(IfOperStatusDown),
		VALUE_NAME_ITEM(IfOperStatusTesting),
		VALUE_NAME_ITEM(IfOperStatusUnknown),
		VALUE_NAME_ITEM(IfOperStatusDormant),
		VALUE_NAME_ITEM(IfOperStatusNotPresent),
		VALUE_NAME_ITEM(IfOperStatusLowerLayerDown)
	};

	for(auto p = adapterAddresses; p; p = p->Next) {
		CT2W operStatus(ValueToString(OperStatuses, p->OperStatus).GetString());
		wprintf_s(L"%S:%s:%s\n  %s: %s\n",
			p->AdapterName, p->DnsSuffix, (LPCWSTR)operStatus,
			p->FriendlyName, p->Description);
	}

	return S_OK;
}
