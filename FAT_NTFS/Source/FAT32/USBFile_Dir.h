#ifndef _USBFile_Directory
#define _USBFile_Directory

#include "Endianness.h"
#include <windows.h>
#include <stdio.h>

typedef struct FILE_Attr
{
	short ATTR_READ_ONLY;
	short ATTR_HIDDEN;
	short ATTR_SYSTEM;
	short ATTR_VOLUME_LABEL;
	short ATTR_DIRECTORY;
	short ATTR_ARCHIVE;
}FILE_Attr;

typedef struct FILE_info
{
	int FILE_addr;
	int parent_FILE_addr;
	char FILE_Name[50];
	int FILE_FstClusLo;
	int FILE_FstClusHi;
	int FstCluster;
	int FILE_Size;
	FILE_Attr FILE_Attr;

}FILE_info;

typedef struct DIR_Files
{
	FILE_info* files;
	int number_of_files;
}DIR_Files;

typedef struct Clusters
{
	int* clusterArr;
	int num_of_clusters;
};

FILE_info File_Initializer(BYTE*, unsigned int, unsigned int, unsigned int);
DIR_Files retrieve_Dir_Files(BYTE*, BYTE*, FILE_info, unsigned int, unsigned int, unsigned int, unsigned int);
DIR_Files retrieve_subDir_Files(BYTE*, BYTE*, FILE_info, unsigned int, unsigned int, unsigned int, unsigned int);
Clusters retrieve_allClusters(BYTE*, unsigned int, unsigned int);

static char volumeLabel_Name[12];

#endif 