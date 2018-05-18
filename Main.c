#include"RamDisk.h"

NTSTATUS
DriverEntry(
	IN PDRIVER_OBJECT DriverObject,
	IN PUNICODE_STRING RegistryPath
)
/*
函数名：DriverEntry
功能描述：设置驱动的DriverAdd函数
返回值；依赖于WdfDriverCreate函数
*/
{
	WDF_DRIVER_CONFIG Config;
	WDF_DRIVER_CONFIG_INIT(&Config,EvtDiskDriverDeviceAdd);
	return WdfDriverCreate(DriverObject,RegistryPath, WDF_NO_OBJECT_ATTRIBUTES,&Config,WDF_NO_HANDLE);
}


