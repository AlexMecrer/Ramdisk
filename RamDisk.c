#include "RamDisk.h"

NTSTATUS 
EvtDiskDriverDeviceAdd(
	WDFDRIVER Driver,
	PWDFDEVICE_INIT DeviceInit
)
/*
函数名：EvtDiskDriverDeviceAdd
描述：使用WdfDeviceInit*函数设置将要创建设备的属性（操作DeviceInit），使用Attr
设置设备的上下文调用WdfDeviceCreate创建目标设备
返回值：依赖WdfDeviceCreate
*/
{
	NTSTATUS status = STATUS_SUCCESS;
	UNICODE_STRING DeviceName = RTL_CONSTANT_STRING(DEVICENAME);
	WDFDEVICE* Device;
	WDF_OBJECT_ATTRIBUTES Attr;
	WDF_OBJECT_ATTRIBUTES_INIT(&Attr);
	status=WdfDeviceInitAssignName(DeviceInit,&DeviceName);
	WdfDeviceInitSetIoType(DeviceInit,WdfDeviceIoDirect);
	WdfDeviceInitSetExclusive(DeviceInit,FALSE);
	WdfDeviceInitSetDeviceType(DeviceInit,FILE_DEVICE_DISK);
	WDF_OBJECT_ATTRIBUTES_INIT_CONTEXT_TYPE(&Attr,DISK_EXTENSION);//设置设备上下文
	status = WdfDeviceCreate(&DeviceInit,&Attr,Device);
	return status;
}
