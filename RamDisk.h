
#include<wdm.h>
#include<ntdddisk.h>
#include<ntstrsafe.h>
#include<wdf.h>

#define DEVICENAME L"\\Devices\\RamDisk"



typedef struct {
	USHORT cbSize;
}DISK_EX,*PDISK_EX;




NTSTATUS
EvtDiskDriverDeviceAdd(
	WDFDRIVER Driver,
	PWDFDEVICE_INIT DeviceInit
);