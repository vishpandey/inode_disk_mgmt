#include <iostream>
#include <unistd.h>
#include <cmath>

using namespace std;

#define DATA_SIZE 512 * 1024 * 1024
#define BLOCK_SIZE 4096
#define NO_OF_DATA_BLOCKS 131072
#define NO_OF_INODES 78644
#define NO_OF_SB_BLOCKS 52
#define NO_OF_TOTAL_BLOCKS 209767

string diskPrefix = "disks/d_";
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

struct fileInodeMap globalFileInodeMap[NO_OF_INODES];
struct SuperBlock globalSuperBlock;

void initializeDisk(struct SuperBlock *sb, FILE *diskDescriptor) {
	for(int i = 0; i < NO_OF_INODES; i++) {
		sb->free_inodes[i] = true;
	}
	for(int i = 0; i < NO_OF_DATA_BLOCKS; i++) {
		sb->free_data_blocks[i] = true;
	}

	int sb_size = ceil(((float)sizeof(sb)));
	fseek(diskDescriptor, 0, SEEK_SET);

	fwrite(&sb, sizeof(struct SuperBlock), 1, diskDescriptor);

	cout << "written 1 superblock" << endl;

	fseek(diskDescriptor, sb_size, SEEK_SET);

	struct fileInodeMap tempFileInodeMap[NO_OF_INODES];
	int fileInodeMapSize = ceil(((float)sizeof(struct fileInodeMap)));

	fwrite(&tempFileInodeMap, fileInodeMapSize, NO_OF_INODES, diskDescriptor);

	cout << "written " << NO_OF_INODES << " file inode map" << endl;

	int fileInodeSize = ceil(((float)sizeof(struct Inode)));
	int offset = sb_size + NO_OF_INODES * fileInodeMapSize;
	fseek(diskDescriptor, offset, SEEK_SET);

	struct Inode tempInodeArr[NO_OF_INODES];
	fwrite(&tempInodeArr, fileInodeSize, NO_OF_INODES, diskDescriptor);

	cout << "written " << NO_OF_INODES << " inodes" << endl;

	fclose(diskDescriptor);
}


void create_disk(string diskname, string diskpath) {
	FILE *diskDescriptor = fopen(diskpath.c_str(), "wb"); 

	fseek(diskDescriptor, total_disk_size, SEEK_SET);
	fputc('\0', diskDescriptor);

	struct SuperBlock *sb = (struct SuperBlock*)malloc(1 * sizeof(struct SuperBlock));;
	initializeDisk(sb, diskDescriptor);

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

			diskpath = diskPrefix + diskname;
			
			if(access(diskpath.c_str(), F_OK) != -1) {
				cout << "Disk already exists" << endl;
				continue;
			}

			create_disk(diskname, diskpath);
		}
	}
}