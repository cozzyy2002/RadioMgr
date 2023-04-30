#include <Windows.h>
#include <SetupAPI.h>
#include <initguid.h>

#include "../Common/SafeHandle.h"
#include "../Common/Assert.h"

#pragma comment(lib, "SetupApi.lib")

DEFINE_GUID(BluetoothDeviceInterfaceClassGUID, 0x00f40965, 0xe89d, 0x4487, 0x98, 0x90, 0x87, 0xc3, 0xab, 0xb2, 0x11, 0xf4);

static void DeleteDevInfo(HDEVINFO h) {
	if(h != INVALID_HANDLE_VALUE) {
		//wprintf_s(L"Destroying HDEVINFO 0x%p", h);
		WIN32_EXPECT(SetupDiDestroyDeviceInfoList(h));
	}
}
using HDevInfo = SafeHandle<HDEVINFO, DeleteDevInfo>;

HRESULT BluetoothGATT(int argc, wchar_t** argv)
{
	DWORD flag = DIGCF_DEVICEINTERFACE /*| DIGCF_ALLCLASSES*/;
	HDevInfo hDevInfo = SetupDiGetClassDevs(&BluetoothDeviceInterfaceClassGUID, NULL, NULL, flag);
	WIN32_ASSERT(hDevInfo != INVALID_HANDLE_VALUE);

	SP_DEVINFO_DATA data{sizeof(data)};
	for(DWORD index = 0; SetupDiEnumDeviceInfo(hDevInfo, index, &data); index++) {
		OLECHAR strGuid[40];
		HR_ASSERT(0 < StringFromGUID2(data.ClassGuid, strGuid, ARRAYSIZE(strGuid)), E_UNEXPECTED);
		DWORD keyCount;
		WIN32_ASSERT(
			SetupDiGetDevicePropertyKeys(hDevInfo, &data, NULL, 0, &keyCount, 0)
			!= ERROR_INSUFFICIENT_BUFFER
		);
		wprintf(L"%d %s: %d keys\n", index, strGuid, keyCount);
		auto keys = std::make_unique<DEVPROPKEY[]>(keyCount);
		WIN32_ASSERT(SetupDiGetDevicePropertyKeys(hDevInfo, &data, keys.get(), keyCount, NULL, 0));
		for(DWORD i = 0; i < keyCount; i++) {
			auto& key = keys[i];
			DEVPROPTYPE propType;
			DWORD propSize;
			WIN32_ASSERT(
				SetupDiGetDeviceProperty(hDevInfo, &data, &key, &propType, NULL, 0, &propSize, 0)
				!= ERROR_INSUFFICIENT_BUFFER
			);
			auto prop = std::make_unique<BYTE[]>(propSize);
			WIN32_ASSERT(SetupDiGetDeviceProperty(hDevInfo, &data, &key, &propType, prop.get(), propSize, NULL, 0));
			HR_ASSERT(0 < StringFromGUID2(key.fmtid, strGuid, ARRAYSIZE(strGuid)), E_UNEXPECTED);
			wprintf(L"  % 3d %s Type=0x%08x, Size=%d\n", i, strGuid, propType, propSize);
		}
	}

	return S_OK;
}