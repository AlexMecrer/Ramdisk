#include "RamDisk.h"

NTSTATUS 
EvtDiskDriverDeviceAdd(
	WDFDRIVER Driver,
	PWDFDEVICE_INIT DeviceInit
)
/*
��������EvtDiskDriverDeviceAdd
������ʹ��WdfDeviceInit*�������ý�Ҫ�����豸�����ԣ�����DeviceInit����ʹ��Attr
�����豸�������ĵ���WdfDeviceCreate����Ŀ���豸
����ֵ������WdfDeviceCreate
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
	WDF_OBJECT_ATTRIBUTES_INIT_CONTEXT_TYPE(&Attr,DISK_EXTENSION);//�����豸������
	status = WdfDeviceCreate(&DeviceInit,&Attr,Device);
	return status;
}
