#ifndef _FS_H
#define _FS_H

#include <cstddef>
#include <string>

using namespace std;
/** Maximum filename length (including the NULL character) */
#define FS_FILENAME_LEN 3

/** Maximum number of files in the root directory */
#define FS_FILE_MAX_COUNT 8

/** Maximum number of open files */
#define FS_OPEN_MAX_COUNT 5

/** Maximum column of the disk (by blocks) */
#define FS_DISK_MAX 128

/**
 * fs_mount - Mount a file system
 * @diskname: Name of the virtual disk file
 *
 * Open the virtual disk file @diskname and mount the file system that it
 * contains. A file system needs to be mounted before files can be read from it
 * with fs_read() or written to it with fs_write().
 *
 * Return: -1 if virtual disk file @diskname cannot be opened, or if no valid
 * file system can be located. 0 otherwise.
*/
int fs_mount(const char* diskname);

/**
 * fs_umount - Unmount the file system
 * @diskname: Name of the virtual disk file
 *
 * Unmount the currently mounted file system and close the associated 
 * virtual disk file.
 *
 * Return: -1 if the file system cannot be unmounted or if no file system 
 * is mounted. 0 otherwise.
*/
int fs_umount(const char* diskname);

/**
 * fs_info - show information about file system
 *
 * Display some information about the currently mounted file system.
 *
 * Return: -1 if no underlying virtual disk was opened. 0 otherwise.
*/
int fs_info(void);

/**
 * create_file - Create a new file
 * @filename: File name
 * @attribute: File attribute
 * 
 * Create a new and empty file named @filename in the root directory of the
 * mounted file system. String @filename must be NULL_terminated and its total
 * length cannot exceed %FS_FILENAME_LEN characters (including the NULL
 * character).
 *
 * Return: -1 if @filename is invalid. if a file named @filename already exist,
 * or if string @filename is too long, or if the root directory already contains
 * %FS_FILE_MAX_COUNT files. 0 otherwise.
*/
int create_file(const string &pathname, char attribute);

/**
 * open_file - Open a file
 * @filename: File name
 * @flag: Access mode (e.g., read, write, or both)
 *
 * Open an existing file for reading, writing, or both. 
 * The file must exist in the file system.
 *
 * Return: -1 if the file does not exist or if the maximum number of 
 * open files is exceeded. File descriptor otherwise.
*/
int open_file(const string &filename, int flag);

/**
 * read_file - Read data from a file
 * @filename: File name
 * @read_length: Number of bytes to read
 *
 * Read up to @read_length bytes from the specified file. 
 * The file must be opened for reading.
 *
 * Return: -1 if the file is not open or if the read operation fails.
 * Number of bytes read otherwise.
*/
int read_file(const string &filename, int read_length);

/**
 * write_file - Write data to a file
 * @filename: File name
 * @buffer: Data to write
 * @write_length: Number of bytes to write
 *
 * Write up to @write_length bytes from the @buffer to the specified file. 
 * The file must be opened for writing.
 *
 * Return: -1 if the file is not open, the write operation fails, 
 * or there is insufficient space. Number of bytes written otherwise.
*/
int write_file(const string &filename, const string buffer, int write_length);

/**
 * close_file - Close an open file
 * @filename: File name
 *
 * Close the specified file that was previously opened.
 *
 * Return: -1 if the file is not open. 0 otherwise.
*/
int close_file(const string &filename);

/**
 * delete_file - Delete a file
 * @filename: File name
 *
 * Delete the specified file from the mounted file system.
 *
 * Return: -1 if the file does not exist or cannot be deleted. 0 otherwise.
*/
int delete_file(const string &filename);

/**
 * typefile - Display file contents
 * @filename: File name
 *
 * Display the contents of the specified file.
 *
 * Return: -1 if the file does not exist or cannot be read. 0 otherwise.
*/
int typefile(const string &filename);

/**
 * change - Change file attributes
 * @filename: File name
 * @attribute: New attribute value
 *
 * Modify the attributes of the specified file.
 *
 * Return: -1 if the file does not exist or if the attribute change fails. 
 * 0 otherwise.
*/
int change(const string &filename, int attribute);

/**
 * md - Create a new directory
 * @pathdir: Path of the directory to create
 *
 * Create a new directory at the specified path in the mounted file system.
 *
 * Return: -1 if the directory already exists, the path is invalid, 
 * or there is insufficient space. 0 otherwise.
*/
int md(const string &pathdir);

/**
 * dir - List directory contents
 * @pathdir: Path of the directory
 *
 * Display the contents of the specified directory.
 *
 * Return: -1 if the directory does not exist or cannot be accessed. 0 otherwise.
*/
int dir(const string &pathdir);

/**
 * rd - Remove a directory
 * @pathdir: Path of the directory to remove
 *
 * Delete the specified directory from the file system. 
 * The directory must be empty.
 *
 * Return: -1 if the directory does not exist, is not empty, 
 * or cannot be deleted. 0 otherwise.
*/
int rd(const string &pathdir);

#endif