#include <iostream>
#include <windows.h>
#include <vector>
#include <map>
using namespace std;

vector <vector<int>> indexTree (100, vector<int> (1,0));
map <int, string> nameTree;
void ReadSect(const char*  drive, BYTE*& DATA, unsigned int _nsect, unsigned int number_sector);
void print_sector(BYTE* sector);
int64_t toNumber(BYTE* DATA, int offset, int number);
string toString(BYTE* DATA, int offset, int number);
string toBinary(int n);
int read_inforEntry(BYTE* Entry, int start);
int read_NameEntry(BYTE* Entry, int start, int ID, bool& pa);
void read_DataEntry(BYTE* Entry, int start);
void readBPB(BYTE* sector, const char * disk);

int main()
{
    BYTE* sector;
    char* path = new char[6];
    path[0] = '\\';
    path[1] = '\\';
    path[2] = '.';
    path[3] = '\\';
    path[4] = 'C';
    path[5] = ':';
    path[6] = (char)0;

    wcout << L"Drive letter of USB: ";
    char str;
    cin >> str;
    path[4] = str;
    ReadSect(path,sector, 0, 1);
    print_sector(sector);
    readBPB(sector, path);
	return 0;
}
int64_t toNumber(BYTE* DATA, int offset, int number)  //lay bao nhieu byte tuwf vi tri 0x...
{
    int64_t k = 0;
    memcpy(&k, DATA + offset, number);
    return k;
}
string toString(BYTE* DATA, int offset, int number)
{
    char* tmp = new char[number + 1];
    memcpy(tmp, DATA + offset, number);
    string s = "";
    for (int i = 0; i < number; i++)
        if (tmp[i] != 0x00)
            s += tmp[i];

    return s;
}
string toBinary(int n);
int read_inforEntry(BYTE* Entry, int start)
{
    int status = toNumber(Entry, start + 56, 4);
    string bin = toBinary(status);
    for (int i = bin.length() - 1; i >= 0; i--)
    {
        int n = bin.length();
        if (bin[i] == '1')
        {
            if (i == n - 2)
            {
                return -1;
            }
            if (i == n - 3)
            {
                return -1;
            }
        }
    }
    cout << "- attribute $STANDARD_INFORMATION" << endl;
    int size = toNumber(Entry, start + 4, 4);
    cout << "\t+ Length of attribute (include header): " << size << endl;
    cout << "\t+ Status Attribute of File: " << bin << endl;
    for (int i = bin.length() - 1; i >= 0; i--)
    {
        int n = bin.length();
        if (bin[i] == '1')
        {
            if (i == n - 1)
                cout << "\t+ Read Only" << endl;
            if (i == n - 2)
                cout << "\t+ Hidden" << endl;
            if (i == n - 3)
                cout << "\t \t+ File System" << endl;
            if (i == n - 4)
                cout << "\t \t+ Vollabel" << endl;
            if (i == n - 5)
                cout << "\t \t+ Directory" << endl;
            if (i == n - 6)
                cout << "\t \t+ Archive" << endl;
        }
    }


    return size;
}
int read_NameEntry(BYTE* Entry, int start, int ID, bool& pa)
{
    cout << "- attribute $FILE NAME" << endl;
    int size = toNumber(Entry, start + 4, 4);
    cout << "\t+ Length of attribute (include header): " << size << endl;
    int parent_file = toNumber(Entry, start + 24, 6);
    cout << "\t+ Parent file: " << parent_file << endl;
    for (int i = 0;i < indexTree.size(); i++)
    {
        if (indexTree[i][0] == parent_file)
        {
            indexTree[i].push_back(ID);
            pa = 1;
        }
    }
    int lengthName = toNumber(Entry, start + 88, 1);
    cout << "\t+ Lenght of name file: " << lengthName << endl;
    string name = toString(Entry, start + 90, lengthName * 2);
    cout << "\t+ Name of file: " << name << endl;
    nameTree[ID] = name;
    return size;
}
void read_DataEntry(BYTE* Entry, int start)
{
    cout << "- attribute $DATA" << endl;
    int size = toNumber(Entry, start + 4, 4);
    cout << "\t+ Length of attribute (include header): " << size << endl;
    int sizeFile = toNumber(Entry, start + 40, 8);
    cout << "\t+ Size of file: " << sizeFile << endl;
    int sizeReal = toNumber(Entry, start + 48, 8);
    cout << "\t+ Real Size of file: " << sizeReal << endl;
}
void readBPB( BYTE* sector, const char* disk)
{
    unsigned int size_sector = toNumber(sector, 0x0B, 2);
    cout << "Size of sector: " << size_sector << endl;
    unsigned int cluster = toNumber(sector, 0x0D, 1);
    cout << "Sector/ Cluster: " << cluster << endl;
    unsigned int track = toNumber(sector, 0x018, 2);
    cout << "Sector/ track: " << track << endl;
    unsigned int side = toNumber(sector, 0x01A, 2);
    cout << "Number side: " << side << endl;
    unsigned int totalSector = toNumber(sector, 0x028, 8);
    cout << "Total sector on disk: " << totalSector << endl;
    unsigned int MFTStart = toNumber(sector, 0x030, 8) * 8;
    cout << "Cluster start of MFT is: " << MFTStart << endl;

    BYTE* MFT;

    ReadSect(disk, MFT, MFTStart, 1);
    //print_sector(MFT);

    // Read $MFT Entry
    // INFORMATION
    int Entry_infor = toNumber(MFT, 0x014, 2);
    cout << "Attribute INFOR Entry start at: " << Entry_infor << endl;
    int len_infor = toNumber(MFT, Entry_infor + 4, 4);
    cout << "Length of INFOR Entry: " << len_infor << endl;
    // FILE NAME
    int Entry_Name = Entry_infor + len_infor;
    cout << "Attribute FILE NAME Entry start at: " << Entry_Name << endl;
    int len_name = toNumber(MFT, Entry_Name + 4, 4);
    cout << "Length of FILE NAME Entry: " << len_name << endl;
    // DATA
    int Entry_Data = Entry_Name + len_name;
    cout << "Attribute DATA Entry start at: " << Entry_Data << endl;
    int len_data = toNumber(MFT, Entry_Data + 4, 4);
    cout << "Length of DATA Entry: " << len_data << endl;
    // main DATA

    int len_MFT = MFTStart + (toNumber(MFT, Entry_Data + 24, 8) + 1) * 8;
    cout << "Number sector in MFT is: " << len_MFT - MFTStart << endl;



    for (int i = 2; i < len_MFT - MFTStart; i += 2)
    {
        int currentSector = MFTStart + i;
        BYTE* currentEntry;
        ReadSect(disk, currentEntry, currentSector, 1);

        if (toString(currentEntry, 0x00, 4) == "FILE")
        {

            bool pa = 0;

            int startInfor = toNumber(currentEntry, 0x014, 2);
            int sizeInfor = read_inforEntry(currentEntry, startInfor);
            if (sizeInfor == -1)
                continue;
            cout << endl;
            int ID = toNumber(currentEntry, 0x02C, 4);
            cout << "ID File: " << ID << endl;
            int startName = sizeInfor + 56;
            int sizeName = read_NameEntry(currentEntry, startName, ID, pa);
            int startData = startName + sizeName;
            read_DataEntry(currentEntry, startData);
            if (pa == 0) indexTree[i][0] = ID;
            cout << endl;
        }
        delete currentEntry;
    }
    cout << "\t \t \t \t CAY THU MUC" << endl;
    for (int i = 0; i < indexTree.size(); i++)
    {
        if (indexTree[i][0] != 0)
        {
            cout << "-" << nameTree[indexTree[i][0]] << endl;
            for (int j = 1; j < indexTree[i].size(); j++)
            {
                cout << "    +" << nameTree[indexTree[i][j]] << endl;
            }
        }
    }
}
void ReadSect
(const char* _dsk,    // disk to access
    BYTE*& DATA,    // buffer where sector will be stored
    unsigned int _nsect,  // sector number, starting with 0
    unsigned int number_sector
)
{
    DATA = new BYTE[number_sector * 512];
    memset(DATA, 0, 512 * number_sector);
    DWORD dwBytesRead(0);

    wstring drive = L"";
    int n = strlen(_dsk);
    for (int i = 0; i < n; i++)
        drive += wchar_t(_dsk[i]);

    HANDLE hFloppy = NULL;
    hFloppy = CreateFile(drive.c_str(),    // Floppy drive to open
        GENERIC_READ,              // Access mode
        FILE_SHARE_READ | FILE_SHARE_WRITE,           // Share Mode
        NULL,                      // Security Descriptor
        OPEN_EXISTING,             // How to create
        0,                         // File attributes
        NULL);                     // Handle to template

    if (hFloppy != NULL)
    {
        LARGE_INTEGER li;
        li.QuadPart = _nsect * 512;
        SetFilePointerEx(hFloppy, li, 0, FILE_BEGIN);
        // Read the boot sector
        if (!ReadFile(hFloppy, DATA, 512 * number_sector, &dwBytesRead, NULL))
        {
            printf("Error in reading floppy disk\n");
        }

        CloseHandle(hFloppy);
        // Close the handle
    }
}
void print_sector(BYTE *sector)
{
    int count = 0;
    int num = 0;

    cout << "         0  1  2  3  4  5  6  7    8  9  A  B  C  D  E  F" << endl;
    
    cout << "0x0" << num << "0  ";
    bool flag = 0;
    for (int i = 0; i < 512; i++)
    {
        count++;
        if (i % 8 == 0)
            cout << "  ";
        printf("%02X ", sector[i]);
        if (i == 255)
        {
            flag = 1;
            num = 0;
        }
        
        if (i == 511) break;
        if (count == 16)
        {
            int index = i;

            cout << endl;
            
            if (flag == 0)
            {
                num++;
                if (num < 10)
                    cout << "0x0" << num << "0  ";
                else
                {
                    char hex = char(num - 10 + 'A');
                    cout << "0x0" << hex << "0  ";
                }
               
            }
            else
            {
                if (num < 10)
                    cout << "0x1" << num << "0  ";
                else
                {
                    char hex = char(num - 10 + 'A');
                    cout << "0x1" << hex << "0  ";
                }
                num++;
            }
            
            count = 0;
        }
    }
    cout << endl;
}
string toBinary(int n)
{
    string r;
    while (n != 0) { r = (n % 2 == 0 ? "0" : "1") + r; n /= 2; }
    return r;
}