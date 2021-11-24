#include <bits/stdc++.h>
#include <unistd.h>
#include <signal.h>
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
string mount_diskpath = "";
string mount_diskname = "";
bool isMounted = false;

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
  long int file_size;
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

  sb.no_of_files = 0;

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

void unmountDisk() {
  FILE *diskDescriptor = fopen(mount_diskpath.c_str(), "rb+");
  char *sbBuff = (char *)malloc(sizeof(char) * sizeof(globalSuperBlock));
  memset(sbBuff, 0, sizeof(globalSuperBlock));
  memcpy(sbBuff, &globalSuperBlock, sizeof(globalSuperBlock));

  fwrite(sbBuff, sizeof(char), sizeof(globalSuperBlock), diskDescriptor);

  free(sbBuff);

  fseek(diskDescriptor, sizeof(globalSuperBlock), SEEK_SET);

  long long int fileInodeMapSize = sizeof(globalFileInodeMap);

  char *fileInodeMapBuff = (char *)malloc(sizeof(char) * fileInodeMapSize);

  memset(fileInodeMapBuff, 0, fileInodeMapSize);
  memcpy(fileInodeMapBuff, globalFileInodeMap, fileInodeMapSize);

  fwrite(fileInodeMapBuff, sizeof(char), fileInodeMapSize, diskDescriptor);

  free(fileInodeMapBuff);

  long long int fileInodeSize = sizeof(globalInodeArr);
  fseek(diskDescriptor, inodeStartingIndex, SEEK_SET);

  char *inodeBuff = (char *)malloc(sizeof(char) * fileInodeSize);
  memset(inodeBuff, 0, fileInodeSize);
  memcpy(inodeBuff, globalInodeArr, fileInodeSize);

  fwrite(inodeBuff, sizeof(char), fileInodeSize, diskDescriptor);

  free(inodeBuff);

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

  fclose(diskDescriptor);

  return true;
}

int getInodeIndexFromFilename(string filename) {
  int temp_inode_index;
  bool filename_to_open_found = false;

  for (int i = 0; i < NO_OF_INODES; i++) {
    if (strcmp(globalFileInodeMap[i].filename, filename.c_str()) == 0) {
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

int getFileModeFromInodeIndex(int index) { return globalInodeArr[index].mode; }

int getNextFreeDataBlock() {
  // cout << globalSuperBlock.free_data_blocks[0] << endl;
  for (int i = 0; i < NO_OF_DATA_BLOCKS; i++) {
    if (globalSuperBlock.free_data_blocks[i] == 1) {
      return i;
    }
  }

  return -1;
}

string readFile(string diskpath, char *mode, int start_block, int end_block) {
  FILE *disk_descriptor = fopen(diskpath.c_str(), mode);
  string file_content = "";
  for (int i = start_block; i <= end_block; i++) {
    char *content_buffer = (char *)malloc(BLOCK_SIZE + 1);
    fseek(disk_descriptor, dataBlockStartingIndex + (i * BLOCK_SIZE), SEEK_SET);
    fread(content_buffer, sizeof(char), BLOCK_SIZE, disk_descriptor);
    file_content += string(content_buffer);

    free(content_buffer);
  }

  fclose(disk_descriptor);

  return file_content;
}

void writeContentToFile(string diskpath, char *mode, int start_block,
                        int end_block, string content) {
  FILE *disk_descriptor = fopen(diskpath.c_str(), mode);

  fseek(disk_descriptor, dataBlockStartingIndex + (start_block * BLOCK_SIZE),
        SEEK_SET);

  long int content_size = content.length();
  char *content_buffer = (char *)malloc(content_size + 1);
  strcpy(content_buffer, content.c_str());
  fwrite(content_buffer, 1, strlen(content_buffer), disk_descriptor);

  free(content_buffer);

  fclose(disk_descriptor);
}

void appendContentToFile(int start_block, int offset, string content) {
  FILE *disk_descriptor = fopen(mount_diskpath.c_str(), "rb+");
  fseek(disk_descriptor,
        dataBlockStartingIndex + ((start_block)*BLOCK_SIZE) + offset, SEEK_SET);
  long int content_size = content.length();
  char *content_buffer = (char *)malloc(content_size + 1);
  strcpy(content_buffer, content.c_str());
  fwrite(content_buffer, 1, strlen(content_buffer), disk_descriptor);

  free(content_buffer);

  fclose(disk_descriptor);
}

bool verifyWrittenContent(string diskpath, int start_block, int end_block,
                          string content) {
  string actual_content = readFile(diskpath, "rb+", start_block, end_block);

  if (actual_content == content) {
    return true;
  }

  return false;
}

void setDataBlocksBusy(long int start_block, long int end_block) {
  end_block =
      (end_block >= NO_OF_DATA_BLOCKS) ? (NO_OF_DATA_BLOCKS - 1) : end_block;
  for (int i = start_block; i <= end_block; i++) {
    globalSuperBlock.free_data_blocks[i] = false;
  }
}

void setInodeBusy(int inode_index) {
  globalSuperBlock.free_inodes[inode_index] = false;
}

void addDataBlockRefToInode(int inode_index, long int start_block,
                            long int end_block) {
  for (int i = start_block; i <= end_block; i++) {
    globalInodeArr[inode_index].bPointer[i - start_block] = i;
  }

  globalInodeArr[inode_index].filled_data_blocks = end_block - start_block + 1;
}

string readAllFileContent(string diskpath, int start_block,
                          long int size_to_read) {
  FILE *disk_descriptor = fopen(diskpath.c_str(), "rb+");
  string file_content = "";
  long int end_block = start_block + (size_to_read / BLOCK_SIZE);

  cout << "reading from start block : " << start_block << endl;
  cout << "reading from end block : " << end_block << endl;

  for (int i = start_block; i <= end_block; i++) {
    char *content_buffer = (char *)malloc(BLOCK_SIZE + 1);
    fseek(disk_descriptor, dataBlockStartingIndex + (i * BLOCK_SIZE), SEEK_SET);
    fread(content_buffer, sizeof(char), BLOCK_SIZE, disk_descriptor);
    file_content += string(content_buffer);

    free(content_buffer);
  }

  fclose(disk_descriptor);

  return file_content;
}

void initiateWriteOperation(string diskpath) {
  int fd_to_write;
  string content_to_write;
  string enter;

  cout << "Please enter fd to write to : " << endl;
  ;
  cin >> fd_to_write;

  string filename = getFilenameFromFd(fd_to_write);

  if (filename == "") {
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

  long int data_blocks_required = (content_length / BLOCK_SIZE) + 1;

  cout << "data blocks required to write the content" << data_blocks_required
       << endl;

  int start_block = globalInodeArr[inode_index].start_data_block;

  int end_block = (data_blocks_required > 10)
                      ? (start_block + 9)
                      : (start_block + data_blocks_required - 1);

  if (end_block >= NO_OF_DATA_BLOCKS) {
    end_block = NO_OF_DATA_BLOCKS - 1;
  }

  cout << "writing content from data block " << start_block << " to "
       << end_block << endl;

  writeContentToFile(diskpath, "rb+", start_block, end_block, content_to_write);
  if (verifyWrittenContent(diskpath, start_block, end_block,
                           content_to_write)) {
    cout << "Content written successfully and verified" << endl;
  } else {
    cout << "Content written successfully but could not be verified" << endl;
  }

  globalInodeArr[inode_index].file_size = content_to_write.length();
}

void initiateAppendOperation(int inode_index) {
  string content_to_append;
  cout << "Please enter the content to append : " << endl;
  cin.ignore();
  getline(cin, content_to_append);

  long int start_block = globalInodeArr[inode_index].start_data_block;
  long int file_size = globalInodeArr[inode_index].file_size;
  long int last_filled_block = start_block + (file_size / BLOCK_SIZE);
  long int content_to_append_size = content_to_append.length();
  int content_in_last_block_size = file_size % BLOCK_SIZE;

  int offset = 0;
  long int new_start_block, new_end_block;
  long int max_allowed_content_size;

  if (content_in_last_block_size == 0) {
    new_start_block = last_filled_block + 1;

    if (new_start_block > NO_OF_DATA_BLOCKS ||
        new_start_block > start_block + 9) {
      cout << "File is already full" << endl;
      return;
    }

    new_end_block = new_start_block + (content_to_append_size / BLOCK_SIZE) - 1;

    if (new_end_block > NO_OF_DATA_BLOCKS || new_end_block > start_block + 9) {
      cout << "File is full. Some data may be truncated" << endl;

      new_end_block = (new_end_block > NO_OF_DATA_BLOCKS)
                          ? (NO_OF_DATA_BLOCKS - 1)
                          : (start_block + 9);
    }

    max_allowed_content_size =
        ((new_start_block - new_end_block) + 1) * BLOCK_SIZE;

    offset = 0;

  } else {
    new_start_block = last_filled_block;

    if (new_start_block > NO_OF_DATA_BLOCKS ||
        new_start_block > start_block + 9) {
      cout << "File is already full" << endl;

      return;
    }

    offset = BLOCK_SIZE - content_in_last_block_size;
    if (content_to_append_size < offset) {
      new_end_block = new_start_block;
    } else {
      new_end_block =
          new_start_block + ((content_to_append_size - offset) / BLOCK_SIZE);
    }

    if (new_end_block > NO_OF_DATA_BLOCKS || new_end_block > start_block + 9) {
      cout << "File is full. Some data may be truncated" << endl;

      new_end_block = (new_end_block > NO_OF_DATA_BLOCKS)
                          ? (NO_OF_DATA_BLOCKS - 1)
                          : (start_block + 9);
    }

    max_allowed_content_size =
        ((new_start_block - new_end_block + 1)) * BLOCK_SIZE;
  }

  /* cout << "max size of content to write : " << max_allowed_content_size <<
  endl; cout << "new_start_block : " << new_start_block << endl; cout <<
  "new_end_block3 : " << new_end_block << endl;*/

  if (content_to_append_size > max_allowed_content_size) {
    content_to_append.resize(max_allowed_content_size);
  }

  appendContentToFile(new_start_block, content_in_last_block_size,
                      content_to_append);

  globalInodeArr[inode_index].file_size += content_to_append.length();
}

string getModeText(int mode) {
  if (mode == 1) {
    return "read";
  } else if (mode == 2) {
    return "write";
  }

  return "append";
}

void signal_callback_handler(int signum) {
 	if (isMounted) {
		cout << "Cannot close the application as mount path is busy" << endl;
		return;
	}

	exit(signum);
}

int main() {

  signal(SIGINT, signal_callback_handler);

  isMounted = false; 
  bool isUnmountAllowed = true;
  string filename, currently_open_filename, create_diskpath;
  ;
  int currently_open_file_mode;
  string filename_to_read, filename_to_close, filename_to_append;
  bool filename_to_open_found = false, filename_to_read_found = false,
       filename_to_close_found = false;
  bool filename_to_append_found = false;
  int temp_inode_index = -1, fd_to_read, fd_to_close, fd_to_append;

  while (true) {
    int option;

    filename_to_open_found = false;
    filename_to_read_found = false;
    filename_to_close_found = false;
    filename_to_append_found = false;
    isUnmountAllowed = true;

    string create_diskname;

    if (!isMounted) {
      cin >> option;
      if (option == 1) {
        cout << "Enter disk name : ";
        cin >> create_diskname;

        // diskpath = diskPrefix + diskname + diskExt;
        string create_diskpath = diskPrefix + create_diskname;

        if (access(create_diskpath.c_str(), F_OK) != -1) {
          cout << "Disk already exists" << endl;
          continue;
        }

        // create_disk(diskname, diskpath);
        initializeDiskFile(create_diskpath);
      } else if (option == 2) {
        cout << "Enter disk name to mount  ";
        cin >> mount_diskname;

        mount_diskpath = diskPrefix + mount_diskname;

        if (access(mount_diskpath.c_str(), F_OK) == -1) {
          cout << "Disk doesn't exists" << endl;
          continue;
        }

        if (mountDisk(mount_diskpath)) {
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

        if (first_free_data_block == -1) {
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
        globalInodeArr[freeInodeIndex].file_size = 0;

        setDataBlocksBusy(first_free_data_block, first_free_data_block + 9);
        setInodeBusy(freeInodeIndex);
        cout << "set data blocks as busy" << endl;

        addDataBlockRefToInode(freeInodeIndex, first_free_data_block,
                               first_free_data_block + 9); 

        free(charFilename);

        globalSuperBlock.no_of_files++;
        cout << "Created file sccessfully" << endl;
      } else if (option == 2) {
        cout << "Enter filename to open: ";
        cin >> currently_open_filename;

        cout << "Enter mode: " << endl;
        cin >> currently_open_file_mode;

        int original_file_mode = currently_open_file_mode + 1;

        if (original_file_mode != 1 && original_file_mode != 2 &&
            original_file_mode != 3) {
          cout << "mode not allowed" << endl;
          continue;
        }

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

	int fd_index = fdToFilenameMap.size();
        fdToFilenameMap.insert(make_pair(fd_index, currently_open_filename));
        cout << "insert entry in fd to filename map" << endl;
        fileNameToFdMap.insert(make_pair(currently_open_filename, fd_index));
        cout << "insert entry in filename to fd map" << endl;

        /*auto fd_iter = fileNameToFdMap.find(currently_open_filename);

        if (fd_iter == fileNameToFdMap.end()) {
          cout << "file doesn't exist on the disk" << endl;
          continue;
        }*/

        globalInodeArr[temp_inode_index].mode = currently_open_file_mode + 1;

        cout << "fd: " << fd_index
             << " filename: " << currently_open_filename
             << " mode: " << globalInodeArr[temp_inode_index].mode << endl;
      } else if (option == 3) {
        cout << "enter fd to read : " << endl;
        cin >> fd_to_read;

        auto fd_iter = fdToFilenameMap.find(fd_to_read);

        if (fd_iter == fdToFilenameMap.end()) {
          cout << "file doesn't exist on the disk" << endl;
          continue;
        }

        filename_to_read = fd_iter->second;
        for (int i = 0; i < NO_OF_INODES; i++) {
          if (strcmp(globalFileInodeMap[i].filename,
                     filename_to_read.c_str()) == 0) {
            temp_inode_index = globalFileInodeMap[i].inodeIndex;
            filename_to_read_found = true;
            break;
          }
        }

        if (!filename_to_read_found) {
          cout << "file doesn't exist" << endl;
          continue;
        }

        if (temp_inode_index == -1) {
          cout << "could not find file inode." << endl;
          continue;
        }

        if (globalInodeArr[temp_inode_index].mode != 1) {
          cout << "File is not opened in correct mode" << endl;
          continue;
        }

        long int start_block =
            globalInodeArr[temp_inode_index].start_data_block;
        long int file_size = globalInodeArr[temp_inode_index].file_size;
        cout << "Reading " << file_size << " bytes of data" << endl;

        cout << readAllFileContent(mount_diskpath, start_block, file_size)
             << endl;
      } else if (option == 4) {
        initiateWriteOperation(mount_diskpath);
      } else if (option == 6) {
        cout << "enter fd to close : " << endl;
        cin >> fd_to_close;

        auto fd_iter = fdToFilenameMap.find(fd_to_close);

        if (fd_iter == fdToFilenameMap.end()) {
          cout << "file doesn't exist on the disk" << endl;
          continue;
        }

        filename_to_close = fd_iter->second;
        for (int i = 0; i < NO_OF_INODES; i++) {
          if (strcmp(globalFileInodeMap[i].filename,
                     filename_to_close.c_str()) == 0) {
            temp_inode_index = globalFileInodeMap[i].inodeIndex;
            filename_to_close_found = true;
            break;
          }
        }

        if (!filename_to_close_found) {
          cout << "file doesn't exist" << endl;
          continue;
        }

        if (temp_inode_index == -1) {
          cout << "could not find file inode." << endl;
          continue;
        }

        globalInodeArr[temp_inode_index].mode = 0;

        cout << "File closed Successfully" << endl;
      } else if (option == 5) {
        cout << "Enter fd to append" << endl;
        cin >> fd_to_append;

        auto fd_iter = fdToFilenameMap.find(fd_to_append);

        if (fd_iter == fdToFilenameMap.end()) {
          cout << "file doesn't exist on the disk" << endl;
          continue;
        }

        filename_to_append = fd_iter->second;
        for (int i = 0; i < NO_OF_INODES; i++) {
          if (strcmp(globalFileInodeMap[i].filename,
                     filename_to_append.c_str()) == 0) {
            temp_inode_index = globalFileInodeMap[i].inodeIndex;
            filename_to_append_found = true;
            break;
          }
        }

        if (!filename_to_append_found) {
          cout << "file doesn't exist" << endl;
          continue;
        }

        if (temp_inode_index == -1) {
          cout << "could not find file inode." << endl;
          continue;
        }

        if (globalInodeArr[temp_inode_index].mode != 3) {
          cout << "File is not opened in correct mode" << endl;
          continue;
        }

        initiateAppendOperation(temp_inode_index);
      } else if (option == 8) {
        cout << "Files currently on the disk : " << globalSuperBlock.no_of_files
             << endl;

        for (int i = 0; i < globalSuperBlock.no_of_files; i++) {
          cout << globalFileInodeMap[i].filename << endl;
        }
      } else if (option == 9) {
        cout << "List of currently opened files : " << endl;
        for (int i = 0; i < globalSuperBlock.no_of_files; i++) {
          auto file_iter = fileNameToFdMap.find(globalFileInodeMap[i].filename);

          if (file_iter == fileNameToFdMap.end() ||
              globalInodeArr[globalFileInodeMap[i].inodeIndex].mode == 0) {
            continue;
          }
          cout << file_iter->second << " ";
          cout << globalFileInodeMap[i].filename << " ";
          cout << getModeText(
                      globalInodeArr[globalFileInodeMap[i].inodeIndex].mode)
               << endl;
        }
      } else if (option == 10) {
        cout << "unmounting disk " << mount_diskname << endl;

        for (int i = 0; i < globalSuperBlock.no_of_files; i++) {
          if (globalInodeArr[globalFileInodeMap[i].inodeIndex].mode != 0) {
            cout << "disk is still in use. Please close all open files before "
                    "unmounting"
                 << endl;
	    isUnmountAllowed = false;
            break;
          }
        }

	if (!isUnmountAllowed) {
		continue;
	}

        unmountDisk();
        isMounted = false;
        cout << "unmounted disk successfully" << endl;
      } 
    }
  }

  return 0;
}
