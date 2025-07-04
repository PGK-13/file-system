# FAT32 File System (fs_test)

> A minimal experimental FAT32 file system implementation in C.

## ✨ Overview

This project implements a simple FAT32 file system to help understand:

- How the FAT (File Allocation Table) works
- How file and directory entries are managed
- How reading/writing clusters is done on a disk image

It was developed as a personal learning exercise.  
Feel free to use or adapt for your own OS or file system experiments.

## 🗂️ Features

✅ Parse FAT32 boot sector  
✅ Read FAT table and cluster chains  
✅ List files and directories  
✅ Read file contents  
✅ Simple write support (optional)

## ⚙️ Build & Run

### Requirements

- GCC or Clang
- Make (optional)

### Build

```bash
gcc -o fs_test main.c fat32.c -Wall
```

Or with `Makefile`:

```bash
make
```

### Run

Make sure you have a FAT32 disk image (e.g. `fat32.img`):

```
./fs_test fat32.img
```

You can adjust the path to your disk image or modify `main.c` for testing.

## 📁 Project Structure

```
fs_test/
 ├── fat32.c        # FAT32 core implementation
 ├── fat32.h        # Header file
 ├── main.c         # Entry point for testing
 ├── Makefile       # Optional build file
 ├── fat32.img      # (Optional) example image for testing
```

## 📝 TODO

-  Add unit tests
-  Improve write support
-  Implement long filename (LFN) support

## 📜 License

MIT — use it freely for learning or experimentation!
