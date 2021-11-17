#include <iostream>
#include <unistd.h>
#include <cmath>
#include <cstring>

using namespace std;

#define DATA_SIZE 512 * 1024 * 1024
#define BLOCK_SIZE 4096
#define NO_OF_DATA_BLOCKS 131072
#define NO_OF_INODES 78644
#define NO_OF_SB_BLOCKS 52
#define NO_OF_TOTAL_BLOCKS 209767

string diskPrefix = "disks/d_";
string diskExt = ".txt";
string diskpath;


struct SuperBlock {
	bool free_inodes[NO_OF_INODES];
	bool free_data_blocks[NO_OF_DATA_BLOCKS];  
};

struct fileInodeMap {
	char filename[10];
	int inodeIndex;
};

struct Inode {
	char filename[10];
	char filepath[100];

	int bPointer[10];
};

long long int total_disk_size = 1 * sizeof(struct SuperBlock) + 
								NO_OF_INODES * sizeof(struct Inode) + 
								NO_OF_DATA_BLOCKS * BLOCK_SIZE;

long long int inodeStartingIndex = sizeof(struct SuperBlock) + NO_OF_INODES * sizeof(struct fileInodeMap);
long long int dataBlockStartingIndex = inodeStartingIndex + NO_OF_DATA_BLOCKS * BLOCK_SIZE;

struct fileInodeMap globalFileInodeMap[NO_OF_INODES];
struct SuperBlock globalSuperBlock;

void initializeDisk(string diskpath) {
	/* *** Writing SUPER BLOCK */
	FILE*  diskDescriptor = fopen(diskpath.c_str(), "wb");
	//fseek(diskDescriptor, 0, SEEK_SET);

	cout << "init sb object" << endl;
	struct SuperBlock sb;
	long int i;
	cout << "filling free blocks values" << endl;
	for(i = 0; i < NO_OF_INODES; i++) {
		sb.free_inodes[i] = true;
	}

	cout << "written free inodes" << endl;
	for(i = 0; i < NO_OF_DATA_BLOCKS; i++) {
		sb.free_data_blocks[i] = true;
	}

	cout << "written free data blocks" << endl;
	fseek(diskDescriptor, 0, SEEK_SET);
	long long int sb_size = sizeof(sb);
	char sbBuff[sb_size];
    	memset(sbBuff, 0, sb_size);
    	memcpy(sbBuff, &sb, sizeof(sb));
    	fwrite(sbBuff, sizeof(char), sizeof(sb), diskDescriptor);

	cout << "written 1 superblock" << endl;
	/* WRITING SUPER BLOCK FINISHED */
	
	/* WRITING FILE INODE MAP BEGIN */
	fseek(diskDescriptor, sb_size, SEEK_SET);

	struct fileInodeMap tempFileInodeMap[NO_OF_INODES];
	long long int fileInodeMapSize = sizeof(tempFileInodeMap);
	
	char fileInodeMapBuff[fileInodeMapSize];
    	memset(fileInodeMapBuff, 0, fileInodeMapSize);
    	memcpy(fileInodeMapBuff, tempFileInodeMap, fileInodeMapSize);
    	fwrite(fileInodeMapBuff, sizeof(char), fileInodeMapSize, diskDescriptor);

	cout << "written " << NO_OF_INODES << " file inode map" << endl;

	/* WRITING FILE INODE MAP END */

	/* WRITING INODE BLCOKS BEGIN */

	struct Inode tempInodeArr[NO_OF_INODES];
	int fileInodeSize = sizeof(tempInodeArr);
	int offset = sb_size + fileInodeMapSize;
	
	fseek(diskDescriptor, offset, SEEK_SET);
	
	char inodeBuff[fileInodeSize];
    	memset(inodeBuff, 0, fileInodeSize);
    	memcpy(inodeBuff, tempInodeArr, fileInodeSize);
    	fwrite(inodeBuff, sizeof(char), fileInodeSize, diskDescriptor);

	cout << "written " << NO_OF_INODES << " inodes" << endl;

	/* WRITING INODE BLOCKS END */

	fclose(diskDescriptor);
}

void initiateDisk(FILE* diskDescriptor) {
	char buffer[BLOCK_SIZE];
	memset(buffer, 0, BLOCK_SIZE);

	for(int i = 0; i < NO_OF_TOTAL_BLOCKS; i++) {
		fwrite(buffer, 1, BLOCK_SIZE, diskDescriptor);
	}
}

void create_disk(string diskname, string diskpath) {
	cout << "initiating disk creation" << endl;
	FILE *diskDescriptor = fopen(diskpath.c_str(), "wb");

	if(diskDescriptor == NULL) {
		cout << "could not create file" << endl;
	}	
	
	initiateDisk(diskDescriptor);

	cout << "completed disk initialization" << endl;

	fclose(diskDescriptor);
	
	//cout << "writing initial disk data" << endl;

	if(diskDescriptor == NULL) {
		cout << "could not fill disk file" << endl;
	}	
	
	initializeDisk(diskpath);	
	
	//fclose(diskDescriptor);

	return;
}

int main() {

	while(true) {

		int option;
		cin >> option;

		string diskname;

		if(option == 1) {
			cout << "Enter disk name : ";
			cin >>  diskname;

			diskpath = diskPrefix + diskname + diskExt;
			
			if(access(diskpath.c_str(), F_OK) != -1) {
				cout << "Disk already exists" << endl;
				continue;
			}

			create_disk(diskname, diskpath);
		}
	}
}
