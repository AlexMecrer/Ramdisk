#include "RamDisk.h"

NTSTATUS 
EvtDiskDriverDeviceAdd(
	WDFDRIVER Driver,
	PWDFDEVICE_INIT DeviceInit
)
{
	NTSTATUS status = STATUS_SUCCESS;
	UNICODE_STRING DeviceName = RTL_CONSTANT_STRING(DEVICENAME);
	WDFDEVICE* Device;
	WDF_OBJECT_ATTRIBUTES Attr;
	WDF_OBJECT_ATTRIBUTES_INIT(&Attr);
	WDF_OBJECT_ATTRIBUTES_INIT_CONTEXT_TYPE(&Attr,DISK_EX);
	status=WdfDeviceInitAssignName(DeviceInit,&DeviceName);
	WdfDeviceInitSetIoType(DeviceInit,WdfDeviceIoDirect);
	WdfDeviceInitSetExclusive(DeviceInit,FALSE);
	WdfDeviceInitSetDeviceType(DeviceInit,FILE_DEVICE_DISK);
	status = WdfDeviceCreate(&DeviceInit,WDF_NO_OBJECT_ATTRIBUTES,Device);
	return status;
}
