
#include<wdm.h>
#include<ntdddisk.h>
#include<ntstrsafe.h>
#include<wdf.h>

#define DEVICENAME L"\\Devices\\RamDisk"


//����ĺ�Ǽ�һ����ȡ���������������ģ�ÿһ����Ϊ�����ĵĽṹ�嶼����ʹ�������
typedef struct _DISK_EX{
	USHORT cbSize;
}DISK_EXTENSION,*PDISK_EXTENSION;

WDF_DECLARE_CONTEXT_TYPE_WITH_NAME(DISK_EXTENSION,GetDiskExtension)


NTSTATUS
EvtDiskDriverDeviceAdd(
	WDFDRIVER Driver,
	PWDFDEVICE_INIT DeviceInit
);