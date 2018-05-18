
#include<wdm.h>
#include<ntdddisk.h>
#include<ntstrsafe.h>
#include<wdf.h>

#define DEVICENAME L"\\Devices\\RamDisk"


//����ĺ�Ǽ�һ����ȡ���������������ģ�ÿһ����Ϊ�����ĵĽṹ�嶼����ʹ�������
typedef struct _DISK_EX{
	USHORT cbSize;
}DISK_EXTENSION,*PDISK_EXTENSION;

typedef struct Queue_Ex {
	USHORT cbSize;
}QUEUE_EXTENSION,*PQUEUE_EXTENSION;

WDF_DECLARE_CONTEXT_TYPE_WITH_NAME(DISK_EXTENSION,GetDiskExtension)
WDF_DECLARE_CONTEXT_TYPE_WITH_NAME(QUEUE_EXTENSION,GetQueueExtension)

NTSTATUS
EvtDiskDriverDeviceAdd(
	WDFDRIVER Driver,
	PWDFDEVICE_INIT DeviceInit
);

EVT_WDF_IO_QUEUE_IO_READ RamDiskEvtIoRead;
EVT_WDF_IO_QUEUE_IO_WRITE RamDiskEvtIoWrite;
EVT_WDF_IO_QUEUE_IO_DEVICE_CONTROL RamDiskEvtIoDeviceControl;