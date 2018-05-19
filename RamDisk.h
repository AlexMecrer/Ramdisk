
#include<wdm.h>
#include<ntdddisk.h>
#include<ntstrsafe.h>
#include<wdf.h>
#include"Disk.h"


#define DEVICENAME L"\\Devices\\RamDisk"
#define DISKDEFULTSIZE 1024*200
#define SYMBLIC "Z:"



typedef struct _DISK_INFO {
	ULONG   DiskSize;           // Ramdisk size in bytes
	ULONG   RootDirEntries;     // No. of root directory entries
	ULONG   SectorsPerCluster;  // Sectors per cluster
	UNICODE_STRING DriveLetter; // Drive letter to be used
} DISK_INFO, *PDISK_INFO;


typedef struct _DISK_EX{
	USHORT cbSize;
	PVOID Image;
	ULONGLONG ImageSize;
	DISK_GEOMETRY DiskGeoment;
	DISK_INFO DiskInfo;
	UNICODE_STRING SymbolicName;

}DISK_EXTENSION,*PDISK_EXTENSION;

typedef struct Queue_Ex {
	USHORT cbSize;
}QUEUE_EXTENSION,*PQUEUE_EXTENSION;


//下面的宏登记一个获取函数并开辟上下文，每一个作为上下文的结构体都必须使用这个宏
WDF_DECLARE_CONTEXT_TYPE_WITH_NAME(DISK_EXTENSION,GetDiskExtension)
WDF_DECLARE_CONTEXT_TYPE_WITH_NAME(QUEUE_EXTENSION,GetQueueExtension)

NTSTATUS
EvtDiskDriverDeviceAdd(
	WDFDRIVER Driver,
	PWDFDEVICE_INIT DeviceInit
);

NTSTATUS
FormatDisk(
	PDISK_EXTENSION Disk
);


EVT_WDF_IO_QUEUE_IO_READ RamDiskEvtIoRead;
EVT_WDF_IO_QUEUE_IO_WRITE RamDiskEvtIoWrite;
EVT_WDF_IO_QUEUE_IO_DEVICE_CONTROL RamDiskEvtIoDeviceControl;