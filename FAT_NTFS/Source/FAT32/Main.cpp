#include <stdio.h>
#include <conio.h>
#include <ctype.h>
#include <vector>
#include <string>
#include <Windows.h>

#include "BIOS_Parameter_Block.h"
#include "USBFile_Dir.h"

void printTable(BYTE*);
BYTE* readFile_Sector(HANDLE, DWORD&, unsigned int);
void printDir(std::vector<FILE_info>, std::vector< std::vector<FILE_info >>, char);

int main()
{
	//USB maximum capacity
	const int MAX_USB_GB = 16;

	//Get the drive letter from the user
	printf("Drive letter of USB: ");
	char path[] = "//./?:";
	scanf_s("%c", path + 4, 1);
	char driveLetter = path[4];
	//char path[] = "//./F:";

	//Open the volume
	HANDLE hUSB = CreateFileA(
		path,	//Drive to open
		GENERIC_READ,		//Access mode
		FILE_SHARE_READ,	//Share mode
		NULL,				//Security descriptor
		OPEN_EXISTING,		//How to create
		0,					//File attributes
		NULL);				//Handle to template

	//If handle fails
	if (hUSB == INVALID_HANDLE_VALUE)
	{
		printf("WARNING: Check drive letter!.");
		_getch();
		return -1;
	}

	//Buffer to store the boot sector
	BYTE* sector = new BYTE[512];
	BYTE buffer[512];
	memcpy(buffer, sector, 512);
	memset(buffer, 0, 512);

	DWORD bytesRead = 0;
	_BPB MBR_Info;

	//Read boot sector to sector
	sector = readFile_Sector(hUSB, bytesRead, 0);
	//Copy sector to buffer
	memcpy(buffer, sector, 512);
	printf("Boot Sector");
	printTable(buffer);

	//BPB Information
	MBR_Info = retrieve_BPB_info(buffer);
	print_BPB_Info(&MBR_Info);

	int FirstSectorofCluster = ((MBR_Info.BPB_RootClus - 2) * MBR_Info.BPB_SectorPerCluster) + MBR_Info.BPB_FirstDataSector;
	int Root_Dir = FirstSectorofCluster * MBR_Info.BPB_BytesPerSector;
	printf("First sector of cluster is %d\n", FirstSectorofCluster);
	sector = readFile_Sector(hUSB, bytesRead, Root_Dir);
	memcpy(buffer, sector, 512);
	//printTable(buffer);

	BYTE FAT_buffer[512];
	int FATOffset = MBR_Info.BPB_BytesPerSector * MBR_Info.BPB_RsvdSectorCnt;
	sector = readFile_Sector(hUSB, bytesRead, FATOffset);
	memcpy(FAT_buffer, sector, 512);
	//printTable(FAT_buffer);

	FILE_info current_file_info = File_Initializer(buffer, 0, Root_Dir, 0);
	DIR_Files files = retrieve_Dir_Files(buffer, FAT_buffer, current_file_info, MBR_Info.BPB_SectorPerCluster, MBR_Info.BPB_BytesPerSector, MBR_Info.BPB_FirstDataSector, FATOffset);

	std::vector<FILE_info> tree;
	std::vector<std::vector<FILE_info>> subFiles_Name;
	subFiles_Name.resize(files.number_of_files);

	for (int i = 0; i < files.number_of_files; i++)
	{
		tree.push_back(files.files[i]);
		std::vector<FILE_info> file_name;
		DIR_Files subFiles;
		if (files.files[i].FILE_Attr.ATTR_DIRECTORY)
		{
			int firstClusterofSUBDIR = files.files[i].FstCluster;
			int subFileSector = 0;
			Clusters clusters = retrieve_allClusters(FAT_buffer, firstClusterofSUBDIR, FATOffset);
			for (int index = 0; index < clusters.num_of_clusters; index++)
			{
				subFileSector = ((clusters.clusterArr[index] - 2) * MBR_Info.BPB_SectorPerCluster) + MBR_Info.BPB_FirstDataSector;
				BYTE subFile_buffer[512];
				sector = readFile_Sector(hUSB, bytesRead, subFileSector * MBR_Info.BPB_BytesPerSector);
				memcpy(subFile_buffer, sector, 512);
				//printTable(subFile_buffer);

				subFiles = retrieve_subDir_Files(subFile_buffer, FAT_buffer, files.files[i], MBR_Info.BPB_SectorPerCluster, MBR_Info.BPB_BytesPerSector, MBR_Info.BPB_FirstDataSector, FATOffset);
				for (int j = 0; j < subFiles.number_of_files; j++)
					file_name.push_back(subFiles.files[j]);
			}
		}
		subFiles_Name[i] = file_name;
	}

	printDir(tree, subFiles_Name, driveLetter);

	CloseHandle(hUSB);

	_getch();
	return 0;
}

void printTable(BYTE* sector)
{
	printf("\n");
	for (int i = 0; i <= 15; i++)
		printf("%X  ", i);

	for (int i = 0; i < 512; i++)
	{
		if (i % 16 == 0)
			printf("\n");
		//printf("%c ", isascii(sector[i]) ? sector[i] : '.');
		printf("%02X ", sector[i]);
	}
	printf("\n\n");
}

BYTE* readFile_Sector(HANDLE hUSB, DWORD& bytesRead, unsigned int sector_position)
{
	BYTE sector[512];
	//Set a Point to Read
	SetFilePointer(hUSB, sector_position, NULL, FILE_BEGIN);
	if (!ReadFile(hUSB, sector, sizeof(sector), &bytesRead, NULL))
	{
		printf("ReadFile: %u\n", GetLastError());
		return NULL;
	}
	return sector;
}

void printDir(std::vector<FILE_info> tree, std::vector< std::vector<FILE_info>> subFiles_Name, char driveLetter)
{
	printf("\n");
	printf("Directory of %c:/\n", driveLetter);
	for (int i = 0; i < tree.size(); i++)
	{
		if (tree[i].FILE_Attr.ATTR_SYSTEM || tree[i].FILE_Attr.ATTR_HIDDEN)
			continue;
		if (tree[i].FILE_Attr.ATTR_VOLUME_LABEL)
			printf("Volume in drive %c is %s\n\n", driveLetter, tree[i].FILE_Name);
		if (tree[i].FILE_Attr.ATTR_DIRECTORY)
		{
			printf("%s\n", tree[i].FILE_Name);
			for (int j = 0; j < subFiles_Name[i].size(); j++)
				printf("  |__%s\n", subFiles_Name[i][j].FILE_Name);
		}
		if (tree[i].FILE_Attr.ATTR_ARCHIVE)
			printf("%s\n", tree[i].FILE_Name);
	}

	printf("\nFile Name\t\tType\t\tSize (bytes)\tFirst Cluster\n\n");
	int typePosition = 23;
	for (int i = 0; i < tree.size(); i++)
	{
		//Skip system file
		if (tree[i].FILE_Attr.ATTR_SYSTEM || tree[i].FILE_Attr.ATTR_HIDDEN)
			continue;
		//Print File Name
		printf("%s", tree[i].FILE_Name);
		int lastChar = strlen(tree[i].FILE_Name) - 1;
		for (int i = lastChar; i < typePosition; i++)
			printf(" ");
		//Print File type
		if (tree[i].FILE_Attr.ATTR_VOLUME_LABEL) printf("<VOLUME LABEL>");
		if (tree[i].FILE_Attr.ATTR_DIRECTORY) printf("<DIR>\t\t");
		if (tree[i].FILE_Attr.ATTR_ARCHIVE) printf("<FILE>\t\t");
		//Print File size
		if (tree[i].FILE_Attr.ATTR_VOLUME_LABEL)
			printf("\t\t\t");
		else if (tree[i].FILE_Attr.ATTR_DIRECTORY)
			printf("\t\t");
		else printf("%d\t\t", tree[i].FILE_Size);
		printf("%d\n", tree[i].FstCluster);
	}

	for (int i = 0; i < subFiles_Name.size(); i++)
	{
		if (subFiles_Name[i].size() == 0)
			continue;
		for (int j = 0; j < subFiles_Name[i].size(); j++)
		{
			printf("%s", subFiles_Name[i][j].FILE_Name);
			int lastChar = strlen(subFiles_Name[i][j].FILE_Name) - 1;
			for (int i = lastChar; i < typePosition; i++)
				printf(" ");
			if (subFiles_Name[i][j].FILE_Attr.ATTR_DIRECTORY) printf("<DIR>\t\t");
			if (subFiles_Name[i][j].FILE_Attr.ATTR_ARCHIVE) printf("<FILE>\t\t");
			if (subFiles_Name[i][j].FILE_Attr.ATTR_DIRECTORY)
				printf("\t\t");
			else printf("%d\t\t", subFiles_Name[i][j].FILE_Size);
			printf("%d\n", subFiles_Name[i][j].FstCluster);
		}
	}
}