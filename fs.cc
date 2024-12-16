#include <iostream>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <vector>
#include <sstream>

#include "fs.h"
#include "disk.h"

/* FAT end-of-chain value */
#define FAT_EOC -1

using namespace std;

typedef struct __attribute__((__packed__)) SuperBlock {
    u_int8_t sig[8]; // 8 bytes
    u_int16_t numBlocks; // 2 bytes
    u_int16_t numDataBlocks; // 2 bytes
    u_int16_t numFAT; // 2 bytes
    u_int16_t rootIndex; // 2 bytes
    u_int16_t dataIndex; // 2 bytes
    u_int8_t padding[46];
}SuperBlock;

typedef struct __attribute__((__packed__)) Root {
    char name[4];
    char type[3]; // the type is a suffix name
    //u_int16_t type; // 0 represent read a file, and 1 represent write a file.
    u_int8_t attribute;
    u_int8_t indexFirstBlock;
    u_int8_t size; // File size in blocks
}Root;

typedef struct FD {
    int id;
    int offset;
    int index; // Index of the file of the root directory
}FD;

// 定义指针结构
typedef struct pointer{
    int dnum; // 磁盘盘块号（行号）
    int bnum; // 磁盘盘块内字节位置（列号）
} pointer;

// 定义已打开文件表
typedef struct OFILE{
    char name[20];  // 文件名
    char attribute; // 文件属性
    int indexOfFirstBlock;     // 文件起始盘块号
    int length;     // 文件长度（字节数）
    int flag;       // 文件操作标志（读/写）
    pointer read;   // 读指针
    pointer write;  // 写指针
} OFILE;

// 打开文件登记表
typedef struct openfile{
    OFILE file[FS_OPEN_MAX_COUNT];
    int length; // 已打开文件的数量
} openfile;

static SuperBlock sblk;
static vector<int> fat(128, 0);
static vector<Root> root(8);

static openfile fd;
static int numFilesOpen = 0;

/**
 * sb_init - init a SuperBlock *sblk
 *
 * Extract the file information into internal (global) data struct
 * Also perform error check
 *
 * Return: -1 if error is found, 0 otherwise
*/
int sb_init()
{
    sblk.numBlocks = 128;
    sblk.numDataBlocks = 125;
    sblk.numFAT = 2;
    sblk.rootIndex = 2;
    sblk.dataIndex = 3;

    return 0;
}

/**
 * fat_init - init a file allocation table
 *
 * Extract the file allocation table into internal (global) data FAT
 * loading fat table from file
 *
 * Return: -1 if error is found, 0 otherwise
*/
int fat_init(const char *diskname)
{
    ifstream file(diskname);
    if (!file.is_open()) {
        cerr << "can't open the file" << endl;
        return -1;
    }

    string line;
    int index = 0;
    while (getline(file, line) && index < 128) {
        istringstream iss(line);
        int value;
        while (iss >> value && index < 128) {
            fat[index++] = value;
        }
    }

    file.close();

    return 0;
}

/**
 * parseDirectoryLine - read the string to the root array
 * @line: a line from the disk(File simulation)
 *
 * a line which point to a the top-level directory, for example
 * if you want to create a directory /a/b/c, we must read the directory b
 * firstly, so the root array store b directory
 *
 * Return: -1 if read success. 0 otherwise.
*/
void parseDirectoryLine(const string& line) {
    istringstream iss(line);
    string token;
    int rootIndex = 0;

    while (iss >> token) {
        if (token == "$") {
            strcpy(root[rootIndex].name, "$\0\0\0");
            iss >> token;
            strcpy(root[rootIndex].type, "$\0\0");
            int attribute, indexFirstBlock, size;
            if (iss >> attribute >> indexFirstBlock >> size) {
                root[rootIndex].attribute = static_cast<uint8_t>(attribute);
                root[rootIndex].indexFirstBlock = static_cast<uint8_t>(indexFirstBlock);
                root[rootIndex].size = static_cast<uint8_t>(size);
                rootIndex++;
            } else {
                std::cerr << "Invalid directory entry format in line" << std::endl;
                break;
            }
            continue;
        }

        if (rootIndex < 8) {
            string name = token.substr(0, 3);
            // because the pre name's length maybe larger than this
            // take a example, pre: xy, now: a, if no init
            // it will be ay
            fill(begin(root[rootIndex].name), end(root[rootIndex].name), '\0');
            copy(name.begin(), name.end(), root[rootIndex].name);

            iss >> token;
            string type = token.substr(0, 2);
            fill(begin(root[rootIndex].type), end(root[rootIndex].type), '\0');
            copy(type.begin(), type.end(), root[rootIndex].type);

            int attribute, indexFirstBlock, size;
            if (iss >> attribute >> indexFirstBlock >> size) {
                root[rootIndex].attribute = static_cast<uint8_t>(attribute);
                root[rootIndex].indexFirstBlock = static_cast<uint8_t>(indexFirstBlock);
                root[rootIndex].size = static_cast<uint8_t>(size);
                rootIndex++;
            } else {
                std::cerr << "Invalid directory entry format in line" << std::endl;
                break;
            }
        } else {
            std::cerr << "Exceeded maximum directory entries" << std::endl;
            break;
        }
    }
}

/**
 * root_init - read the specific disk block to the root array
 * @diskname: disk name
 * @index: the index of the disk block
 *
 * read the line to the root array, init the root directory
 *
 * Return: -1 if read failed, 0 otherwise
*/
int root_init(const char *diskname, u_int8_t index)
{
    ifstream file(diskname);
    string line;

    // read the root directory
    for (int i = 0; i < index; ++i) {
        std::getline(file, line);
    }

    if (getline(file, line)) {
        parseDirectoryLine(line);
    }

    return 0;
}

// int fd_init()
// {
//     for (int i = 0; i < FS_OPEN_MAX_COUNT; i++) {
//         filedes[i].id = -1;
//         filedes[i].offset = 0;
//         filedes[i].index = -1;
//     }

//     return 0;
// }

int fs_mount(const char *diskname)
{
    if (block_disk_open(diskname) != 0) {
        return -1;
    }

    sb_init();
    fat_init(diskname);
    root_init(diskname, 2);

    //fd_init();

    return 0;
}

/**
 * saveFatToFile - write the fat array back to the disk(file)
 * @diskname: the disk's name
 *
 * At the end of the program, you must write back the data to the disk
 * it can be persistent storage.
 *
 * Return: -1 if write back failed. 0 otherwise.
*/
void saveFatToFile(const string& filename) {
    // read all the content from the file
    ifstream fileIn(filename);
    vector<string> lines;
    string line;
    while (getline(fileIn, line)) {
        lines.push_back(line);
    }
    fileIn.close();

    // ready for the new content, write fat to the first two lines
    ostringstream fatData;
    for (int i = 0; i < 64; ++i) {
        fatData << fat[i];
        if (i < 63) fatData << " ";
    }
    string firstLine = fatData.str();
    fatData.str("");
    fatData.clear();

    for (int i = 64; i < 128; ++i) {
        fatData << fat[i];
        if (i < 127) fatData << " ";
    }
    string secondLine = fatData.str();

    // use new fat data substitude for the old fat
    if (lines.size() >= 2) {
        lines[0] = firstLine;
        lines[1] = secondLine;
    } else {
        // if data line < 2, supplement blank lines, (but it will never happen)
        lines.resize(2, "");
        lines[0] = firstLine;
        lines[1] = secondLine;
    }

    // write back all the content to the file
    ofstream fileOut(filename);
    for (const auto& l : lines) {
        fileOut << l; // don need extra "\n"
        if (&l != &lines.back()) // add the "\n" if it is not the last line
            fileOut << "\n";
    }
    fileOut.close();
}

int fs_umount(const char *diskname)
{
    if (block_disk_close() != 0) {
        return -1;
    }

    saveFatToFile(diskname);

    return 0;
}

/**
 * num_free_fat - calculate how much free block it have
 *
 * Return: the number of free block
*/
int num_free_fat()
{
    int count = 0;
    for (int i = 0; i < sblk.numDataBlocks; i++) {
        if (fat[i] == 0)
            count++;
    }

    return count;
}

/**
 * num_free_rdir - Return how much free root directory it have
 *
 * Return: the number of free root directory
*/
int num_free_rdir()
{
    int count = 0;
    for (int i = 1; i < FS_FILE_MAX_COUNT; i++) {
        if ((char)*(root[i].name) == '\0')
            count++;
    }

    return count;
}

int fs_info(void)
{
    if (block_disk_count() == -1) {
        return -1;
    }

    cout << "FS info:" << endl;
    cout << "total_blk_count = " << sblk.numBlocks << endl;
    cout << "fat_blk_count = " << sblk.numFAT << endl;
    cout << "data_blk_count = " << sblk.numDataBlocks << endl;
    cout << "rdir_blk = " << sblk.rootIndex << endl;
    cout << "data_blk = " << sblk.dataIndex << endl;

    cout << "fat_free = " << num_free_fat() << endl;
    cout << "rdir_free = " << num_free_rdir() << endl;

    for (int i = 0; i < 8; ++i) {
        if (root[i].name[0] == '\0') continue;  // 跳过空目录项
        std::cout << "File " << i << ": " << root[i].name << ", "
                  << "Type: " << root[i].type << ", "
                  << "Attribute: " << static_cast<int>(root[i].attribute) << ", "
                  << "IndexFirstBlock: " << static_cast<int>(root[i].indexFirstBlock) << ", "
                  << "Size: " << static_cast<int>(root[i].size) << " blocks" << std::endl;
    }
    return 0;
}

/**
 * formatRoot - transfer the root directory to the string
 * @root: the root array
 *
 * change to a string
 *
 * Return: the string of the transfered root dir
*/
string formatRoot(const Root& root)
{
    ostringstream oss;
    oss << root.name << " " << root.type << " "
        << static_cast<int>(root.attribute) << " "
        << static_cast<int>(root.indexFirstBlock) << " "
        << static_cast<int>(root.size);
    return oss.str();
}

/**
 * writeDirToDisk - write root array to Specific lines
 * @roots: the root array, that store directory structure
 * @filename: the diskname
 * @linetoreplace: the line to be replaced
 *
 * when the directory is changed, you must write it to the disk timely
 * because you have to keep reading it later, such as the root_init function
 * or when you create a new directory, you must init the disk timely.
*/
void writeDirToDisk(const vector<Root>& roots, const string& filename, int lineToReplace)
{
    ifstream inputFile(filename);
    vector<string> lines;
    string line;

    // read all the file content to lines
    while (getline(inputFile, line)) {
        lines.push_back(line);
    }
    inputFile.close();

    // change subDir array to a string
    ostringstream oss;
    for (const auto& root : roots) {
        oss << formatRoot(root) << " ";
    }
    string rootLine = oss.str();

    // replace
    if (lineToReplace < lines.size()) {
        lines[lineToReplace] = rootLine;
    } else {
        // fill with '\n'
        while (lines.size() <= lineToReplace) {
            lines.push_back("");
        }
        lines[lineToReplace] = rootLine;
    }

    // write back file
    ofstream outputFile(filename);
    for (const auto& l : lines) {
        outputFile << l << "\n";
    }
}


/**
 * valid_filename - judge that a filename is valid
 * @filename: File name
 *
 * Return: -1 if the filename is invalid, 0 otherwise.
*/
int valid_name(const string &filename)
{
    char bad_chars[] = "!@#%^*|~&";
    for (int i = 0; i < strlen(bad_chars); i++) {
        if (filename.find(bad_chars) != string::npos)
            return -1;
    } // check valid filename

    return 0;
}

/**
 * find_empty_fat - find the empty block to use
 *
 * Return: -1 if no space to allocate. 0 otherwise
*/
int find_empty_fat()
{
    u_int8_t itr = 3;
    while (itr < sblk.numDataBlocks) {
        if (fat[itr] == 0) {
            return itr;
        }
        itr++;
    }

    return -1; // no space
}

/**
 * splitPath - split the pathname through '/'
 * @path: a string path: /a/b/c
 *
 * split the path to the file name, for example, /a/b/c
 * split into a array that ['a', 'b', 'c'].
 *
 * Return: a vector<string>
*/
vector<string> splitPath(const string &path) {
    vector<std::string> tokens;
    istringstream stream(path);
    string token;

    // Use '/' as delimiter to split path
    while (std::getline(stream, token, '/')) {
        if (!token.empty()) {  // Ignore empty tokens
            tokens.push_back(token);
        }
    }
    return tokens;
}

/**
 * splitSuffix - split the file name through '.'
 * @path: a file name, like 'a.txt'
 *
 * split the file name to the file name and suffix, for example, a.txt
 * split into a array that ['a', 'txt'].
 *
 * Return: a vector<string>
*/
vector<string> splitSuffix(string filename)
{
    size_t dotPos = filename.find_last_of('.');
    vector<string> result;

    if (dotPos != string::npos) {
        string name = filename.substr(0, dotPos);
        string extension = filename.substr(dotPos + 1);

        result.push_back(name);
        result.push_back(extension);
    } else {
        result.push_back(filename);
        cout << "No extension found in the filename." << endl;
    }

    return result;
}

/**
 * update_block - write the new data to disk block
 * @diskname: disk name
 * @line_number: the specific disk block
 * @new_data: the block's new data
 *
 * write the data to the block, and cover the old data
*/
void update_block(const string& diskname, int line_number, const string& new_data) {
    if (line_number < 0 || line_number >= FS_DISK_MAX) {
        cerr << "Error: Invalid line number. Must be between 0 and " << FS_DISK_MAX - 1 << "." << endl;
        return;
    }

    // read the file, store to the vector<string>
    ifstream infile(diskname);
    vector<string> lines;
    string line;

    while (getline(infile, line)) {
        lines.push_back(line.empty() ? "" : line);
    }
    infile.close();

    // modify the content
    if (line_number < lines.size()) {
        lines[line_number] = new_data;
    }

    while (lines.size() <= line_number) {
        lines.push_back(""); // fill with blank line
    }
    lines[line_number] = new_data;

    // write back file
    ofstream outfile(diskname);
    for (const auto& l : lines) {
        outfile << l << "\n";
    }
    outfile.close();
}

int create_file(const string &pathname, char attribute)
{
    if (valid_name(pathname) == -1)
        return -1;
    root_init("disk.txt", 2);

    vector<string> tokens = splitPath(pathname);
    int k = 0; // tokens 's index
    bool flag = false;
    int current_index = 2;

    while (tokens.size() - k > 1) {
        for (int i = 0; i < FS_FILE_MAX_COUNT; i++) {
            if (root[i].name == tokens[k]) { // find it
                k++;
                current_index = root[i].indexFirstBlock;
                root_init("disk.txt", root[i].indexFirstBlock);
                flag = true;
                break;
            }
        }
        if (flag) {
            flag = false;
            continue;
        } else {
            cerr << "can't find the sub Directory" << endl;
            return -1;
        }
    }

    vector<string> nameAndSuffix = splitSuffix(tokens[k]);

    // if the name is exists
    for (int i = 0; i < FS_FILE_MAX_COUNT; i++) {
        if (nameAndSuffix[0] == root[i].name) {
            cerr << "the file is exists" << endl;
            return -1;
        }
    }

    if (tokens.size() - k == 1) {
        for (int i = 0; i < FS_FILE_MAX_COUNT; i++) {
            if(root[i].name[0] == '$') {
                strncpy(root[i].name, nameAndSuffix[0].c_str(), sizeof(root[i].name) - 1);
                root[i].name[sizeof(root[i].name) - 1] = '\0';
                if (nameAndSuffix.size() == 2) {
                    strncpy(root[i].type, nameAndSuffix[1].c_str(), sizeof(root[i].type) - 1);
                    root[i].type[sizeof(root[i].type) - 1] = '\0';
                } else {
                    memset(root[i].type, '\0', sizeof(root[i].type));
                }
                root[i].attribute = attribute;
                root[i].indexFirstBlock = find_empty_fat();
                fat[root[i].indexFirstBlock] = FAT_EOC;
                root[i].size = 1;
                writeDirToDisk(root, "disk.txt", current_index);
                string block_data(BLOCK_SIZE, '#');
                update_block("disk.txt", root[i].indexFirstBlock, block_data);
                cout << "file create success!" << endl;
                root_init("disk.txt", 2);
                return 0;
            }
        }
        cerr << "the dir is full" << endl;
        return -1;
    }

    return 0;
}

int open_file(const string &filename, int flag)
{
    if (valid_name(filename) == -1) {
        return -1;
    }
    root_init("disk.txt", 2);

    // if the open file count > FS_OPEN_MAX_COUNT
    if (fd.length >= FS_OPEN_MAX_COUNT) {
        return -1;
    }

    vector<string> tokens = splitPath(filename);
    int k = 0; // tokens 's index
    bool flag1 = false;
    int current_index = 2;

    while (tokens.size() - k > 1) {
        for (int i = 0; i < FS_FILE_MAX_COUNT; i++) {
            if (root[i].name == tokens[k]) { // find it
                k++;
                current_index = root[i].indexFirstBlock;
                root_init("disk.txt", root[i].indexFirstBlock);
                flag1 = true;
                break;
            }
        }
        if (flag1) {
            flag1 = false;
            continue;
        } else {
            cerr << "can't find the sub Directory" << endl;
            return -1;
        }
    }

    vector<string> nameAndSuffix = splitSuffix(tokens[k]);

    int index = 0;
    for (int i = 0; i < FS_FILE_MAX_COUNT; i++) {
        if (nameAndSuffix[0] == root[i].name) {
            if (root[i].attribute == 8) {
                cerr << "it is a directory, not a file" << endl;
                // it is no a file, it is a directory
                return -1;
            }else {
                index = i;
                break;
            }
        }
    }

    for (int i = 0; i < FS_OPEN_MAX_COUNT; i++) {
        if (fd.file[i].name[0] == '\0') {
            fd.length++;
            strcpy(fd.file[i].name, root[index].name);
            fd.file[i].attribute = root[index].attribute;
            fd.file[i].indexOfFirstBlock = root[index].indexFirstBlock;
            fd.file[i].length = root[index].size;
            fd.file[i].flag = flag; // 0 read or 1 write
            fd.file[i].read.dnum = root[index].indexFirstBlock;
            fd.file[i].read.bnum = 0;
            fd.file[i].write.dnum = root[index].indexFirstBlock;
            fd.file[i].write.bnum = 0;
            return 0;
        }
    }

    return -1;
}

int read_file(const string &filename, int read_length)
{
    // open the disk
    ifstream disk("disk.txt", ios::in);
    if (!disk.is_open()) {
        std::cerr << "can't open the disk" << std::endl;
        return false;
    }

    // invalid name
    if (valid_name(filename) == -1) {
        return -1;
    }
    root_init("disk.txt", 2);

    vector<string> tokens = splitPath(filename);
    int k = 0; // tokens 's index
    bool flag1 = false;
    int current_index = 2;

    while (tokens.size() - k > 1) {
        for (int i = 0; i < FS_FILE_MAX_COUNT; i++) {
            if (root[i].name == tokens[k]) { // find it
                k++;
                current_index = root[i].indexFirstBlock;
                root_init("disk.txt", root[i].indexFirstBlock);
                flag1 = true;
                break;
            }
        }
        if (flag1) {
            flag1 = false;
            continue;
        } else {
            cerr << "can't find the sub Directory" << endl;
            return -1;
        }
    }

    vector<string> nameAndSuffix = splitSuffix(tokens[k]);

    int index = 0;
    flag1 = false;
    // find the file
    for (int i = 0; i < FS_OPEN_MAX_COUNT; i++) {
        if (nameAndSuffix[0] == fd.file[i].name) {
            index = i;
            flag1 = true;
            break;
        }
    }
    if(!flag1)
        open_file(filename, 0);

    string line; // store the line we read currently
    int current_line = 0; // the current line number
    int remaining_length = read_length; // the remain length don't read
    string data; // the data we read
    while (getline(disk, line)) {
        if (current_line >= fd.file[index].read.dnum) {
            for (int i = fd.file[index].read.bnum; i < BLOCK_SIZE; i++) {
                fd.file[index].read.bnum++;
                if (line[i] == '#') {  // if encounter '#', stop
                    remaining_length = 0;
                    break;
                }
                data.push_back(line[i]);
                --remaining_length;
                if (fd.file[index].read.bnum >= 64) {
                    if (fat[fd.file[index].read.dnum] != -1) {
                        fd.file[index].read.dnum = fat[fd.file[index].read.dnum];
                    }
                    fd.file[index].read.bnum = 0;
                }
                if (remaining_length == 0) break; // up to the read_length
            }
        }

        if (remaining_length == 0) break; // up to the read_length
        ++current_line;
    }
    cout << data << endl;

    return 0;
}

// void replace_line(const string& filename, int line_number, const string& new_content) {
//     ifstream file_in(filename);
//     if (!file_in.is_open()) {
//         std::cerr << "can't open the file" << filename << std::endl;
//         return;
//     }

//     // read the disk's content
//     vector<string> lines;
//     string line;
//     while (getline(file_in, line)) {
//         lines.push_back(line);
//     }
//     file_in.close();

//     // check the block index is legal
//     if (line_number < 0 || line_number >= static_cast<int>(lines.size())) {
//         cerr << "the line number over the broad" << endl;
//         return;
//     }

//     // replace the content
//     lines[line_number] = new_content;

//     // write back disk
//     ofstream file_out(filename);
//     if (!file_out.is_open()) {
//         cerr << "can't open the file" << filename << endl;
//         return;
//     }

//     for (const auto& l : lines) {
//         file_out << l << "\n";
//     }
//     file_out.close();
// }

int write_file(const string &filename, const string buffer, int write_length)
{
    // open the disk
    fstream disk("disk.txt", ios::in | ios::out);
    if (!disk.is_open()) {
        cerr << "can't open the disk" << std::endl;
        return false;
    }

    // invalid name
    if (valid_name(filename) == -1) {
        return -1;
    }
    root_init("disk.txt", 2);

    vector<string> tokens = splitPath(filename);
    int k = 0; // tokens 's index
    bool flag1 = false;
    int current_index = 2;

    while (tokens.size() - k > 1) {
        for (int i = 0; i < FS_FILE_MAX_COUNT; i++) {
            if (root[i].name == tokens[k]) { // find it
                k++;
                current_index = root[i].indexFirstBlock;
                root_init("disk.txt", root[i].indexFirstBlock);
                flag1 = true;
                break;
            }
        }
        if (flag1) {
            flag1 = false;
            continue;
        } else {
            cerr << "can't find the sub Directory" << endl;
            return -1;
        }
    }

    vector<string> nameAndSuffix = splitSuffix(tokens[k]);

    int index = 0;
    flag1 = false;
    // find the file
    for (int i = 0; i < FS_OPEN_MAX_COUNT; i++) {
        if (nameAndSuffix[0] == fd.file[i].name) {
            index = i;
            flag1 = true;
            break;
        }
    }
    if(!flag1)
        open_file(filename, 1);

    int dir_index = 0;
    for (int i = 0; i < FS_FILE_MAX_COUNT; i++) {
        if (nameAndSuffix[0] == root[i].name) {
            dir_index = i;
            break;
        }
    }

    string line; // store the line we read currently
    int current_line = 0; // the current line number
    int remaining_length = write_length; // the remain length don't read
    int buffer_index = 0; // the index of buffer
    while (getline(disk, line)) {
        if (current_line >= fd.file[index].write.dnum) {
            for (int i = fd.file[index].write.bnum; i < BLOCK_SIZE; i++) {
                line[fd.file[index].write.bnum] = buffer[buffer_index];
                fd.file[index].write.bnum++;
                buffer_index++;
                remaining_length--;
                if (remaining_length == 0 || buffer_index == buffer.size()) break;
                // the block space isn't enough
                if (fd.file[index].write.bnum >= 64) {
                    // write current data to the block
                    update_block("disk.txt", fd.file[index].write.dnum, line);
                    if (fat[fd.file[index].write.dnum] != -1) {
                        fd.file[index].write.dnum = fat[fd.file[index].write.dnum];
                        fd.file[index].write.bnum = 0;
                    } else {
                        int empty_block_index = find_empty_fat();
                        fat[fd.file[index].write.dnum] = empty_block_index;
                        fat[empty_block_index] = FAT_EOC;
                        string block_data(BLOCK_SIZE, '#');
                        update_block("disk.txt", empty_block_index, block_data);
                        root[dir_index].size++;
                        writeDirToDisk(root, "disk.txt", current_index);
                        fd.file[index].write.dnum = empty_block_index;
                        fd.file[index].write.bnum = 0;
                        break;
                    }
                }
            }
        }
        if (remaining_length == 0 || buffer_index == buffer.size()) break; // 数据读取完成
        ++current_line;
    }
    line[fd.file[index].write.bnum] = '#';
    update_block("disk.txt", fd.file[index].write.dnum, line);
    cout << "write success" << endl;

    return 0;
}

int close_file(const string &filename)
{
    // invalid name
    if (valid_name(filename) == -1) {
        return -1;
    }
    root_init("disk.txt", 2);

    vector<string> tokens = splitPath(filename);
    int k = 0; // tokens 's index
    bool flag1 = false;
    int current_index = 2;

    while (tokens.size() - k > 1) {
        for (int i = 0; i < FS_FILE_MAX_COUNT; i++) {
            if (root[i].name == tokens[k]) { // find it
                k++;
                current_index = root[i].indexFirstBlock;
                root_init("disk.txt", root[i].indexFirstBlock);
                flag1 = true;
                break;
            }
        }
        if (flag1) {
            flag1 = false;
            continue;
        } else {
            cerr << "can't find the sub Directory" << endl;
            return -1;
        }
    }

    vector<string> nameAndSuffix = splitSuffix(tokens[k]);

    for (int i = 0; i < FS_OPEN_MAX_COUNT; i++) {
        if (nameAndSuffix[0] == fd.file[i].name) {
            fill(begin(fd.file[i].name), end(fd.file[i].name), '\0');
            fd.file[i].attribute = 0;
            fd.file[i].indexOfFirstBlock = 0;
            fd.file[i].length = 0;
            fd.file[i].flag = -1; // 0 read or 1 write
            fd.file[i].read.dnum = 0;
            fd.file[i].read.bnum = 0;
            fd.file[i].write.dnum = 0;
            fd.file[i].write.bnum = 0;

            cout << "close success" << endl;
            return 0;
        }
    }

    cout << "can't find the file" << endl;
    return -1;
}

int delete_file(const string &filename)
{
    // invalid name
    if (valid_name(filename) == -1) {
        return -1;
    }
    root_init("disk.txt", 2);

    vector<string> tokens = splitPath(filename);
    int k = 0; // tokens 's index
    bool flag1 = false;
    int current_index = 2;

    while (tokens.size() - k > 1) {
        for (int i = 0; i < FS_FILE_MAX_COUNT; i++) {
            if (root[i].name == tokens[k]) { // find it
                k++;
                current_index = root[i].indexFirstBlock;
                root_init("disk.txt", root[i].indexFirstBlock);
                flag1 = true;
                break;
            }
        }
        if (flag1) {
            flag1 = false;
            continue;
        } else {
            cerr << "can't find the sub Directory" << endl;
            return -1;
        }
    }

    vector<string> nameAndSuffix = splitSuffix(tokens[k]);

    for (int i = 0; i < FS_OPEN_MAX_COUNT; i++) {
        if (nameAndSuffix[0] == fd.file[i].name) {
            cerr << "the file open, can't delete" << endl;
            return -1;
        }
    }

    int dir_index = 0;
    flag1 = false;
    for (int i = 0; i < FS_FILE_MAX_COUNT; i++) {
        if (nameAndSuffix[0] == root[i].name) {
            int linked_index = root[i].indexFirstBlock;
            int linked_index_copy = linked_index;
            int tmp = 0;
            while (linked_index != -1) {
                tmp = linked_index;
                linked_index = fat[linked_index];
                fat[tmp] = 0;
            }
            strncpy(root[i].name, "$\0\0\0", sizeof(root[i].name));
            strncpy(root[i].type, "$\0\0", sizeof(root[i].type));
            root[i].attribute = 0;
            root[i].indexFirstBlock = 0;
            root[i].size = 0;
            writeDirToDisk(root, "disk.txt", current_index);

            cout << "file delete success" << endl;
            return 0;
        }
    }

   cerr << "can't find the file" << endl;
   return -1;
}

int typefile(const string &filename)
{
    // open the disk
    ifstream disk("disk.txt", ios::in);
    if (!disk.is_open()) {
        std::cerr << "can't open the disk" << std::endl;
        return false;
    }

    // invalid name
    if (valid_name(filename) == -1) {
        return -1;
    }
    root_init("disk.txt", 2);

    vector<string> tokens = splitPath(filename);
    int k = 0; // tokens 's index
    bool flag1 = false;
    int current_index = 2;

    while (tokens.size() - k > 1) {
        for (int i = 0; i < FS_FILE_MAX_COUNT; i++) {
            if (root[i].name == tokens[k]) { // find it
                k++;
                current_index = root[i].indexFirstBlock;
                root_init("disk.txt", root[i].indexFirstBlock);
                flag1 = true;
                break;
            }
        }
        if (flag1) {
            flag1 = false;
            continue;
        } else {
            cerr << "can't find the sub Directory" << endl;
            return -1;
        }
    }

    vector<string> nameAndSuffix = splitSuffix(tokens[k]);

    for (int i = 0; i < FS_OPEN_MAX_COUNT; i++) {
        if (nameAndSuffix[0] == fd.file[i].name) {
            cerr << "the file is opened, can't show." << endl;
            return -1;
        }
    }

    int dir_index = 0;
    flag1 = false;
    for (int i = 0; i < FS_FILE_MAX_COUNT; i++) {
        if (nameAndSuffix[0] == root[i].name) {
            dir_index = i;
            flag1 = true;
            break;
        }
    }
    if (!flag1) {
        cerr << "the file is no exist." << endl;
        return -1;
    }

    int linked_index = root[dir_index].indexFirstBlock;
    string line; // store the line we read currently
    int current_line = 0; // the current line number
    string data; // the data we read
    while (getline(disk, line)) {
        if (current_line >= linked_index) {
            cout << line << endl;
            if (fat[linked_index] != -1)
                linked_index = fat[linked_index];
            else
                break;
        }
        ++current_line;
    }

    cout << "show the file success" << endl;
    return 0;
}

int change(const string &filename, int attribute)
{
    // open the disk
    ifstream disk("disk.txt", ios::in);
    if (!disk.is_open()) {
        std::cerr << "can't open the disk" << std::endl;
        return false;
    }

    // invalid name
    if (valid_name(filename) == -1) {
        return -1;
    }
    root_init("disk.txt", 2);

    vector<string> tokens = splitPath(filename);
    int k = 0; // tokens 's index
    bool flag1 = false;
    int current_index = 2;

    while (tokens.size() - k > 1) {
        for (int i = 0; i < FS_FILE_MAX_COUNT; i++) {
            if (root[i].name == tokens[k]) { // find it
                k++;
                current_index = root[i].indexFirstBlock;
                root_init("disk.txt", root[i].indexFirstBlock);
                flag1 = true;
                break;
            }
        }
        if (flag1) {
            flag1 = false;
            continue;
        } else {
            cerr << "can't find the sub Directory" << endl;
            return -1;
        }
    }

    vector<string> nameAndSuffix = splitSuffix(tokens[k]);

    for (int i = 0; i < FS_OPEN_MAX_COUNT; i++) {
        if (nameAndSuffix[0] == fd.file[i].name) {
            cerr << "the file is opened, can't show." << endl;
            return -1;
        }
    }

    for (int i = 0; i < FS_FILE_MAX_COUNT; i++) {
        if (nameAndSuffix[0] == root[i].name) {
            root[i].attribute = attribute;
            writeDirToDisk(root, "disk.txt", current_index);

            cout << "change success" << endl;
            return 0;
        }
    }

    cerr << "can't find the file" << endl;
    return -1;
}

int md(const string &pathdir)
{
    if (valid_name(pathdir) == -1)
        return -1;
    root_init("disk.txt", 2);

    int k = 0; // tokens 's index
    bool flag = false;
    int current_index = 2;
    vector<string> tokens = splitPath(pathdir);

    while (tokens.size() - k > 1) {
        for (int i = 0; i < FS_FILE_MAX_COUNT; i++) {
            if (root[i].name == tokens[k] && root[i].attribute == 8) { // find it
                k++;
                current_index = root[i].indexFirstBlock;
                root_init("disk.txt", root[i].indexFirstBlock);
                flag = true;
                break;
            }
        }
        if (flag) {
            flag = false;
            continue;
        } else {
            cerr << "can't find the sub Directory" << endl;
            return -1;
        }
    }

    // if the name is exists
    for (int i = 0; i < FS_FILE_MAX_COUNT; i++) {
        if (tokens[k] == root[i].name) {
            cerr << "the directory is exists" << endl;
            return -1;
        }
    }

    if (tokens.size() - k == 1) {
        for (int i = 0; i < FS_FILE_MAX_COUNT; i++) {
            if(root[i].name[0] == '$') {
                strncpy(root[i].name, tokens[k].c_str(), sizeof(root[i].name) - 1);
                root[i].name[sizeof(root[i].name) - 1] = '\0';
                strncpy(root[i].type, "$\0\0", sizeof(root[i].type));
                root[i].attribute = 8;
                root[i].indexFirstBlock = find_empty_fat();
                fat[root[i].indexFirstBlock] = FAT_EOC;
                root[i].size = 1;
                writeDirToDisk(root, "disk.txt", current_index);
                vector<Root> subDir = {
                    {"$", "$", 0, 0, 0},
                    {"$", "$", 0, 0, 0},
                    {"$", "$", 0, 0, 0},
                    {"$", "$", 0, 0, 0},
                    {"$", "$", 0, 0, 0},
                    {"$", "$", 0, 0, 0},
                    {"$", "$", 0, 0, 0},
                    {"$", "$", 0, 0, 0}
                };
                writeDirToDisk(subDir, "disk.txt", root[i].indexFirstBlock);
                cout << "directory create success!" << endl;
                root_init("disk.txt", 2);
                return 0;
            }
        }
        cerr << "the dir is full" << endl;
        return -1;
    }

    return 0;
}

int dir(const string &pathdir)
{
    if (valid_name(pathdir) == -1)
        return -1;
    root_init("disk.txt", 2);

    if (pathdir == "/") {
        cout << left << setw(10) << "name"
                << setw(10) << "type"
                << setw(12) << "attribute"
                << setw(18) << "indexoffirstblock"
                << setw(10) << "size" << endl;
        cout << string(60, '-') << endl;

        for (int j = 0; j < FS_FILE_MAX_COUNT; j++) {
            if (root[j].name[0] != '$') {
                cout << left << setw(10) << root[j].name
                << setw(10) << root[j].type
                << setw(12) << static_cast<int>(root[j].indexFirstBlock)
                << setw(18) << static_cast<int>(root[j].attribute)
                << setw(10) << static_cast<int>(root[j].size)
                << endl;
            }
        }
        cout << "print success" << endl;
        return 0;
    }

    int k = 0; // tokens 's index
    bool flag = false;
    int current_index = 2;
    vector<string> tokens = splitPath(pathdir);

    while (tokens.size() - k > 1) {
        for (int i = 0; i < FS_FILE_MAX_COUNT; i++) {
            if (root[i].name == tokens[k]) { // find it
                k++;
                current_index = root[i].indexFirstBlock;
                root_init("disk.txt", root[i].indexFirstBlock);
                flag = true;
                break;
            }
        }
        if (flag) {
            flag = false;
            continue;
        } else {
            cerr << "can't find the sub Directory" << endl;
            return -1;
        }
    }

    // find the dir
    for (int i = 0; i < FS_FILE_MAX_COUNT; i++) {
        if (tokens[k] == root[i].name) {
            current_index = root[i].indexFirstBlock;
            root_init("disk.txt", root[i].indexFirstBlock);
            cout << "name  type   attribute  indexoffirstblock  size " << endl;
            for (int j = 0; j < FS_FILE_MAX_COUNT; j++) {
                if (root[j].name[0] != '$') {
                    cout << root[j].name << "      "
                    << root[j].type << "      "
                    << static_cast<int>(root[j].indexFirstBlock) << "             "
                    << static_cast<int>(root[j].attribute) << "               "
                    << static_cast<int>(root[j].size)
                    << endl;
                }

            }
            cout << "print success" << endl;
        }
    }

    cerr << "can't find the dir" << endl;
    return -1;
}

int rd(const string &pathdir)
{
    if (valid_name(pathdir) == -1)
        return -1;
    root_init("disk.txt", 2);

    int k = 0; // tokens 's index
    bool flag = false;
    int current_index = 2;
    vector<string> tokens = splitPath(pathdir);

    while (tokens.size() - k > 1) {
        for (int i = 0; i < FS_FILE_MAX_COUNT; i++) {
            if (root[i].name == tokens[k]) { // find it
                k++;
                current_index = root[i].indexFirstBlock;
                root_init("disk.txt", root[i].indexFirstBlock);
                flag = true;
                break;
            }
        }
        if (flag) {
            flag = false;
            continue;
        } else {
            cerr << "can't find the sub Directory" << endl;
            return -1;
        }
    }

    // find the dir
    for (int i = 0; i < FS_FILE_MAX_COUNT; i++) {
        if (tokens[k] == root[i].name) {
            int pre_index = current_index;
            current_index = root[i].indexFirstBlock;
            root_init("disk.txt", root[i].indexFirstBlock);
            for(int j = 0; j < FS_FILE_MAX_COUNT; j++) {
                if (root[j].name[0] != '$') {
                    cerr << "it is no a empty dir" << endl;
                    return -1;
                }
            }
            vector<Root> subDir = {
                {"$", "$", 0, 0, 0},
                {"$", "$", 0, 0, 0},
                {"$", "$", 0, 0, 0},
                {"$", "$", 0, 0, 0},
                {"$", "$", 0, 0, 0},
                {"$", "$", 0, 0, 0},
                {"$", "$", 0, 0, 0},
                {"$", "$", 0, 0, 0}
            };
            writeDirToDisk(subDir, "disk.txt", current_index);
            root_init("disk.txt", pre_index);
            for(int j = 0; j < FS_FILE_MAX_COUNT; j++) {
                if (root[j].name == tokens[k]) {
                    strncpy(root[i].name, "$\0\0\0", sizeof(root[i].name));
                    strncpy(root[i].type, "$\0\0", sizeof(root[i].type));
                    root[i].attribute = 0;
                    root[i].indexFirstBlock = 0;
                    root[i].size = 0;
                    writeDirToDisk(root, "disk.txt", pre_index);
                }
            }
            cout << "delete dir success" << endl;
            return 0;
        }
    }
    cerr << "can't find the dir." << endl;
    return -1;
}