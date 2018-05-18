#include"RamDisk.h"

NTSTATUS
DriverEntry(
	IN PDRIVER_OBJECT DriverObject,
	IN PUNICODE_STRING RegistryPath
)
{
	WDF_DRIVER_CONFIG Config;
	WDF_DRIVER_CONFIG_INIT(&Config,EvtDiskDriverDeviceAdd);
	return WdfDriverCreate(DriverObject,RegistryPath, WDF_NO_OBJECT_ATTRIBUTES,&Config,WDF_NO_HANDLE);
}
