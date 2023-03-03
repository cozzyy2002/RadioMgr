#include "BluetoothCommon.h"
#include <string>
#include <sstream>

void HandleDeleteFunc(HANDLE h) { WIN32_EXPECT(CloseHandle(h)); }

HRESULT EnumBluetoothRadios()
{
	BLUETOOTH_FIND_RADIO_PARAMS params{sizeof(params)};
	HANDLE hRadio;
	auto hFind = BluetoothFindFirstRadio(&params, &hRadio);
	WIN32_ASSERT(hFind);
	do {
		HRadio _h(hRadio);

		BLUETOOTH_RADIO_INFO info{sizeof(info)};
		HR_ASSERT_OK(HRESULT_FROM_WIN32(BluetoothGetRadioInfo(hRadio, &info)));
		BTH_MFG_3COM;
		wprintf_s(
			L"%02x:%02x:%02x:%02x:%02x:%02x %s Manufacturer 0x%04x, Subversion 0x%04x\n"
			L"  Class of Device = %s\n",
			A2ARG(info, address), info.szName, info.manufacturer, info.lmpSubversion,
			ClassOfDevice(info.ulClassofDevice).c_str());
	} while(BluetoothFindNextRadio(hFind, &hRadio));
	auto hr = WIN32_EXPECT(GetLastError() == ERROR_NO_MORE_ITEMS);
	WIN32_ASSERT(BluetoothFindRadioClose(hFind));

	return hr;
}

static std::wstring GuidToString(REFGUID guid);

HRESULT SelectBluetoothDevice()
{
	BLUETOOTH_COD_PAIRS codPairs[] = {
		{0x240418, L"Headphones"},
		{0x240404, L"Wearable Headset Device"},
		{0xffffff, L"All Classes"}
	};

	BLUETOOTH_SELECT_DEVICE_PARAMS params{sizeof(params)};
	params.fShowAuthenticated = TRUE;
	params.fShowRemembered = TRUE;
	params.fShowUnknown = TRUE;
	params.cNumDevices = 1;

	auto bSelect = BluetoothSelectDevices(&params);
	WIN32_ASSERT(bSelect || (GetLastError() == ERROR_CANCELLED));
	if(bSelect) {
		auto deviceInfo(*params.pDevices);
		BluetoothSelectDevicesFree(&params);

		BLUETOOTH_FIND_RADIO_PARAMS findParams{sizeof(findParams)};
		HANDLE hRadio;
		auto hFind = BluetoothFindFirstRadio(&findParams, &hRadio);
		WIN32_ASSERT(hFind);
		HRadio _h(hRadio);

		GUID serviceGuids[10];
		DWORD serviceCount = ARRAYSIZE(serviceGuids);
		BluetoothEnumerateInstalledServices(hRadio, &deviceInfo, &serviceCount, serviceGuids);
		wprintf_s(L"Device %s has %d Services\n", deviceInfo.szName, serviceCount);
		for(DWORD i = 0; i < serviceCount; i++) {
			wprintf_s(L"  %d %s\n", i, GuidToString(serviceGuids[i]).c_str());
		}

		// Set Bluetooth service state to ENABLE to connect to the device.
		// See https://stackoverflow.com/questions/68550324/how-to-connect-to-bluetooth-device-on-windows
		//auto serviceFlag = deviceInfo.fConnected ? BLUETOOTH_SERVICE_DISABLE : BLUETOOTH_SERVICE_ENABLE;
		wprintf_s(L"Connecting to the device %s\n", deviceInfo.szName);
		HR_ASSERT_OK(HRESULT_FROM_WIN32(BluetoothSetServiceState(hRadio, &deviceInfo, &AudioSinkServiceClass_UUID, BLUETOOTH_SERVICE_DISABLE)));
		HR_ASSERT_OK(HRESULT_FROM_WIN32(BluetoothSetServiceState(hRadio, &deviceInfo, &AudioSinkServiceClass_UUID, BLUETOOTH_SERVICE_ENABLE)));
	}

	return S_OK;
}

struct GuidValueName
{
	REFGUID guid;
	LPCWSTR str;
};

#define GUID_ITEM(x) {x, L#x}
static const GuidValueName GUIDs[] = {
	GUID_ITEM(ServiceDiscoveryServerServiceClassID_UUID),
	GUID_ITEM(BrowseGroupDescriptorServiceClassID_UUID),
	GUID_ITEM(PublicBrowseGroupServiceClass_UUID),
	GUID_ITEM(SerialPortServiceClass_UUID),
	GUID_ITEM(LANAccessUsingPPPServiceClass_UUID),
	GUID_ITEM(DialupNetworkingServiceClass_UUID),
	GUID_ITEM(IrMCSyncServiceClass_UUID),
	GUID_ITEM(OBEXObjectPushServiceClass_UUID),
	GUID_ITEM(OBEXFileTransferServiceClass_UUID),
	GUID_ITEM(IrMCSyncCommandServiceClass_UUID),
	GUID_ITEM(HeadsetServiceClass_UUID),
	GUID_ITEM(CordlessTelephonyServiceClass_UUID),
	GUID_ITEM(AudioSourceServiceClass_UUID),
	GUID_ITEM(AudioSinkServiceClass_UUID),
	GUID_ITEM(AVRemoteControlTargetServiceClass_UUID),
	GUID_ITEM(AVRemoteControlServiceClass_UUID),
	GUID_ITEM(AVRemoteControlControllerServiceClass_UUID),
	GUID_ITEM(IntercomServiceClass_UUID),
	GUID_ITEM(FaxServiceClass_UUID),
	GUID_ITEM(HeadsetAudioGatewayServiceClass_UUID),
	GUID_ITEM(WAPServiceClass_UUID),
	GUID_ITEM(WAPClientServiceClass_UUID),
	GUID_ITEM(PANUServiceClass_UUID),
	GUID_ITEM(NAPServiceClass_UUID),
	GUID_ITEM(GNServiceClass_UUID),
	GUID_ITEM(DirectPrintingServiceClass_UUID),
	GUID_ITEM(ReferencePrintingServiceClass_UUID),
	GUID_ITEM(ImagingResponderServiceClass_UUID),
	GUID_ITEM(ImagingAutomaticArchiveServiceClass_UUID),
	GUID_ITEM(ImagingReferenceObjectsServiceClass_UUID),
	GUID_ITEM(HandsfreeServiceClass_UUID),
	GUID_ITEM(HandsfreeAudioGatewayServiceClass_UUID),
	GUID_ITEM(DirectPrintingReferenceObjectsServiceClass_UUID),
	GUID_ITEM(ReflectedUIServiceClass_UUID),
	GUID_ITEM(PrintingStatusServiceClass_UUID),
	GUID_ITEM(HumanInterfaceDeviceServiceClass_UUID),
	GUID_ITEM(HCRPrintServiceClass_UUID),
	GUID_ITEM(HCRScanServiceClass_UUID),
	GUID_ITEM(CommonISDNAccessServiceClass_UUID),
	GUID_ITEM(VideoConferencingGWServiceClass_UUID),
	GUID_ITEM(UDIMTServiceClass_UUID),
	GUID_ITEM(UDITAServiceClass_UUID),
	GUID_ITEM(AudioVideoServiceClass_UUID),
	GUID_ITEM(SimAccessServiceClass_UUID),
	GUID_ITEM(PhonebookAccessPceServiceClass_UUID),
	GUID_ITEM(PhonebookAccessPseServiceClass_UUID),
	GUID_ITEM(HeadsetHSServiceClass_UUID),
	GUID_ITEM(MessageAccessServerServiceClass_UUID),
	GUID_ITEM(MessageNotificationServerServiceClass_UUID),
	GUID_ITEM(GNSSServerServiceClass_UUID),
	GUID_ITEM(ThreeDimensionalDisplayServiceClass_UUID),
	GUID_ITEM(ThreeDimensionalGlassesServiceClass_UUID),
	GUID_ITEM(MPSServiceClass_UUID),
	GUID_ITEM(CTNAccessServiceClass_UUID),
	GUID_ITEM(CTNNotificationServiceClass_UUID),
	GUID_ITEM(PnPInformationServiceClass_UUID),
	GUID_ITEM(GenericNetworkingServiceClass_UUID),
	GUID_ITEM(GenericFileTransferServiceClass_UUID),
	GUID_ITEM(GenericAudioServiceClass_UUID),
	GUID_ITEM(GenericTelephonyServiceClass_UUID),
	GUID_ITEM(UPnpServiceClass_UUID),
	GUID_ITEM(UPnpIpServiceClass_UUID),
	GUID_ITEM(ESdpUpnpIpPanServiceClass_UUID),
	GUID_ITEM(ESdpUpnpIpLapServiceClass_UUID),
	GUID_ITEM(ESdpUpnpL2capServiceClass_UUID),
	GUID_ITEM(VideoSourceServiceClass_UUID),
	GUID_ITEM(VideoSinkServiceClass_UUID),
	GUID_ITEM(HealthDeviceProfileSourceServiceClass_UUID),
	GUID_ITEM(HealthDeviceProfileSinkServiceClass_UUID),
};

std::wstring GuidToString(REFGUID guid)
{
	LPCWSTR name = L"UNKNOWN";
	for(auto& x : GUIDs) {
		if(x.guid == guid) {
			name = x.str;
			break;
		}
	}

	WCHAR strGuid[40];
	StringFromGUID2(guid, strGuid, ARRAYSIZE(strGuid));
	std::wostringstream stream;
	stream << strGuid << L":" << name;
	return stream.str();
}
