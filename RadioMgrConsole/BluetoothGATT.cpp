#include <Windows.h>
#include <SetupAPI.h>
#include <initguid.h>
#include <bluetoothleapis.h>
#include <devpkey.h>
#include <propkey.h>

#include "BluetoothCommon.h"
#include "../Common/SafeHandle.h"
#include "../Common/Assert.h"

#pragma comment(lib, "SetupApi.lib")
#pragma comment(lib, "BluetoothAPIs.lib")

static HRESULT ShowGATT(HANDLE hDevice);
static void showProperty(const DEVPROPKEY& key, DEVPROPTYPE propType, DWORD propSize, LPCVOID prop);

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
			wprintf(L"  % 3d ", i);
			showProperty(key, propType, propSize, prop.get());
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

using ShowPropFunc = void (*)(DEVPROPTYPE propType, DWORD propSize, LPCVOID prop);
static void showDeviceCapablities(DEVPROPTYPE propType, DWORD propSize, LPCVOID prop);
static void showPowerData(DEVPROPTYPE propType, DWORD propSize, LPCVOID prop);
static void showDevNodeStatus(DEVPROPTYPE propType, DWORD propSize, LPCVOID prop);
static void defaultShowPropFunc(DEVPROPTYPE propType, DWORD propSize, LPCVOID prop);

struct DevPropKeyName {
	// One of following:
	//   DEVPROPKEY defined in devpkey.h
	//   PROPERTYKEY defined in propkey.h
	const void* key;
	LPCWSTR name;
	ShowPropFunc showProc;
};

// https://stackoverflow.com/questions/71607722/how-to-know-if-a-bluetooth-device-is-connected-or-disconnected-by-powershell
// How to know if a bluetooth device is connected or disconnected by Powershell
DEFINE_PROPERTYKEY(UNKNOWN_PKEY_IsDevicerConnected, 0x83DA6326, 0x97A6, 0x4088, 0x94, 0x53, 0xA1, 0x92, 0x3F, 0x57, 0x3B, 0x29, 15);

static const DevPropKeyName devPropKeyNames[] = {
#define ITEM(x, ...) { &x, L#x, __VA_ARGS__ }
	// Definition is not found.
	ITEM(UNKNOWN_PKEY_IsDevicerConnected),

	// DEVPROPKEY defined in devpkey.h
	ITEM(DEVPKEY_Device_DeviceDesc),
	ITEM(DEVPKEY_Device_HardwareIds),
	ITEM(DEVPKEY_Device_CompatibleIds),
	ITEM(DEVPKEY_Device_Service),
	ITEM(DEVPKEY_Device_Class),
	ITEM(DEVPKEY_Device_ClassGuid),
	ITEM(DEVPKEY_Device_Driver),
	ITEM(DEVPKEY_Device_ConfigFlags),
	ITEM(DEVPKEY_Device_Manufacturer),
	ITEM(DEVPKEY_Device_FriendlyName),
	ITEM(DEVPKEY_Device_LocationInfo),
	ITEM(DEVPKEY_Device_PDOName),
	ITEM(DEVPKEY_Device_Capabilities,				showDeviceCapablities),
	ITEM(DEVPKEY_Device_UINumber),
	ITEM(DEVPKEY_Device_UpperFilters),
	ITEM(DEVPKEY_Device_LowerFilters),
	ITEM(DEVPKEY_Device_BusTypeGuid),
	ITEM(DEVPKEY_Device_LegacyBusType),
	ITEM(DEVPKEY_Device_BusNumber),
	ITEM(DEVPKEY_Device_EnumeratorName),
	ITEM(DEVPKEY_Device_Security),
	ITEM(DEVPKEY_Device_SecuritySDS),
	ITEM(DEVPKEY_Device_DevType),
	ITEM(DEVPKEY_Device_Exclusive),
	ITEM(DEVPKEY_Device_Characteristics),
	ITEM(DEVPKEY_Device_Address),
	ITEM(DEVPKEY_Device_UINumberDescFormat),
	ITEM(DEVPKEY_Device_PowerData,					showPowerData),
	ITEM(DEVPKEY_Device_RemovalPolicy),
	ITEM(DEVPKEY_Device_RemovalPolicyDefault),
	ITEM(DEVPKEY_Device_RemovalPolicyOverride),
	ITEM(DEVPKEY_Device_InstallState),
	ITEM(DEVPKEY_Device_LocationPaths),
	ITEM(DEVPKEY_Device_BaseContainerId),
	ITEM(DEVPKEY_Device_InstanceId),
	ITEM(DEVPKEY_Device_DevNodeStatus,				showDevNodeStatus),
	ITEM(DEVPKEY_Device_ProblemCode),
	ITEM(DEVPKEY_Device_EjectionRelations),
	ITEM(DEVPKEY_Device_RemovalRelations),
	ITEM(DEVPKEY_Device_PowerRelations),
	ITEM(DEVPKEY_Device_BusRelations),
	ITEM(DEVPKEY_Device_Parent),
	ITEM(DEVPKEY_Device_Children),
	ITEM(DEVPKEY_Device_Siblings),
	ITEM(DEVPKEY_Device_TransportRelations),
	ITEM(DEVPKEY_Device_ProblemStatus),
	ITEM(DEVPKEY_Device_Reported),
	ITEM(DEVPKEY_Device_Legacy),
	ITEM(DEVPKEY_Device_ContainerId),
	ITEM(DEVPKEY_Device_InLocalMachineContainer),
	ITEM(DEVPKEY_Device_Model),
	ITEM(DEVPKEY_Device_ModelId),
	ITEM(DEVPKEY_Device_FriendlyNameAttributes),
	ITEM(DEVPKEY_Device_ManufacturerAttributes),
	ITEM(DEVPKEY_Device_PresenceNotForDevice),
	ITEM(DEVPKEY_Device_SignalStrength),
	ITEM(DEVPKEY_Device_IsAssociateableByUserAction),
	ITEM(DEVPKEY_Device_ShowInUninstallUI),
	ITEM(DEVPKEY_Device_Numa_Proximity_Domain),
	ITEM(DEVPKEY_Device_DHP_Rebalance_Policy),
	ITEM(DEVPKEY_Device_Numa_Node),
	ITEM(DEVPKEY_Device_BusReportedDeviceDesc),
	ITEM(DEVPKEY_Device_IsPresent),
	ITEM(DEVPKEY_Device_HasProblem),
	ITEM(DEVPKEY_Device_ConfigurationId),
	ITEM(DEVPKEY_Device_ReportedDeviceIdsHash),
	ITEM(DEVPKEY_Device_PhysicalDeviceLocation),
	ITEM(DEVPKEY_Device_BiosDeviceName),
	ITEM(DEVPKEY_Device_DriverProblemDesc),
	ITEM(DEVPKEY_Device_DebuggerSafe),
	ITEM(DEVPKEY_Device_PostInstallInProgress),
	ITEM(DEVPKEY_Device_Stack),
	ITEM(DEVPKEY_Device_ExtendedConfigurationIds),
	ITEM(DEVPKEY_Device_IsRebootRequired),
	ITEM(DEVPKEY_Device_FirmwareDate),
	ITEM(DEVPKEY_Device_FirmwareVersion),
	ITEM(DEVPKEY_Device_FirmwareRevision),
	ITEM(DEVPKEY_Device_DependencyProviders),
	ITEM(DEVPKEY_Device_DependencyDependents),
	ITEM(DEVPKEY_Device_SoftRestartSupported),
	ITEM(DEVPKEY_Device_ExtendedAddress),
	ITEM(DEVPKEY_Device_AssignedToGuest),
	ITEM(DEVPKEY_Device_CreatorProcessId),
	ITEM(DEVPKEY_Device_FirmwareVendor),
	ITEM(DEVPKEY_Device_SessionId),
	ITEM(DEVPKEY_Device_InstallDate),
	ITEM(DEVPKEY_Device_FirstInstallDate),
	ITEM(DEVPKEY_Device_LastArrivalDate),
	ITEM(DEVPKEY_Device_LastRemovalDate),
	ITEM(DEVPKEY_Device_DriverDate),
	ITEM(DEVPKEY_Device_DriverVersion),
	ITEM(DEVPKEY_Device_DriverDesc),
	ITEM(DEVPKEY_Device_DriverInfPath),
	ITEM(DEVPKEY_Device_DriverInfSection),
	ITEM(DEVPKEY_Device_DriverInfSectionExt),
	ITEM(DEVPKEY_Device_MatchingDeviceId),
	ITEM(DEVPKEY_Device_DriverProvider),
	ITEM(DEVPKEY_Device_DriverPropPageProvider),
	ITEM(DEVPKEY_Device_DriverCoInstallers),
	ITEM(DEVPKEY_Device_ResourcePickerTags),
	ITEM(DEVPKEY_Device_ResourcePickerExceptions),
	ITEM(DEVPKEY_Device_DriverRank),
	ITEM(DEVPKEY_Device_DriverLogoLevel),
	ITEM(DEVPKEY_Device_NoConnectSound),
	ITEM(DEVPKEY_Device_GenericDriverInstalled),
	ITEM(DEVPKEY_Device_AdditionalSoftwareRequested),
	ITEM(DEVPKEY_Device_SafeRemovalRequired),
	ITEM(DEVPKEY_Device_SafeRemovalRequiredOverride),
	ITEM(DEVPKEY_DeviceContainer_Address),
	ITEM(DEVPKEY_DeviceContainer_DiscoveryMethod),
	ITEM(DEVPKEY_DeviceContainer_IsEncrypted),
	ITEM(DEVPKEY_DeviceContainer_IsAuthenticated),
	ITEM(DEVPKEY_DeviceContainer_IsConnected),
	ITEM(DEVPKEY_DeviceContainer_IsPaired),
	ITEM(DEVPKEY_DeviceContainer_Icon),
	ITEM(DEVPKEY_DeviceContainer_Version),
	ITEM(DEVPKEY_DeviceContainer_Last_Seen),
	ITEM(DEVPKEY_DeviceContainer_Last_Connected),
	ITEM(DEVPKEY_DeviceContainer_IsShowInDisconnectedState),
	ITEM(DEVPKEY_DeviceContainer_IsLocalMachine),
	ITEM(DEVPKEY_DeviceContainer_MetadataPath),
	ITEM(DEVPKEY_DeviceContainer_IsMetadataSearchInProgress),
	ITEM(DEVPKEY_DeviceContainer_MetadataChecksum),
	ITEM(DEVPKEY_DeviceContainer_IsNotInterestingForDisplay),
	ITEM(DEVPKEY_DeviceContainer_LaunchDeviceStageOnDeviceConnect),
	ITEM(DEVPKEY_DeviceContainer_LaunchDeviceStageFromExplorer),
	ITEM(DEVPKEY_DeviceContainer_BaselineExperienceId),
	ITEM(DEVPKEY_DeviceContainer_IsDeviceUniquelyIdentifiable),
	ITEM(DEVPKEY_DeviceContainer_AssociationArray),
	ITEM(DEVPKEY_DeviceContainer_DeviceDescription1),
	ITEM(DEVPKEY_DeviceContainer_DeviceDescription2),
	ITEM(DEVPKEY_DeviceContainer_HasProblem),
	ITEM(DEVPKEY_DeviceContainer_IsSharedDevice),
	ITEM(DEVPKEY_DeviceContainer_IsNetworkDevice),
	ITEM(DEVPKEY_DeviceContainer_IsDefaultDevice),
	ITEM(DEVPKEY_DeviceContainer_MetadataCabinet),
	ITEM(DEVPKEY_DeviceContainer_RequiresPairingElevation),
	ITEM(DEVPKEY_DeviceContainer_ExperienceId),
	ITEM(DEVPKEY_DeviceContainer_Category),
	ITEM(DEVPKEY_DeviceContainer_Category_Desc_Singular),
	ITEM(DEVPKEY_DeviceContainer_Category_Desc_Plural),
	ITEM(DEVPKEY_DeviceContainer_Category_Icon),
	ITEM(DEVPKEY_DeviceContainer_CategoryGroup_Desc),
	ITEM(DEVPKEY_DeviceContainer_CategoryGroup_Icon),
	ITEM(DEVPKEY_DeviceContainer_PrimaryCategory),
	ITEM(DEVPKEY_DeviceContainer_UnpairUninstall),
	ITEM(DEVPKEY_DeviceContainer_RequiresUninstallElevation),
	ITEM(DEVPKEY_DeviceContainer_DeviceFunctionSubRank),
	ITEM(DEVPKEY_DeviceContainer_AlwaysShowDeviceAsConnected),
	ITEM(DEVPKEY_DeviceContainer_ConfigFlags),
	ITEM(DEVPKEY_DeviceContainer_PrivilegedPackageFamilyNames),
	ITEM(DEVPKEY_DeviceContainer_CustomPrivilegedPackageFamilyNames),
	ITEM(DEVPKEY_DeviceContainer_IsRebootRequired),
	ITEM(DEVPKEY_DeviceContainer_FriendlyName),
	ITEM(DEVPKEY_DeviceContainer_Manufacturer),
	ITEM(DEVPKEY_DeviceContainer_ModelName),
	ITEM(DEVPKEY_DeviceContainer_ModelNumber),
	ITEM(DEVPKEY_DeviceContainer_InstallInProgress),

	// PROPERTYKEY defined in propkey.h
	ITEM(PKEY_Device_PrinterURL),
	ITEM(PKEY_DeviceInterface_Bluetooth_DeviceAddress),
	ITEM(PKEY_DeviceInterface_Bluetooth_Flags),
	ITEM(PKEY_DeviceInterface_Bluetooth_LastConnectedTime),
	ITEM(PKEY_DeviceInterface_Bluetooth_Manufacturer),
	ITEM(PKEY_DeviceInterface_Bluetooth_ModelNumber),
	ITEM(PKEY_DeviceInterface_Bluetooth_ProductId),
	ITEM(PKEY_DeviceInterface_Bluetooth_ProductVersion),
	ITEM(PKEY_DeviceInterface_Bluetooth_ServiceGuid),
	ITEM(PKEY_DeviceInterface_Bluetooth_VendorId),
	ITEM(PKEY_DeviceInterface_Bluetooth_VendorIdSource),
	ITEM(PKEY_Devices_Aep_ProtocolId),
	ITEM(PKEY_ItemNameDisplay),
	ITEM(PKEY_Devices_Aep_DeviceAddress),
	ITEM(PKEY_Devices_Aep_AepId),
	ITEM(DEVPKEY_Device_Legacy),
#undef ITEM
};

void showProperty(const DEVPROPKEY& key, DEVPROPTYPE propType, DWORD propSize, LPCVOID prop)
{
	LPCWSTR keyName = nullptr;
	ShowPropFunc showPropFunc = nullptr;
	for(auto& x : devPropKeyNames) {
		if(*(DEVPROPKEY*)x.key == key) {
			keyName = x.name;
			showPropFunc = x.showProc;
			break;
		}
	}

	WCHAR name[100];
	if(!keyName) {
		swprintf_s(name, L"%s,%d", GuidToString(key.fmtid).c_str(), key.pid);
		keyName = name;
	}
	wprintf(L"%-45s ", keyName);

	if(!showPropFunc) { showPropFunc = defaultShowPropFunc; }
	showPropFunc(propType, propSize, prop);
}

// Shows property value for DEVPKEY_Device_Capabilities.
void showDeviceCapablities(DEVPROPTYPE propType, DWORD propSize, LPCVOID prop)
{
	if((propType != DEVPROP_TYPE_UINT32) || (propSize != sizeof(UINT32))) {
		defaultShowPropFunc(propType, propSize, prop);
		return;
	}

	// Names for device capabilities bit mask found in DEVICE_CAPABILITIES structure.
	// See https://learn.microsoft.com/ja-jp/windows-hardware/drivers/ddi/wdm/ns-wdm-_device_capabilities
	static const LPCWSTR capNames[] = {
		// Note: Bit mask retrieved as property value seems to not include DeviceD1/2 flags though DEVICE_CAPABILITIES includes them.
		//       Ignoring these flags makes interpreted string as same as capabilities shown by Device Manager.
		//L"DeviceD1",
		//L"DeviceD2",
		L"LockSupported",
		L"EjectSupported",
		L"Removable",
		L"DockDevice",
		L"UniqueID",
		L"SilentInstall",
		L"RawDeviceOK",
		L"SurpriseRemovalOK",
		L"WakeFromD0",
		L"WakeFromD1",
		L"WakeFromD2",
		L"WakeFromD3",
		L"HardwareDisabled",
		L"NonDynamic",
		L"WarmEjectSupported",
		L"NoDisplayInUI",
		L"Reserved1",
		L"WakeFromInterrupt",
		L"SecureDevice",
		L"ChildOfVgaEnabledBridge",
		L"DecodeIoOnBoot",
	};

	auto devCap = *(UINT32*)prop;
	wprintf(L"0x%x: %s", devCap, BitMaskToString(devCap, capNames, ARRAYSIZE(capNames)).c_str());
}

// Shows property value in CM_POWER_DATA structure for DEVPKEY_Device_PowerData key.
void showPowerData(DEVPROPTYPE propType, DWORD propSize, LPCVOID prop)
{
	if((propType != DEVPROP_TYPE_BINARY) || (propSize != sizeof(CM_POWER_DATA))) {
		defaultShowPropFunc(propType, propSize, prop);
		return;
	}

	// Names for CM_POWER_DATA::PD_Capabilities bitmask.
	static const LPCWSTR caps[] = {
		L"PDCAP_D0_SUPPORTED",				// 0x00000001
		L"PDCAP_D1_SUPPORTED",				// 0x00000002
		L"PDCAP_D2_SUPPORTED",				// 0x00000004
		L"PDCAP_D3_SUPPORTED",				// 0x00000008
		L"PDCAP_WAKE_FROM_D0_SUPPORTED",	// 0x00000010
		L"PDCAP_WAKE_FROM_D1_SUPPORTED",	// 0x00000020
		L"PDCAP_WAKE_FROM_D2_SUPPORTED",	// 0x00000040
		L"PDCAP_WAKE_FROM_D3_SUPPORTED",	// 0x00000080
		L"PDCAP_WARM_EJECT_SUPPORTED",		// 0x00000100
	};

	// Names for CM_POWER_DATA::PD_MostRecentPowerState enum.
	static const LPCWSTR states[] = {
		L"PowerDeviceUnspecified",	// = 0,
		L"PowerDeviceD0",
		L"PowerDeviceD1",
		L"PowerDeviceD2",
		L"PowerDeviceD3",
		L"PowerDeviceMaximum"
	};

	auto ppd = (CM_POWER_DATA*)prop;
	wprintf(
		L"CM_POWER_DATA"
		L"\n        MostRecentPowerState = %s(%d)"
		L"\n        Capabilities = 0x%x: %s"
		L"\n        Latency: D1 = %0.1f mSec, D2 = %0.1f mSec, D3 = %0.1f mSec"
		,
		GetArrayItem(states, ppd->PD_MostRecentPowerState), ppd->PD_MostRecentPowerState,
		ppd->PD_Capabilities, BitMaskToString(ppd->PD_Capabilities, caps, ARRAYSIZE(caps)).c_str(),
		(double)ppd->PD_D1Latency / 10, (double)ppd->PD_D2Latency / 10, (double)ppd->PD_D3Latency / 10
	);

	wprintf(L"\n        PowerStateMapping");
	// Note: Device Manager shows S0 ~ S5 of Power State Mapping
	for(int i = 0; i <= 5; i++) {
		auto state = ppd->PD_PowerStateMapping[i];
		wprintf(L"\n          S%d -> %s(%d)",
			i, GetArrayItem(states, state), state
		);
	}

	wprintf(L"\n        DeepestSystemWake = %s(%d)",
		GetArrayItem(states, ppd->PD_DeepestSystemWake), ppd->PD_DeepestSystemWake
	);
}

void showDevNodeStatus(DEVPROPTYPE propType, DWORD propSize, LPCVOID prop)
{
	if((propType != DEVPROP_TYPE_UINT32) || (propSize != sizeof(UINT32))) {
		defaultShowPropFunc(propType, propSize, prop);
		return;
	}

	// Names for DevNodeStatus bitmask defined in cfg.h.
	static const LPCWSTR statusNames[] = {
		//
		// Device Instance status flags, returned by call to CM_Get_DevInst_Status
		//
		L"DN_ROOT_ENUMERATED",	// (0x00000001) // Was enumerated by ROOT
		L"DN_DRIVER_LOADED",	// (0x00000002) // Has Register_Device_Driver
		L"DN_ENUM_LOADED",		// (0x00000004) // Has Register_Enumerator
		L"DN_STARTED",			// (0x00000008) // Is currently configured
		L"DN_MANUAL",			// (0x00000010) // Manually installed
		L"DN_NEED_TO_ENUM",		// (0x00000020) // May need reenumeration
		L"DN_NOT_FIRST_TIME",	// (0x00000040) // Has received a config
		L"DN_HARDWARE_ENUM",	// (0x00000080) // Enum generates hardware ID
		L"DN_LIAR",				// (0x00000100) // Lied about can reconfig once
		L"DN_HAS_MARK",			// (0x00000200) // Not CM_Create_DevInst lately
		L"DN_HAS_PROBLEM",		// (0x00000400) // Need device installer
		L"DN_FILTERED",			// (0x00000800) // Is filtered
		L"DN_MOVED",			// (0x00001000) // Has been moved
		L"DN_DISABLEABLE",		// (0x00002000) // Can be disabled
		L"DN_REMOVABLE",		// (0x00004000) // Can be removed
		L"DN_PRIVATE_PROBLEM",	// (0x00008000) // Has a private problem
		L"DN_MF_PARENT",		// (0x00010000) // Multi function parent
		L"DN_MF_CHILD",			// (0x00020000) // Multi function child
		L"DN_WILL_BE_REMOVED",	// (0x00040000) // DevInst is being removed
		//
		// Windows 4 OPK2 Flags
		//
		L"DN_NOT_FIRST_TIMEE",	// 0x00080000  // S: Has received a config enumerate
		L"DN_STOP_FREE_RES",	// 0x00100000  // S: When child is stopped, free resources
		L"DN_REBAL_CANDIDATE",	// 0x00200000  // S: Don't skip during rebalance
		L"DN_BAD_PARTIAL",		// 0x00400000  // S: This devnode's log_confs do not have same resources
		L"DN_NT_ENUMERATOR",	// 0x00800000  // S: This devnode's is an NT enumerator
		L"DN_NT_DRIVER",		// 0x01000000  // S: This devnode's is an NT driver
		//
		// Windows 4.1 Flags
		//
		//L"DN_NEEDS_LOCKING",	// 0x02000000  // S: Devnode need lock resume processing
		L"DN_DEVICE_DISCONNECTED",		// Device Manager shows this instead of DN_NEEDS_LOCKING.
		L"DN_ARM_WAKEUP",		// 0x04000000  // S: Devnode can be the wakeup device
		L"DN_APM_ENUMERATOR",	// 0x08000000  // S: APM aware enumerator
		L"DN_APM_DRIVER",		// 0x10000000  // S: APM aware driver
		L"DN_SILENT_INSTALL",	// 0x20000000  // S: Silent install
		L"DN_NO_SHOW_IN_DM",	// 0x40000000  // S: No show in device manager
		L"DN_BOOT_LOG_PROB",	// 0x80000000  // S: Had a problem during preassignment of boot log conf
	};

	auto status = *(UINT32*)prop;
	wprintf(L"0x%x: %s", status, BitMaskToString(status, statusNames, ARRAYSIZE(statusNames)).c_str());
}

void defaultShowPropFunc(DEVPROPTYPE propType, DWORD propSize, LPCVOID prop)
{
	switch(propType) {
	case DEVPROP_TYPE_BOOLEAN:
		wprintf(L"BOOLEAN %s", *(BYTE*)prop ? L"TRUE" : L"FALSE");
		break;
	case DEVPROP_TYPE_BYTE:
		wprintf(L"BYTE %u", *(BYTE*)prop);
		break;
	case DEVPROP_TYPE_INT16:
		wprintf(L"INT16 %d", *(INT16*)prop);
		break;
	case DEVPROP_TYPE_UINT16:
		wprintf(L"UINT16 %u", *(UINT16*)prop);
		break;
	case DEVPROP_TYPE_INT32:
		wprintf(L"UINT32 %d", *(INT32*)prop);
		break;
	case DEVPROP_TYPE_UINT32:
		wprintf(L"UINT32 %u", *(UINT32*)prop);
		break;
	case DEVPROP_TYPE_INT64:
		wprintf(L"UINT64 %I64d", *(INT64*)prop);
		break;
	case DEVPROP_TYPE_UINT64:
		wprintf(L"UINT64 %I64u", *(UINT64*)prop);
		break;
	case DEVPROP_TYPE_FILETIME:
	{
		FILETIME ft;
		FileTimeToLocalFileTime((FILETIME*)prop, &ft);
		SYSTEMTIME t;
		FileTimeToSystemTime(&ft, &t);
		wprintf(L"%04d/%02d/%02d %02d:%02d:%02d.%03d", t.wYear, t.wMonth, t.wDay, t.wHour, t.wMinute, t.wSecond, t.wMilliseconds);
	}
	break;
	case DEVPROP_TYPE_GUID:
		wprintf(L"%s", GuidToString(*(GUID*)prop).c_str());
		break;
	case DEVPROP_TYPE_STRING:
		wprintf(L"`%s`", (WCHAR*)prop);
		break;
	case DEVPROP_TYPE_STRING_LIST:
		// Note: Number of characters includes number of NULL terminaters(end of each string and end of the list).
		wprintf(L"STRING_LIST %d characters", (int)(propSize / sizeof(WCHAR)));
		for(auto p = (WCHAR*)prop; *p; p += (wcslen(p) + 1)) {
			wprintf(L"\n        `%s`", p);
		}
		break;
	case DEVPROP_TYPE_BINARY:
		wprintf(L"BINARY %d bytes", propSize);
		for(DWORD i = 0; i < propSize; i++) {
			if((i % 16) == 0) { wprintf(L"\n       "); }
			wprintf(L" %02x", ((BYTE*)prop)[i]);
		}
		break;
	default:
		wprintf(L"Type = 0x%08x, Size = %d", propType, propSize);
		break;
	}
}
