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
//string diskExt = ".txt";
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
long long int dataBlockStartingIndex = inodeStartingIndex + NO_OF_INODES * sizeof(struct Inode);

struct fileInodeMap globalFileInodeMap[NO_OF_INODES];
struct SuperBlock globalSuperBlock;

void initializeDiskFile(string diskpath) {
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
	
	char *sbBuff = (char *)malloc(sizeof(char) * (sb_size + 1));
    	memset(sbBuff, 0, sb_size);
    	memcpy(sbBuff, &sb, sizeof(sb));
    	
	fwrite(sbBuff, sizeof(char), sizeof(sb), diskDescriptor);
	
	free(sbBuff);
	
	cout << "written 1 superblock" << endl;
	/* WRITING SUPER BLOCK FINISHED */
	
	/* WRITING FILE INODE MAP BEGIN */
	fseek(diskDescriptor, sb_size, SEEK_SET);

	struct fileInodeMap *tempFileInodeMap = (struct fileInodeMap*)malloc(sizeof(struct fileInodeMap) * NO_OF_INODES);

	long long int fileInodeMapSize = sizeof(tempFileInodeMap);
	
	char *fileInodeMapBuff = (char *)malloc(sizeof(char) * fileInodeMapSize);
    	
	memset(fileInodeMapBuff, 0, fileInodeMapSize);
    	memcpy(fileInodeMapBuff, tempFileInodeMap, fileInodeMapSize);
    	
	fwrite(fileInodeMapBuff, sizeof(char), fileInodeMapSize, diskDescriptor);
	
	free(fileInodeMapBuff);
	free(tempFileInodeMap);
	
	cout << "written " << NO_OF_INODES << " file inode map" << endl;

	/* WRITING FILE INODE MAP END */

	/* WRITING INODE BLCOKS BEGIN */

	struct Inode *tempInodeArr = (struct Inode*)malloc(sizeof(struct Inode) * NO_OF_INODES);
	
	long long int fileInodeSize = sizeof(tempInodeArr);
	long long int offset = sb_size + fileInodeMapSize;
	
	fseek(diskDescriptor, offset, SEEK_SET);
	
	char *inodeBuff = (char *)malloc(sizeof(char) * fileInodeSize);
    	memset(inodeBuff, 0, fileInodeSize);
    	memcpy(inodeBuff, tempInodeArr, fileInodeSize);
    	
	fwrite(inodeBuff, sizeof(char), fileInodeSize, diskDescriptor);

	free(tempInodeArr);
	free(inodeBuff);

	cout << "written " << NO_OF_INODES << " inodes" << endl;

	/* WRITING INODE BLOCKS END */

	fseek(diskDescriptor, dataBlockStartingIndex, SEEK_SET);
	long long int tempDataSize = NO_OF_DATA_BLOCKS * BLOCK_SIZE;
	char *tempDataBuffer = (char *) malloc(tempDataSize);
	memset(tempDataBuffer, 0, tempDataSize);
	fwrite(tempDataBuffer, 1, tempDataSize, diskDescriptor);
	free(tempDataBuffer);

	fclose(diskDescriptor);
}

void initiateEmptySb(FILE* diskDescriptor) {
	fseek(diskDescriptor, 0, SEEK_SET);
	char sbBuffer[sizeof(struct SuperBlock) + 1];
	memset(sbBuffer, 0, sizeof(struct SuperBlock));
	fwrite(sbBuffer, 1, sizeof(struct SuperBlock), diskDescriptor);
}

void initiateEmptyFileInodeMap(FILE* diskDescriptor) {
	fseek(diskDescriptor, sizeof(struct SuperBlock), SEEK_SET);
	long long int tempFileInodeMapSize = NO_OF_INODES * sizeof(struct fileInodeMap);
	char *tempFileInodeBuffer = (char *) malloc(tempFileInodeMapSize);
	memset(tempFileInodeBuffer, 0, tempFileInodeMapSize);
	fwrite(tempFileInodeBuffer, 1, tempFileInodeMapSize, diskDescriptor);

	free(tempFileInodeBuffer);
}

void initiateEmptyInode(FILE* diskDescriptor) {
	fseek(diskDescriptor, inodeStartingIndex, SEEK_SET);
	long long int tempInodeSize = NO_OF_INODES * sizeof(struct Inode);
	char *tempInodeBuffer = (char *) malloc(tempInodeSize);
	memset(tempInodeBuffer, 0, tempInodeSize);
	fwrite(tempInodeBuffer, 1, tempInodeSize, diskDescriptor);

	free(tempInodeBuffer);
}

void initiateEmptyDataBlocks(FILE *diskDescriptor) {
	fseek(diskDescriptor, dataBlockStartingIndex, SEEK_SET);
	long long int tempDataSize = NO_OF_DATA_BLOCKS * BLOCK_SIZE;
	char *tempDataBuffer = (char *) malloc(tempDataSize);
	memset(tempDataBuffer, 0, tempDataSize);
	fwrite(tempDataBuffer, 1, tempDataSize, diskDescriptor);

	free(tempDataBuffer);
}


void initiateDisk(string diskpath) {
	cout << "initiating disk creation" << endl;
	FILE *diskDescriptor = fopen(diskpath.c_str(), "wb");
	fseek(diskDescriptor, 0, SEEK_SET);
	if(diskDescriptor == NULL) {
		cout << "could not create file" << endl;
	}	

	initiateEmptySb(diskDescriptor);
	initiateEmptyFileInodeMap(diskDescriptor);
	initiateEmptyInode(diskDescriptor);
	initiateEmptyDataBlocks(diskDescriptor);
	

	fclose(diskDescriptor);

}

void create_disk(string diskname, string diskpath) {	
	initiateDisk(diskpath);
	cout << "completed disk initialization" << endl;	
	//cout << "writing initial disk data" << endl;

	/* if(diskDescriptor == NULL) {
		cout << "could not fill disk file" << endl;
	}*/	
	
	//fclose(diskDescriptor);

	return;
}
void mountDisk(string diskPath) {
	FILE* diskptr = fopen(diskPath.c_str(), "rb+");

	char sb_buff[sizeof(globalSuperBlock)];
    	memset(sb_buff, 0, sizeof(globalSuperBlock));
    	fread(sb_buff, sizeof(char), sizeof(globalSuperBlock), diskptr);
    	memcpy(&globalSuperBlock, sb_buff, sizeof(globalSuperBlock));
	fclose(diskptr);
}


int main() {

	while(true) {

		int option;
		cin >> option;

		string diskname;

		if(option == 1) {
			cout << "Enter disk name : ";
			cin >>  diskname;

			//diskpath = diskPrefix + diskname + diskExt;
			diskpath = diskPrefix + diskname;
			
			if(access(diskpath.c_str(), F_OK) != -1) {
				cout << "Disk already exists" << endl;
				continue;
			}

			//create_disk(diskname, diskpath);
			initializeDiskFile(diskpath);
		} else if (option == 2) {
			mountDisk(diskpath);
		}
	}

	return 0;
}
