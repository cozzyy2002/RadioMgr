#include <Windows.h>
#include <SetupAPI.h>
#include <initguid.h>
#include <bluetoothleapis.h>

#include "BluetoothCommon.h"
#include "../Common/SafeHandle.h"
#include "../Common/Assert.h"

#pragma comment(lib, "SetupApi.lib")
#pragma comment(lib, "BluetoothAPIs.lib")

static HRESULT ShowGATT(HANDLE hDevice);

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

#if 1
	SP_DEVINFO_LIST_DETAIL_DATA detail{sizeof(detail)};
	WIN32_ASSERT(SetupDiGetDeviceInfoListDetail(hDevInfo, &detail));
	wprintf_s(L"RemoteMachineName: `%s`\n", detail.RemoteMachineName);
#endif

#if 0
	SP_DEVICE_INTERFACE_DATA interfaceData{sizeof(interfaceData)};
	for(DWORD index = 0; SetupDiEnumDeviceInterfaces(hDevInfo, NULL, &BluetoothDeviceInterfaceClassGUID, index, &interfaceData); index++) {
		DWORD detailSize = 0;
		SetupDiGetDeviceInterfaceDetail(hDevInfo, &interfaceData, NULL, 0, &detailSize, NULL);
		auto error = GetLastError();
		WIN32_ASSERT(error == ERROR_INSUFFICIENT_BUFFER);
		auto detail = std::make_unique<BYTE[]>(detailSize);
		auto pDetail = (PSP_DEVICE_INTERFACE_DETAIL_DATA)detail.get();
		pDetail->cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA); // detailSize;
		SP_DEVINFO_DATA infoData{sizeof(infoData)};
		WIN32_ASSERT(SetupDiGetDeviceInterfaceDetail(hDevInfo, &interfaceData, pDetail, detailSize, NULL, &infoData));
		wprintf_s(L"  %d %d byte: `%s`\n", index, detailSize, pDetail->DevicePath);

		HFile hFile = CreateFile(pDetail->DevicePath, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
		WIN32_ASSERT(hFile != INVALID_HANDLE_VALUE);
		HR_EXPECT_OK(ShowGATT(hFile));
	}
#endif

#if 1
	SP_DEVINFO_DATA data{sizeof(data)};
	for(DWORD index = 0; SetupDiEnumDeviceInfo(hDevInfo, index, &data); index++) {
		DWORD keyCount;
		SetupDiGetDevicePropertyKeys(hDevInfo, &data, NULL, 0, &keyCount, 0);
		auto errSetupDiGetDevicePropertyKeys = GetLastError();
		HR_ASSERT(errSetupDiGetDevicePropertyKeys == ERROR_INSUFFICIENT_BUFFER, HRESULT_FROM_WIN32(errSetupDiGetDevicePropertyKeys));
		wprintf(L"%d %s: %d keys\n", index, GuidToString(data.ClassGuid).c_str(), keyCount);
		auto keys = std::make_unique<DEVPROPKEY[]>(keyCount);
		WIN32_ASSERT(SetupDiGetDevicePropertyKeys(hDevInfo, &data, keys.get(), keyCount, NULL, 0));
		for(DWORD i = 0; i < keyCount; i++) {
			auto& key = keys[i];
			DEVPROPTYPE propType;
			DWORD propSize;
			SetupDiGetDeviceProperty(hDevInfo, &data, &key, &propType, NULL, 0, &propSize, 0);
			auto errSetupDiGetDeviceProperty = GetLastError();
			HR_ASSERT(errSetupDiGetDeviceProperty == ERROR_INSUFFICIENT_BUFFER, HRESULT_FROM_WIN32(errSetupDiGetDeviceProperty));
			auto prop = std::make_unique<BYTE[]>(propSize);
			WIN32_ASSERT(SetupDiGetDeviceProperty(hDevInfo, &data, &key, &propType, prop.get(), propSize, NULL, 0));
			wprintf(L"  % 3d %s ", i, GuidToString(key.fmtid).c_str());

			switch(propType) {
			case DEVPROP_TYPE_BOOLEAN:
				wprintf(L"BOOLEAN %s", *(BYTE*)prop.get() ? L"TRUE" : L"FALSE");
				break;
			case DEVPROP_TYPE_BYTE:
				wprintf(L"BYTE %u", *(BYTE*)prop.get());
				break;
			case DEVPROP_TYPE_INT16:
				wprintf(L"INT16 %d", *(INT16*)prop.get());
				break;
			case DEVPROP_TYPE_UINT16:
				wprintf(L"UINT16 %u", *(UINT16*)prop.get());
				break;
			case DEVPROP_TYPE_INT32:
				wprintf(L"UINT32 %d", *(INT32*)prop.get());
				break;
			case DEVPROP_TYPE_UINT32:
				wprintf(L"UINT32 %u", *(UINT32*)prop.get());
				break;
			case DEVPROP_TYPE_INT64:
				wprintf(L"UINT64 %I64d", *(INT64*)prop.get());
				break;
			case DEVPROP_TYPE_UINT64:
				wprintf(L"UINT64 %I64u", *(UINT64*)prop.get());
				break;
			case DEVPROP_TYPE_FILETIME:
				{
					FILETIME ft;
					FileTimeToLocalFileTime((FILETIME*)prop.get(), &ft);
					SYSTEMTIME t;
					FileTimeToSystemTime(&ft, &t);
					wprintf(L"%04d/%02d/%02d %02d:%02d:%02d.%03d", t.wYear, t.wMonth, t.wDay, t.wHour, t.wMinute, t.wSecond, t.wMilliseconds);
				}
				break;
			case DEVPROP_TYPE_GUID:
				wprintf(L"%s", GuidToString(*(GUID*)prop.get()).c_str());
				break;
			case DEVPROP_TYPE_STRING:
				wprintf(L"`%s`", (WCHAR*)prop.get());
				break;
			case DEVPROP_TYPE_STRING_LIST:
				// Note: Number of characters includes number of NULL terminaters(end of each string and end of the list).
				wprintf(L"STRING_LIST %d characters", (int)(propSize / sizeof(WCHAR)));
				for(auto p = (WCHAR*)prop.get(); *p; p += (wcslen(p) + 1)) {
					wprintf(L"\n        `%s`", p);
				}
				break;
			case DEVPROP_TYPE_BINARY:
				wprintf(L"BINARY %d bytes", propSize);
				for(DWORD i = 0; i < propSize; i++) {
					if((i % 16) == 0) { wprintf(L"\n       "); }
					wprintf(L" %02x", ((BYTE*)prop.get())[i]);
				}
				break;
			default:
				wprintf(L"Type=0x%08x, Size=%d", propType, propSize);
				break;
			}
			_putws(L"");
		}
	}
#endif

	return S_OK;
}

HRESULT ShowGATT(HANDLE hDevice)
{
	USHORT serviceCount = 0;
	auto error = BluetoothGATTGetServices(hDevice, 0, NULL, &serviceCount, BLUETOOTH_GATT_FLAG_NONE);
	HR_ASSERT(error == ERROR_MORE_DATA, HRESULT_FROM_WIN32(error));
	wprintf_s(L"    %d Services\n", serviceCount);
	auto services = std::make_unique<BTH_LE_GATT_SERVICE[]>(serviceCount);
	HR_ASSERT_OK(HRESULT_FROM_WIN32(BluetoothGATTGetServices(hDevice, serviceCount, services.get(), NULL, BLUETOOTH_GATT_FLAG_NONE)));

	for(USHORT iService = 0; iService < serviceCount; iService++) {

	}
	return S_OK;
}
