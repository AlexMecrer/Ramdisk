
#include<wdm.h>
#include<ntdddisk.h>
#include<ntstrsafe.h>
#include<wdf.h>

#define DEVICENAME L"\\Devices\\RamDisk"


//下面的宏登记一个获取函数并开辟上下文，每一个作为上下文的结构体都必须使用这个宏
typedef struct _DISK_EX{
	USHORT cbSize;
}DISK_EXTENSION,*PDISK_EXTENSION;

WDF_DECLARE_CONTEXT_TYPE_WITH_NAME(DISK_EXTENSION,GetDiskExtension)


NTSTATUS
EvtDiskDriverDeviceAdd(
	WDFDRIVER Driver,
	PWDFDEVICE_INIT DeviceInit
);