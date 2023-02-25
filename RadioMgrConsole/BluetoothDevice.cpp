#include "BluetoothCommon.h"

HRESULT EnumBluetoothDevices()
{
	BLUETOOTH_DEVICE_SEARCH_PARAMS params = {sizeof(params), TRUE, TRUE, TRUE, TRUE, TRUE, 1};
	BLUETOOTH_DEVICE_INFO info = {sizeof(info)};

	auto hFind = BluetoothFindFirstDevice(&params, &info);
	WIN32_ASSERT(hFind);
	_putws(L"Address           Last seen           Last used           Name");
	do {
		SYSTEMTIME lastSeen, lastUsed;
		WIN32_ASSERT(SystemTimeToTzSpecificLocalTime(NULL, &info.stLastSeen, &lastSeen));
		WIN32_ASSERT(SystemTimeToTzSpecificLocalTime(NULL, &info.stLastUsed, &lastUsed));
		wprintf_s(
			L"%02x:%02x:%02x:%02x:%02x:%02x %04d/%02d/%02d %02d:%02d:%02d %04d/%02d/%02d %02d:%02d:%02d %-20s %s, %s, %s\n"
			L"  Class of Device = %s\n",
			A2ARG(info, Address), T2ARG(lastSeen), T2ARG(lastUsed), info.szName,
			B2ARG(info, Remembered), B2ARG(info, Authenticated), B2ARG(info, Connected),
			ClassOfDevice(info.ulClassofDevice).c_str());

		WIN32_ASSERT(BluetoothDisplayDeviceProperties(NULL, &info));
	} while(BluetoothFindNextDevice(hFind, &info));
	auto hr = WIN32_EXPECT(GetLastError() == ERROR_NO_MORE_ITEMS);
	WIN32_ASSERT(BluetoothFindDeviceClose(hFind));

	return hr;
}
