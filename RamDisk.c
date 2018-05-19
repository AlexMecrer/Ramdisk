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
	WDFDEVICE Device;
	WDF_OBJECT_ATTRIBUTES Attr;
	WDF_IO_QUEUE_CONFIG Config;
	WDFQUEUE Queue;
	WDF_OBJECT_ATTRIBUTES_INIT(&Attr);
	status=WdfDeviceInitAssignName(DeviceInit,&DeviceName);
	WdfDeviceInitSetIoType(DeviceInit,WdfDeviceIoDirect);
	WdfDeviceInitSetExclusive(DeviceInit,FALSE);
	WdfDeviceInitSetDeviceType(DeviceInit,FILE_DEVICE_DISK);
	WDF_OBJECT_ATTRIBUTES_INIT_CONTEXT_TYPE(&Attr,DISK_EXTENSION);//�����豸������
	status = WdfDeviceCreate(&DeviceInit,&Attr,&Device);
	if (!NT_SUCCESS(status))
	{
		KdPrint(("Disk Device Create Fail\r\n"));
	}
	WDF_IO_QUEUE_CONFIG_INIT(&Config,WdfIoQueueDispatchSequential);
	Config.EvtIoRead = RamDiskEvtIoRead;
	Config.EvtIoWrite = RamDiskEvtIoWrite;
	Config.EvtIoDeviceControl = RamDiskEvtIoDeviceControl;
	WDF_OBJECT_ATTRIBUTES_INIT_CONTEXT_TYPE(&Attr, QUEUE_EXTENSION);
	status=WdfIoQueueCreate(Device,&Config,&Attr,&Queue);
	if (!NT_SUCCESS(status))
	{
		KdPrint(("Queue Create Fail\r\n"));
	}
	PDISK_EXTENSION Disk = GetDiskExtension(Device);
	Disk->cbSize = sizeof(DISK_EXTENSION);
	Disk->SymbolicName = RTL_CONSTANT_STRING(SYMBLIC);
	Disk->Image = ExAllocatePoolWithTag(NonPagedPool,Disk->DiskInfo.DiskSize,'Tgz');

	return status;
}

NTSTATUS 
FormatDisk(
	PDISK_EXTENSION Disk
)
/*
��������ʽ��Ŀ���豸
������Ŀ���豸��Disk��
����ֵ��
*/
{
	NTSTATUS status = STATUS_SUCCESS;
	PBOOT_SECTOR sector = (PBOOT_SECTOR)Disk->Image;
	PAGED_CODE();
	ASSERT(sizeof(BOOT_SECTOR)==512);
	ASSERT(Disk->Image!=NULL);
	RtlZeroMemory(Disk->Image,Disk->DiskInfo.DiskSize);
	return status;
}
