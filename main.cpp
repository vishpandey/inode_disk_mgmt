#include <bits/stdc++.h>
#include <unistd.h>

#include <cmath>
#include <cstring>
#include <iostream>

using namespace std;

#define DATA_SIZE 512 * 1024 * 1024
#define BLOCK_SIZE 4096
#define NO_OF_DATA_BLOCKS 131072
#define NO_OF_INODES 78644
#define NO_OF_SB_BLOCKS 52
#define NO_OF_TOTAL_BLOCKS 209767

string diskPrefix = "disks/d_";
// string diskExt = ".txt";
string diskpath;

struct SuperBlock {
  bool free_inodes[NO_OF_INODES];
  bool free_data_blocks[NO_OF_DATA_BLOCKS];

  int no_of_files;
};

struct fileInodeMap {
  char filename[10];
  int inodeIndex;
};

struct Inode {
  char filename[10];
  int mode;
  int start_data_block;
  int filled_data_blocks;
  int bPointer[10];
};

long long int total_disk_size = 1 * sizeof(struct SuperBlock) +
                                NO_OF_INODES * sizeof(struct Inode) +
                                NO_OF_DATA_BLOCKS * BLOCK_SIZE;

long long int inodeStartingIndex =
    sizeof(struct SuperBlock) + NO_OF_INODES * sizeof(struct fileInodeMap);
long long int dataBlockStartingIndex =
    inodeStartingIndex + NO_OF_INODES * sizeof(struct Inode);

struct fileInodeMap globalFileInodeMap[NO_OF_INODES];
struct SuperBlock globalSuperBlock;
struct Inode globalInodeArr[NO_OF_INODES];
unordered_map<int, string> fdToFilenameMap;
unordered_map<string, int> fileNameToFdMap;

void initializeDiskFile(string diskpath) {
  /* *** Writing SUPER BLOCK */
  FILE *diskDescriptor = fopen(diskpath.c_str(), "wb");
  // fseek(diskDescriptor, 0, SEEK_SET);

  cout << "init sb object" << endl;
  struct SuperBlock sb;
  long int i;

  cout << "filling free blocks values" << endl;
  for (i = 0; i < NO_OF_INODES; i++) {
    sb.free_inodes[i] = true;
  }

  cout << "written free inodes" << endl;
  for (i = 0; i < NO_OF_DATA_BLOCKS; i++) {
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

  struct fileInodeMap *tempFileInodeMap =
      (struct fileInodeMap *)malloc(sizeof(struct fileInodeMap) * NO_OF_INODES);

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

  struct Inode *tempInodeArr =
      (struct Inode *)malloc(sizeof(struct Inode) * NO_OF_INODES);

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
  char *tempDataBuffer = (char *)malloc(tempDataSize);
  memset(tempDataBuffer, 0, tempDataSize);
  fwrite(tempDataBuffer, 1, tempDataSize, diskDescriptor);
  free(tempDataBuffer);

  fclose(diskDescriptor);
}

void initiateEmptySb(FILE *diskDescriptor) {
  fseek(diskDescriptor, 0, SEEK_SET);
  char sbBuffer[sizeof(struct SuperBlock) + 1];
  memset(sbBuffer, 0, sizeof(struct SuperBlock));
  fwrite(sbBuffer, 1, sizeof(struct SuperBlock), diskDescriptor);
}

void initiateEmptyFileInodeMap(FILE *diskDescriptor) {
  fseek(diskDescriptor, sizeof(struct SuperBlock), SEEK_SET);
  long long int tempFileInodeMapSize =
      NO_OF_INODES * sizeof(struct fileInodeMap);
  char *tempFileInodeBuffer = (char *)malloc(tempFileInodeMapSize);
  memset(tempFileInodeBuffer, 0, tempFileInodeMapSize);
  fwrite(tempFileInodeBuffer, 1, tempFileInodeMapSize, diskDescriptor);

  free(tempFileInodeBuffer);
}

void initiateEmptyInode(FILE *diskDescriptor) {
  fseek(diskDescriptor, inodeStartingIndex, SEEK_SET);
  long long int tempInodeSize = NO_OF_INODES * sizeof(struct Inode);
  char *tempInodeBuffer = (char *)malloc(tempInodeSize);
  memset(tempInodeBuffer, 0, tempInodeSize);
  fwrite(tempInodeBuffer, 1, tempInodeSize, diskDescriptor);

  free(tempInodeBuffer);
}

void initiateEmptyDataBlocks(FILE *diskDescriptor) {
  fseek(diskDescriptor, dataBlockStartingIndex, SEEK_SET);
  long long int tempDataSize = NO_OF_DATA_BLOCKS * BLOCK_SIZE;
  char *tempDataBuffer = (char *)malloc(tempDataSize);
  memset(tempDataBuffer, 0, tempDataSize);
  fwrite(tempDataBuffer, 1, tempDataSize, diskDescriptor);

  free(tempDataBuffer);
}

void initiateDisk(string diskpath) {
  cout << "initiating disk creation" << endl;
  FILE *diskDescriptor = fopen(diskpath.c_str(), "wb");
  fseek(diskDescriptor, 0, SEEK_SET);
  if (diskDescriptor == NULL) {
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
  // cout << "writing initial disk data" << endl;

  /* if(diskDescriptor == NULL) {
          cout << "could not fill disk file" << endl;
  }*/

  // fclose(diskDescriptor);

  return;
}
bool mountDisk(string diskPath) {
  FILE *diskDescriptor = fopen(diskPath.c_str(), "rb+");

  char *sb_buff = (char *)malloc(sizeof(globalSuperBlock));
  memset(sb_buff, 0, sizeof(globalSuperBlock));
  fread(sb_buff, sizeof(char), sizeof(globalSuperBlock), diskDescriptor);
  memcpy(&globalSuperBlock, sb_buff, sizeof(globalSuperBlock));
  free(sb_buff);
  cout << "copied superblock" << endl;

  long long int globalFileInodeMapSize = sizeof(globalFileInodeMap);
  char *fileInodeMapBuff = (char *)malloc(globalFileInodeMapSize);
  fseek(diskDescriptor, sizeof(struct SuperBlock), SEEK_SET);
  memset(fileInodeMapBuff, 0, globalFileInodeMapSize);
  fread(fileInodeMapBuff, sizeof(char), globalFileInodeMapSize, diskDescriptor);
  memcpy(&globalFileInodeMap, fileInodeMapBuff, globalFileInodeMapSize);
  free(fileInodeMapBuff);
  cout << "copied file to inode map buf" << endl;

  long long int globalInodeArrSize = sizeof(globalInodeArr);
  char *inodeArrBuff = (char *)malloc(globalInodeArrSize);
  fseek(diskDescriptor, inodeStartingIndex, SEEK_SET);
  memset(inodeArrBuff, 0, globalInodeArrSize);
  fread(inodeArrBuff, sizeof(char), globalInodeArrSize, diskDescriptor);
  memcpy(&globalInodeArr, inodeArrBuff, globalInodeArrSize);
  free(inodeArrBuff);
  cout << "copied inode blocks" << endl;

  /*long long int globalDataSize = NO_OF_DATA_BLOCKS * DATA_SIZE;
   char *dataBuff = (char *)malloc(globalDataSize);
   fseek(diskDescriptor, dataBlockStartingIndex, SEEK_SET);
   memset(dataBuff, 0, globalDataSize);
   fread(dataBuff, sizeof(char), globalDataSize);
   memcpy(&global, dataBuff, globalDataSize);
   free(inodeArrBuff);*/

  fclose(diskDescriptor);

  return true;
}

int getInodeIndexFromFilename(string filename) {
	int temp_inode_index;
	bool filename_to_open_found = false;
	
	for (int i = 0; i < NO_OF_INODES; i++) {
          if (strcmp(globalFileInodeMap[i].filename,
                     filename.c_str()) == 0) {
            temp_inode_index = globalFileInodeMap[i].inodeIndex;
            filename_to_open_found = true;
            break;
          }
        }

        if (!filename_to_open_found) {
          return -1;
        }

	return temp_inode_index;
}

string getFilenameFromFd(int fd) {
	auto fd_iter = fdToFilenameMap.find(fd);

        if (fd_iter == fdToFilenameMap.end()) {
          return "";
        }

        return fd_iter->second;
}

int getFileModeFromInodeIndex(int index) {

	return globalInodeArr[index].mode;
}

int getNextFreeDataBlock() {
	//cout << globalSuperBlock.free_data_blocks[0] << endl;
	for(int i = 0; i < NO_OF_DATA_BLOCKS; i++) {
		if (globalSuperBlock.free_data_blocks[i] == 1) {
			return i;
		}
	}

	return -1;
}

void writeContentToFile(string diskpath, char *mode, int start_block, int end_block, string content) {
	FILE *disk_descriptor = fopen(diskpath.c_str(), mode);

	fseek(disk_descriptor, dataBlockStartingIndex + (start_block * DATA_SIZE), SEEK_SET);
	
	long int content_size = content.length();
	char *content_buffer = (char *)malloc(content_size + 1);
  	strcpy(content_buffer, content.c_str());
  	fwrite(content_buffer, 1, strlen(content_buffer), disk_descriptor);

	free(content_buffer);

	fclose(disk_descriptor);	
}

string readFile(string diskpath, char *mode, int start_block, int end_block) {
	FILE *disk_descriptor = fopen(diskpath.c_str(), mode);
	string file_content = "";
	for(int i = start_block; i <= end_block; i++) {
		char *content_buffer = (char *)malloc(DATA_SIZE + 1);
		fseek(disk_descriptor, dataBlockStartingIndex + (i * DATA_SIZE), SEEK_SET);
		fread(content_buffer, sizeof(char), DATA_SIZE, disk_descriptor);
		file_content += string(content_buffer);

		free(content_buffer);
	}

	fclose(disk_descriptor);

	return file_content;
}

void verifyWrittenContent(string diskpath, int start_block, int end_block, string content) {
	string actual_content = readFile(diskpath, "rb+", start_block, end_block);
	
	if (actual_content == content) {
		return true;
	}
	
	return false;
}

void setDataBlocksBusy(long int start_block, long int end_block) {
	end_block = (end_block >= NO_OF_DATA_BLOCKS) ? (NO_OF_DATA_BLOCKS - 1) : end_block;
	for(int i = start_block; i <= end_block; i++) {
		globalSuperBlock.free_data_blocks[i] = false;
	}
}

void addDataBlockRefToInode(int inode_index, long int start_block, long int end_block) {
	for(int i = start_block; i <= end_block; i++) {
		globalInodeArr[inode_index].bPointer[i - start_block] = i;
	}

	globalInodeArr[inode_index].filled_data_blocks = end_block - start_block + 1;
}

void initiateWriteOperation(string filepath) {
	int fd_to_write;
        string content_to_write;
	string enter;
	
	cout << "Please enter fd to write to : " << endl;;
        cin >> fd_to_write;
	
	string filename = getFilenameFromFd(fd_to_write);

	if(filename == "") {
		cout << "Could not find file using fd" << endl;
		return;
	}

	int inode_index = getInodeIndexFromFilename(filename);
	
	if (inode_index == -1) {
		cout << "Could not find inode for the file" << endl;
		return;
	}

	int file_mode = getFileModeFromInodeIndex(inode_index);

	if (file_mode != 2) {
		cout << "File is not opened in correct mode" << endl;
		return;
	}
		
        cout << "Please enter the content to write : " << endl;
	cin.ignore();
	getline(cin, content_to_write);
	
	struct Inode file_inode = globalInodeArr[inode_index];
	
	long int content_length = content_to_write.length();

	long int data_blocks_required = (content_length / DATA_SIZE) + 1;
	
	cout << "data blocks required to write the content" << data_blocks_required << endl;

	int start_block = globalInodeArr[inode_index].start_data_block;

       	int end_block = (data_blocks_required > 10) ? (start_block + 9) : (start_block + data_blocks_required - 1);

	if (end_block >= NO_OF_DATA_BLOCKS) {
		end_block = NO_OF_DATA_BLOCKS - 1;
	}
	
	cout << "writing content from data block " << start_block << " to " << end_block << endl;

	writeContentToFile(diskpath, "wb+", start_block, end_block, content_to_write);
	if (verifyWrittenContent(diskpath, start_block, end_block, content_to_write)) {
		cout << "Content written successfully and verified" << endl;
	} else {
		cout << "Content written successfully but could not be verified" << endl;
	}	
}

int main() {
  bool isMounted = false;
  string filename, currently_open_filename;
  int currently_open_file_mode;

  while (true) {
    int option;

    string diskname;
    if (!isMounted) {
      cin >> option;
      if (option == 1) {
        cout << "Enter disk name : ";
        cin >> diskname;

        // diskpath = diskPrefix + diskname + diskExt;
        diskpath = diskPrefix + diskname;

        if (access(diskpath.c_str(), F_OK) != -1) {
          cout << "Disk already exists" << endl;
          continue;
        }

        // create_disk(diskname, diskpath);
        initializeDiskFile(diskpath);
      } else if (option == 2) {
        cout << "Enter disk name to mount  ";
        cin >> diskname;
        if (access(diskpath.c_str(), F_OK) == -1) {
          cout << "Disk doesn't exists" << endl;
          continue;
        }

        diskpath = diskPrefix + diskname;

        if (mountDisk(diskpath)) {
          isMounted = true;
          cout << "disk mounted successfully" << endl;
        }
      }
    } else {
      cin >> option;

      if (option == 1) {
        bool fileFound = false;

        cout << "Enter filename to create: ";
        cin >> filename;

        char *charFilename = (char *)malloc(filename.length() + 1);
        strcpy(charFilename, filename.c_str());

        if (filename.length() > 10) {
          cout << "filename length should be of length 10" << endl;
          continue;
        }

        for (int i = 0; i < NO_OF_INODES; i++) {
          if (strcmp(globalFileInodeMap[i].filename, charFilename) == 0) {
            fileFound = true;
            break;
          }
        }

        if (fileFound) {
          cout << "file already exists" << endl;
          continue;
        }

        int freeInodeIndex = -1;
        for (int i = 0; i < NO_OF_INODES; i++) {
          if (globalSuperBlock.free_inodes[i] == true) {
            freeInodeIndex = i;
            break;
          }
        }

        if (freeInodeIndex == -1) {
          cout << "no free inode available" << endl;
          continue;
        }

	int first_free_data_block = getNextFreeDataBlock();
	
	if(first_free_data_block == -1) {
		cout << "no free data blocks available" << endl;
		continue;
	}

	
	cout << "got free inode for file : " << freeInodeIndex << endl;
        strcpy(globalFileInodeMap[globalSuperBlock.no_of_files].filename,
               charFilename);
        globalFileInodeMap[globalSuperBlock.no_of_files].inodeIndex =
            freeInodeIndex;
        strcpy(globalInodeArr[freeInodeIndex].filename, charFilename);
	
	cout << "first free data block : " << first_free_data_block << endl;
	globalInodeArr[freeInodeIndex].start_data_block = first_free_data_block;
	globalInodeArr[freeInodeIndex].filled_data_blocks = 0;

	setDataBlocksBusy(first_free_data_block, first_free_data_block + 9);
	cout << "set data blocks as busy" << endl;

	addDataBlockRefToInode(freeInodeIndex, first_free_data_block, first_free_data_block + 9 );

        int fd_index = fdToFilenameMap.size();
        fdToFilenameMap.insert(make_pair(fd_index, filename));
	cout << "insert entry in fd to filename map" << endl;
        fileNameToFdMap.insert(make_pair(filename, fd_index));
	cout << "insert entry in filename to fd map" << endl;

        free(charFilename);

        cout << "Created file sccessfully" << endl;

        continue;
      } else if (option == 2) {
        cout << "Enter filename to open: ";
        cin >> currently_open_filename;

        cout << "Enter mode: " << endl;
        cin >> currently_open_file_mode;

        int original_file_mode = currently_open_file_mode + 1;

        if (original_file_mode != 0 && original_file_mode != 1 &&
            original_file_mode != 2) {
          cout << "mode not allowed" << endl;
          continue;
        }

        bool filename_to_open_found = false;
        int temp_inode_index = -1;

        for (int i = 0; i < NO_OF_INODES; i++) {
          if (strcmp(globalFileInodeMap[i].filename,
                     currently_open_filename.c_str()) == 0) {
            temp_inode_index = globalFileInodeMap[i].inodeIndex;
            filename_to_open_found = true;
            break;
          }
        }

        if (!filename_to_open_found) {
          cout << "file doesn't exist" << endl;
          continue;
        }

        if (globalInodeArr[temp_inode_index].mode != 0) {
          cout << "File is already opened" << endl;
          continue;
        }

        if (temp_inode_index == -1) {
          cout << "could not find file inode." << endl;
	  continue;
        }
	
	auto fd_iter = fileNameToFdMap.find(currently_open_filename);

        if (fd_iter == fileNameToFdMap.end()) {
          cout << "file doesn't exist on the disk" << endl;
          continue;
        }

        globalInodeArr[temp_inode_index].mode = currently_open_file_mode + 1;
 
        cout << "fd: " << fd_iter->second
             << " filename: " << currently_open_filename
             << " mode: " << globalInodeArr[temp_inode_index].mode << endl; 
      } else if (option == 4) {

	initiateWriteOperation(diskpath);
      
      }
    }
  }

  return 0;
}
