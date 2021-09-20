#ifndef _BIOS_Parameter_Block
#define _BIOS_Parameter_Block

#include "Endianness.h"

typedef struct BIOS_PARAMETER_BLOCK
{								//Offset	#- Byte
	int  BPB_BytesPerSector;	//	B		2
	int  BPB_SectorPerCluster;	//	D		1
	int  BPB_RsvdSectorCnt;		//	E		2
	int  BPB_NumFATs;			//	10		1
	int  BPB_RootEntCnt;		//	11		2
	int  BPB_RootDirSectors;
	int  BPB_FATSz32;			//	24		4
	int	 BPB_TotSec32;			//	20		4
	int  BPB_FAT1;
	int  BPB_FirstRDETSector;
	int  BPB_FirstDataSector;
	int	 BPB_DataRegion;
	int  BPB_TotClusters;
	int  BPB_RootClus;			//	2C		4
	char BPB_FAT_Type[8];		//	52		8
}_BPB;


_BPB retrieve_BPB_info(unsigned char* buffer)
{
	_BPB info;
	info.BPB_BytesPerSector = littleEndian_2_byte_Conversion(buffer[11], buffer[12]);

	info.BPB_SectorPerCluster = littleEndian_1_byte_Conversion(buffer[13]);

	info.BPB_RsvdSectorCnt = littleEndian_2_byte_Conversion(buffer[14], buffer[15]);

	info.BPB_NumFATs = littleEndian_1_byte_Conversion(buffer[16]);
	
	info.BPB_RootEntCnt = littleEndian_2_byte_Conversion(buffer[17], buffer[18]); //Entry

	info.BPB_RootDirSectors = info.BPB_RootEntCnt * 32 / info.BPB_BytesPerSector; //SRDET

	info.BPB_FATSz32 = littleEndian_4_byte_Conversion(buffer[36], buffer[37], buffer[38], buffer[39]);

	info.BPB_TotSec32 = littleEndian_4_byte_Conversion(buffer[32], buffer[33], buffer[34], buffer[35]);

	info.BPB_FAT1 = info.BPB_RsvdSectorCnt;

	info.BPB_FirstRDETSector = info.BPB_RsvdSectorCnt + info.BPB_NumFATs * info.BPB_FATSz32;
	
	info.BPB_FirstDataSector = info.BPB_RsvdSectorCnt + (info.BPB_NumFATs * info.BPB_FATSz32) + info.BPB_RootDirSectors;

	info.BPB_DataRegion = info.BPB_TotSec32 - (info.BPB_RsvdSectorCnt + (info.BPB_NumFATs * info.BPB_FATSz32) + info.BPB_RootDirSectors);

	info.BPB_TotClusters = info.BPB_DataRegion / info.BPB_SectorPerCluster;

	info.BPB_RootClus = littleEndian_4_byte_Conversion(buffer[44], buffer[45], buffer[46], buffer[47]);

	int index = 0;
	for (int hex = 82; hex <= 89; hex++)
		info.BPB_FAT_Type[index++] = buffer[hex];

	return info;
}

void print_BPB_Info(_BPB* other)
{
	printf("BPB_Bytes per Sector is		0x%X, %d Byte\n", other->BPB_BytesPerSector, other->BPB_BytesPerSector);
	printf("BPB_Sector per Cluster is	0x%X, %d sector\n", other->BPB_SectorPerCluster, other->BPB_SectorPerCluster);
	printf("BPB_Reserved Sector Count is	0x%X, %d sector\n", other->BPB_RsvdSectorCnt, other->BPB_RsvdSectorCnt);
	printf("BPB_Numbers of FATs is		0x%X, %d \n", other->BPB_NumFATs, other->BPB_NumFATs);
	printf("BPB_Entry of RDET is		0x%X, %d entry\n", other->BPB_RootEntCnt, other->BPB_RootEntCnt);
	printf("BPB_FAT Size is			0x%X, %d sector\n", other->BPB_FATSz32, other->BPB_FATSz32);
	printf("BPB_Total Size is		0x%X, %d sector\n", other->BPB_TotSec32, other->BPB_TotSec32);
	printf("BPB_FAT Type is ");
	for (int i = 0; i < 8; i++)
		printf("%c", other->BPB_FAT_Type[i]);
	printf("\n");
	printf("BPB_Starting sector of FAT1 is %d\n", other->BPB_FAT1);
	printf("BPB_Starting sector of RDET is %d\n", other->BPB_FirstRDETSector);
	printf("BPB_Starting sector of DATA is %d\n", other->BPB_FirstDataSector);
	printf("BPB_Numbers of sectors in data region is %d sector\n", other->BPB_DataRegion);
	printf("BPB_Total Clusters is %d\n", other->BPB_TotClusters);
	printf("BPB_Starting cluster of Root Cluster is 0x%X, %d\n", other->BPB_RootClus, other->BPB_RootClus);
}

#endif