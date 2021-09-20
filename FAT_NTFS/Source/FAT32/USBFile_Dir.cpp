#define _CRT_SECURE_NO_WARNINGS

#include "USBFile_Dir.h"

FILE_Attr get_File_attr(unsigned char);
char* rename_File(char*, unsigned short, unsigned short);

FILE_info File_Initializer(BYTE* buffer, unsigned int position, unsigned int file_addr, unsigned int parent_file_addr)
{
	FILE_info file_info;
	file_info.FILE_addr = file_addr;
	file_info.parent_FILE_addr = parent_file_addr;
	file_info.FILE_Attr = get_File_attr(buffer[position + 11]);
	for (int i = 0; i <= 10; i++)
		file_info.FILE_Name[i] = buffer[position + i];
	file_info.FILE_Name[11] = '\0';
	file_info.FILE_FstClusHi = littleEndian_2_byte_Conversion(buffer[position + 20], buffer[position + 21]);
	file_info.FILE_FstClusLo = littleEndian_2_byte_Conversion(buffer[position + 26], buffer[position + 27]);
	file_info.FILE_Size = littleEndian_4_byte_Conversion(buffer[position + 28], buffer[position + 29], buffer[position + 30], buffer[position + 31]);
	file_info.FstCluster = littleEndian_4_byte_Conversion(buffer[position + 26], buffer[position + 27], buffer[position + 20], buffer[position + 21]);

	char* modified_name = rename_File(file_info.FILE_Name, file_info.FILE_Attr.ATTR_DIRECTORY, file_info.FILE_Attr.ATTR_VOLUME_LABEL);
	strcpy(file_info.FILE_Name, modified_name);

	if (file_info.FILE_Attr.ATTR_VOLUME_LABEL)
		strcpy(volumeLabel_Name, file_info.FILE_Name);

	return file_info;
}

FILE_Attr get_File_attr(unsigned char position_0B)
{
	FILE_Attr file_attr;

	file_attr.ATTR_READ_ONLY = position_0B & 1;
	file_attr.ATTR_HIDDEN = (position_0B >> 1) & 1;
	file_attr.ATTR_SYSTEM = (position_0B >> 2) & 1;
	file_attr.ATTR_VOLUME_LABEL = (position_0B >> 3) & 1;
	file_attr.ATTR_DIRECTORY = (position_0B >> 4) & 1;
	file_attr.ATTR_ARCHIVE = (position_0B >> 5) & 1;

	return file_attr;
}

char* rename_File(char* oldName, unsigned short isDir, unsigned short isVolLabel)
{
	char* newName = oldName;

	int old_index = 0, new_index = 0;
	int isFirstSpace = (isDir || isVolLabel) ? 0 : 1;

	while (oldName[old_index] != '\0')
	{
		if (oldName[old_index] == ' ')
		{
			if (isFirstSpace)
			{
				newName[new_index] = '.';
				new_index++;
				isFirstSpace = 0;
			}
		}
		else
		{
			newName[new_index] = oldName[old_index];
			new_index++;
		}
		old_index++;
	}

	newName[new_index] = '\0';
	for (int i = new_index; i <= 50; i++)
		newName[i] = '\0';

	return newName;
}

bool _Terminated(unsigned char byte_1, unsigned char byte_2)
{
	if ((byte_1 == 0x00 && byte_2 == 0x00) || (byte_1 == 0xFF && byte_2 == 0xFF))
		return true;
	return false;
}

char* subEntry_Name(char* subEntry, int subEntryCnt)
{
	char* newName = NULL;
	char temp[100];
	int index = 0;
	for (int i = subEntryCnt - 1; i >= 0; i--)
	{
		int position = i * 32;
		for (int bytes = 1; bytes <= 10; bytes++)
		{
			if (_Terminated(subEntry[position + bytes], subEntry[position + bytes + 1]))
				break;
			temp[index++] = subEntry[position + bytes]; 
		}

		for (int bytes = 14; bytes <= 25; bytes++)
		{
			if (_Terminated(subEntry[position + bytes], subEntry[position + bytes + 1]))
				break;
			temp[index++] = subEntry[position + bytes];
		}

		for (int bytes = 28; bytes <= 31; bytes++)
		{
			if (_Terminated(subEntry[position + bytes], subEntry[position + bytes + 1]))
				break;
			temp[index++] = subEntry[position + bytes];
		}
	}
	temp[index] = '\0';

	int size = 0, i = 0;
	while (i < index)
	{
		if(temp[i++] != '\0')
			size++;
	}

	newName = new char[size + 1];
	memset(newName, 0, size + 1);
	int new_index = 0;
	for (i = 0; i < index; i++)
		if (temp[i] != '\0')
			newName[new_index++] = temp[i];

	return newName;
}

DIR_Files retrieve_files(BYTE* buffer, BYTE* FAT_buffer, unsigned int FirstCluster, short ATTR_VOLUME_LABEL, unsigned int SectorPerClus, unsigned int BytesPerSector, unsigned int FirstDataSector, unsigned int FAToffset)
{
	int mallocSize = 16;
	FILE_info* files = (FILE_info*)malloc(mallocSize * sizeof(FILE_info));

	int file_index = 0, DIR_Addr = 0, subEntry_Cnt = 0;
	char* subEntry = NULL;

	if (ATTR_VOLUME_LABEL)
		file_index = file_index + 32;

	int num_of_files = 0, position = 0;
	int subEntry_pos_Dectector = 0;
	char* modified_name = NULL;

	Clusters clusters = retrieve_allClusters(FAT_buffer, FirstCluster, FAToffset);

	for (int i = 0; i < clusters.num_of_clusters; i++)
	{
		DIR_Addr = (((clusters.clusterArr[i] - 2) * SectorPerClus) + FirstDataSector) * BytesPerSector;
		file_index = DIR_Addr;

		//512-Byte Table of entries 
		while ((file_index - DIR_Addr) < 512)
		{
			bool isSubEntry = false;
			//E5h – File occupying this entry has been deleted
			if (buffer[position] == 0xE5)
			{
				file_index += 32;
				position += 32;
				subEntry_pos_Dectector += 32;
				continue;
			}

			if (buffer[position] == 0x2E)
			{
				file_index += 32;
				position += 32;
				subEntry_pos_Dectector += 32;
				continue;
			}

			//00 – Empty entry
			if (buffer[position] == 0x00)
				break;

			//0F - Sub entry
			while (buffer[position + 11] == 0x0F)
			{
				subEntry_Cnt++;
				file_index += 32;
				position += 32;
				isSubEntry = true;
				continue;
			}

			//Handles sub entry
			if (isSubEntry)
			{
				int size = position - subEntry_pos_Dectector;
				subEntry = new char[size];
				memset(subEntry, 0, size);
				int temp = 0;
				for (int i = subEntry_pos_Dectector; i < position; i++)
					subEntry[temp++] = buffer[i];

				modified_name = subEntry_Name(subEntry, subEntry_Cnt);
			}

			if (num_of_files >= mallocSize)
			{
				mallocSize = mallocSize * 2;
				files = (FILE_info*)realloc(files, mallocSize * sizeof(FILE_info));
			}

			files[num_of_files] = File_Initializer(buffer, position, file_index, DIR_Addr);
			if (isSubEntry)
			{
				strcpy(files[num_of_files].FILE_Name, modified_name);
				subEntry_Cnt = 0;
				subEntry_pos_Dectector = position;
			}
			position += 32;
			file_index += 32;
			subEntry_pos_Dectector += 32;
			num_of_files++;
		}
	}

	DIR_Files dir_files;
	dir_files.files = files;
	dir_files.number_of_files = num_of_files;
	return dir_files;
}

DIR_Files retrieve_Dir_Files(BYTE* buffer, BYTE* FAT_buffer, FILE_info file_info, unsigned int SectorPerClus, unsigned int BytesPerSector, unsigned int FirstDataSector, unsigned int FAToffset)
{
	return retrieve_files(buffer, FAT_buffer, file_info.FstCluster, file_info.FILE_Attr.ATTR_VOLUME_LABEL, SectorPerClus, BytesPerSector, FirstDataSector, FAToffset);
}

DIR_Files retrieve_subDir_Files(BYTE* buffer, BYTE* FAT_buffer, FILE_info file_info, unsigned int SectorPerClus, unsigned int BytesPerSector, unsigned int FirstDataSector, unsigned int FAToffset)
{
	return retrieve_files(buffer, FAT_buffer, file_info.FstCluster, file_info.FILE_Attr.ATTR_VOLUME_LABEL, SectorPerClus, BytesPerSector, FirstDataSector, FAToffset);
}

Clusters retrieve_allClusters(BYTE* FAT_buffer, unsigned int firstCluster, unsigned int FAToffset)
{
	int* allClusters = (int*)malloc(8 * sizeof(int));
	int i = 0, mallocSize = 8;

	int cluster = firstCluster;
	if (firstCluster == 0)
		cluster += 2;

	int offset_bytes = 0;
	allClusters[i] = cluster;
	i++;

	while (1)
	{
		offset_bytes = cluster * 4; //FAT32 - 4 bytes for each cluster
		int position = offset_bytes;
		cluster = littleEndian_4_byte_Conversion(FAT_buffer[position], FAT_buffer[position + 1], FAT_buffer[position + 2], FAT_buffer[position + 3]) & 0X0fffffff;
		//EOF
		if (cluster == 0xffffff8 || cluster == 0xfffffff)
			break;
		allClusters[i] = cluster;
		if (i == (mallocSize - 1)) {
			mallocSize = mallocSize * 2;
			allClusters = (int*)realloc(allClusters, mallocSize * sizeof(int));
		}

		i++;
	}

	Clusters clusters;

	clusters.clusterArr = allClusters;
	clusters.num_of_clusters = i;

	return clusters;
}