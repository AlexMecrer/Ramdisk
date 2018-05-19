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
	WDFDEVICE Device;
	WDF_OBJECT_ATTRIBUTES Attr;
	WDF_IO_QUEUE_CONFIG Config;
	WDFQUEUE Queue;
	WDF_OBJECT_ATTRIBUTES_INIT(&Attr);
	status=WdfDeviceInitAssignName(DeviceInit,&DeviceName);
	WdfDeviceInitSetIoType(DeviceInit,WdfDeviceIoDirect);
	WdfDeviceInitSetExclusive(DeviceInit,FALSE);
	WdfDeviceInitSetDeviceType(DeviceInit,FILE_DEVICE_DISK);
	WDF_OBJECT_ATTRIBUTES_INIT_CONTEXT_TYPE(&Attr,DISK_EXTENSION);//设置设备上下文
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
	FormatDisk(Disk);
	return status;
}

NTSTATUS 
FormatDisk(
	PDISK_EXTENSION devExt
)
/*
描述：格式化目标设备
参数：目标设备（Disk）
返回值：
*/
{
	PBOOT_SECTOR bootSector = (PBOOT_SECTOR)devExt->DiskImage;
	PUCHAR       firstFatSector;
	ULONG        rootDirEntries;
	ULONG        sectorsPerCluster;
	USHORT       fatType;        // Type FAT 12 or 16
	USHORT       fatEntries;     // Number of cluster entries in FAT
	USHORT       fatSectorCnt;   // Number of sectors for FAT
	PDIR_ENTRY   rootDir;        // Pointer to first entry in root dir

	PAGED_CODE();
	ASSERT(sizeof(BOOT_SECTOR) == 512);
	ASSERT(devExt->DiskImage != NULL);

	RtlZeroMemory(devExt->DiskImage, devExt->DiskRegInfo.DiskSize);

	devExt->DiskGeometry.BytesPerSector = 512;
	devExt->DiskGeometry.SectorsPerTrack = 32;     // Using Ramdisk value
	devExt->DiskGeometry.TracksPerCylinder = 2;    // Using Ramdisk value

												   //
												   // Calculate number of cylinders.
												   //

	devExt->DiskGeometry.Cylinders.QuadPart = devExt->DiskRegInfo.DiskSize / 512 / 32 / 2;

	//
	// Our media type is RAMDISK_MEDIA_TYPE
	//

	devExt->DiskGeometry.MediaType = RAMDISK_MEDIA_TYPE;

	KdPrint((
		"Cylinders: %ld\n TracksPerCylinder: %ld\n SectorsPerTrack: %ld\n BytesPerSector: %ld\n",
		devExt->DiskGeometry.Cylinders.QuadPart, devExt->DiskGeometry.TracksPerCylinder,
		devExt->DiskGeometry.SectorsPerTrack, devExt->DiskGeometry.BytesPerSector
		));

	rootDirEntries = devExt->DiskRegInfo.RootDirEntries;
	sectorsPerCluster = devExt->DiskRegInfo.SectorsPerCluster;

	//
	// Round Root Directory entries up if necessary
	//

	if (rootDirEntries & (DIR_ENTRIES_PER_SECTOR - 1)) {

		rootDirEntries =
			(rootDirEntries + (DIR_ENTRIES_PER_SECTOR - 1)) &
			~(DIR_ENTRIES_PER_SECTOR - 1);
	}

	KdPrint((
		"Root dir entries: %ld\n Sectors/cluster: %ld\n",
		rootDirEntries, sectorsPerCluster
		));

	//
	// We need to have the 0xeb and 0x90 since this is one of the
	// checks the file system recognizer uses
	//

	bootSector->bsJump[0] = 0xeb;
	bootSector->bsJump[1] = 0x3c;
	bootSector->bsJump[2] = 0x90;

	//
	// Set OemName to "RajuRam "
	// NOTE: Fill all 8 characters, eg. sizeof(bootSector->bsOemName);
	//
	bootSector->bsOemName[0] = 'R';
	bootSector->bsOemName[1] = 'a';
	bootSector->bsOemName[2] = 'j';
	bootSector->bsOemName[3] = 'u';
	bootSector->bsOemName[4] = 'R';
	bootSector->bsOemName[5] = 'a';
	bootSector->bsOemName[6] = 'm';
	bootSector->bsOemName[7] = ' ';

	bootSector->bsBytesPerSec = (SHORT)devExt->DiskGeometry.BytesPerSector;
	bootSector->bsResSectors = 1;
	bootSector->bsFATs = 1;
	bootSector->bsRootDirEnts = (USHORT)rootDirEntries;

	bootSector->bsSectors = (USHORT)(devExt->DiskRegInfo.DiskSize /
		devExt->DiskGeometry.BytesPerSector);
	bootSector->bsMedia = (UCHAR)devExt->DiskGeometry.MediaType;
	bootSector->bsSecPerClus = (UCHAR)sectorsPerCluster;

	//
	// Calculate number of sectors required for FAT
	//

	fatEntries =
		(bootSector->bsSectors - bootSector->bsResSectors -
			bootSector->bsRootDirEnts / DIR_ENTRIES_PER_SECTOR) /
		bootSector->bsSecPerClus + 2;

	//
	// Choose between 12 and 16 bit FAT based on number of clusters we
	// need to map
	//

	if (fatEntries > 4087) {
		fatType = 16;
		fatSectorCnt = (fatEntries * 2 + 511) / 512;
		fatEntries = fatEntries + fatSectorCnt;
		fatSectorCnt = (fatEntries * 2 + 511) / 512;
	}
	else {
		fatType = 12;
		fatSectorCnt = (((fatEntries * 3 + 1) / 2) + 511) / 512;
		fatEntries = fatEntries + fatSectorCnt;
		fatSectorCnt = (((fatEntries * 3 + 1) / 2) + 511) / 512;
	}

	bootSector->bsFATsecs = fatSectorCnt;
	bootSector->bsSecPerTrack = (USHORT)devExt->DiskGeometry.SectorsPerTrack;
	bootSector->bsHeads = (USHORT)devExt->DiskGeometry.TracksPerCylinder;
	bootSector->bsBootSignature = 0x29;
	bootSector->bsVolumeID = 0x12345678;

	//
	// Set Label to "RamDisk    "
	// NOTE: Fill all 11 characters, eg. sizeof(bootSector->bsLabel);
	//
	bootSector->bsLabel[0] = 'R';
	bootSector->bsLabel[1] = 'a';
	bootSector->bsLabel[2] = 'm';
	bootSector->bsLabel[3] = 'D';
	bootSector->bsLabel[4] = 'i';
	bootSector->bsLabel[5] = 's';
	bootSector->bsLabel[6] = 'k';
	bootSector->bsLabel[7] = ' ';
	bootSector->bsLabel[8] = ' ';
	bootSector->bsLabel[9] = ' ';
	bootSector->bsLabel[10] = ' ';

	//
	// Set FileSystemType to "FAT1?   "
	// NOTE: Fill all 8 characters, eg. sizeof(bootSector->bsFileSystemType);
	//
	bootSector->bsFileSystemType[0] = 'F';
	bootSector->bsFileSystemType[1] = 'A';
	bootSector->bsFileSystemType[2] = 'T';
	bootSector->bsFileSystemType[3] = '1';
	bootSector->bsFileSystemType[4] = '?';
	bootSector->bsFileSystemType[5] = ' ';
	bootSector->bsFileSystemType[6] = ' ';
	bootSector->bsFileSystemType[7] = ' ';

	bootSector->bsFileSystemType[4] = (fatType == 16) ? '6' : '2';

	bootSector->bsSig2[0] = 0x55;
	bootSector->bsSig2[1] = 0xAA;

	//
	// The FAT is located immediately following the boot sector.
	//

	firstFatSector = (PUCHAR)(bootSector + 1);
	firstFatSector[0] = (UCHAR)devExt->DiskGeometry.MediaType;
	firstFatSector[1] = 0xFF;
	firstFatSector[2] = 0xFF;

	if (fatType == 16) {
		firstFatSector[3] = 0xFF;
	}

	//
	// The Root Directory follows the FAT
	//
	rootDir = (PDIR_ENTRY)(bootSector + 1 + fatSectorCnt);

	//
	// Set device name to "MS-RAMDR"
	// NOTE: Fill all 8 characters, eg. sizeof(rootDir->deName);
	//
	rootDir->deName[0] = 'M';
	rootDir->deName[1] = 'S';
	rootDir->deName[2] = '-';
	rootDir->deName[3] = 'R';
	rootDir->deName[4] = 'A';
	rootDir->deName[5] = 'M';
	rootDir->deName[6] = 'D';
	rootDir->deName[7] = 'R';

	//
	// Set device extension name to "IVE"
	// NOTE: Fill all 3 characters, eg. sizeof(rootDir->deExtension);
	//
	rootDir->deExtension[0] = 'I';
	rootDir->deExtension[1] = 'V';
	rootDir->deExtension[2] = 'E';

	rootDir->deAttributes = DIR_ATTR_VOLUME;

	return STATUS_SUCCESS;
}

VOID 
RamDiskQueryDiskRegParameters(
	PWSTR RegistryPath, 
	PDISK_INFO DiskRegInfo
)
/*++

Routine Description:

This routine is called from the DriverEntry to get the debug
parameters from the registry. If the registry query fails, then
default values are used.

Arguments:

RegistryPath    - Points the service path to get the registry parameters

Return Value:

None

--*/

{
	RTL_QUERY_REGISTRY_TABLE rtlQueryRegTbl[5 + 1];  // Need 1 for NULL
	NTSTATUS                 Status;
	DISK_INFO                defDiskRegInfo;

	PAGED_CODE();

	ASSERT(RegistryPath != NULL);

	// Set the default values

	defDiskRegInfo.DiskSize = DEFAULT_DISK_SIZE;
	defDiskRegInfo.RootDirEntries = DEFAULT_ROOT_DIR_ENTRIES;
	defDiskRegInfo.SectorsPerCluster = DEFAULT_SECTORS_PER_CLUSTER;

	RtlInitUnicodeString(&defDiskRegInfo.DriveLetter, DEFAULT_DRIVE_LETTER);

	RtlZeroMemory(rtlQueryRegTbl, sizeof(rtlQueryRegTbl));

	//
	// Setup the query table
	//

	rtlQueryRegTbl[0].Flags = RTL_QUERY_REGISTRY_SUBKEY;
	rtlQueryRegTbl[0].Name = L"Parameters";
	rtlQueryRegTbl[0].EntryContext = NULL;
	rtlQueryRegTbl[0].DefaultType = (ULONG_PTR)NULL;
	rtlQueryRegTbl[0].DefaultData = NULL;
	rtlQueryRegTbl[0].DefaultLength = (ULONG_PTR)NULL;

	//
	// Disk paramters
	//

	rtlQueryRegTbl[1].Flags = RTL_QUERY_REGISTRY_DIRECT;
	rtlQueryRegTbl[1].Name = L"DiskSize";
	rtlQueryRegTbl[1].EntryContext = &DiskRegInfo->DiskSize;
	rtlQueryRegTbl[1].DefaultType = REG_DWORD;
	rtlQueryRegTbl[1].DefaultData = &defDiskRegInfo.DiskSize;
	rtlQueryRegTbl[1].DefaultLength = sizeof(ULONG);

	rtlQueryRegTbl[2].Flags = RTL_QUERY_REGISTRY_DIRECT;
	rtlQueryRegTbl[2].Name = L"RootDirEntries";
	rtlQueryRegTbl[2].EntryContext = &DiskRegInfo->RootDirEntries;
	rtlQueryRegTbl[2].DefaultType = REG_DWORD;
	rtlQueryRegTbl[2].DefaultData = &defDiskRegInfo.RootDirEntries;
	rtlQueryRegTbl[2].DefaultLength = sizeof(ULONG);

	rtlQueryRegTbl[3].Flags = RTL_QUERY_REGISTRY_DIRECT;
	rtlQueryRegTbl[3].Name = L"SectorsPerCluster";
	rtlQueryRegTbl[3].EntryContext = &DiskRegInfo->SectorsPerCluster;
	rtlQueryRegTbl[3].DefaultType = REG_DWORD;
	rtlQueryRegTbl[3].DefaultData = &defDiskRegInfo.SectorsPerCluster;
	rtlQueryRegTbl[3].DefaultLength = sizeof(ULONG);

	rtlQueryRegTbl[4].Flags = RTL_QUERY_REGISTRY_DIRECT;
	rtlQueryRegTbl[4].Name = L"DriveLetter";
	rtlQueryRegTbl[4].EntryContext = &DiskRegInfo->DriveLetter;
	rtlQueryRegTbl[4].DefaultType = REG_SZ;
	rtlQueryRegTbl[4].DefaultData = defDiskRegInfo.DriveLetter.Buffer;
	rtlQueryRegTbl[4].DefaultLength = 0;


	Status = RtlQueryRegistryValues(
		RTL_REGISTRY_ABSOLUTE | RTL_REGISTRY_OPTIONAL,
		RegistryPath,
		rtlQueryRegTbl,
		NULL,
		NULL
	);

	if (NT_SUCCESS(Status) == FALSE) {

		DiskRegInfo->DiskSize = defDiskRegInfo.DiskSize;
		DiskRegInfo->RootDirEntries = defDiskRegInfo.RootDirEntries;
		DiskRegInfo->SectorsPerCluster = defDiskRegInfo.SectorsPerCluster;
		RtlCopyUnicodeString(&DiskRegInfo->DriveLetter, &defDiskRegInfo.DriveLetter);
	}

	KdPrint(("DiskSize          = 0x%lx\n", DiskRegInfo->DiskSize));
	KdPrint(("RootDirEntries    = 0x%lx\n", DiskRegInfo->RootDirEntries));
	KdPrint(("SectorsPerCluster = 0x%lx\n", DiskRegInfo->SectorsPerCluster));
	KdPrint(("DriveLetter       = %wZ\n", &(DiskRegInfo->DriveLetter)));
	return;
}
