#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h> 
#include <iostream>

#include "disk.h"

using namespace std;

/** Invalid file descriptor */
#define INVALID_FD -1

/** Disk instance description */
struct disk
{
    /* File descriptor */
    int fd;
    /* Block count*/
    size_t bcount;
};

// c++20 feature
/* Currently open virtual disk (invalid by default) */
static struct disk disk = {
    .fd = INVALID_FD,
    .bcount = 0
};

int block_disk_open(const char *diskname)
{
    int fd;
    /** Define some file status */
    struct stat st;

    if (!diskname) {
        // maybe you should replace some std error out

        cout << "invalid file diskname" << endl;
        return -1;
    }

    if (disk.fd != INVALID_FD) {
        cout << "disk already open" << endl;
        return -1;
    }

    if ((fd = open(diskname, O_RDWR, 0644)) < 0) {
        perror("open");
        return -1;
    }

    if (fstat(fd, &st)) {
        perror("fstat");
        return -1;
    }

    // if (st.st_size % BLOCK_SIZE != 0) {
    //     cout << "size " << st.st_size << "is not multiple of"
    //         << BLOCK_SIZE << endl;
    //     return -1;
    // }

    disk.fd = fd;
    disk.bcount = 128;

    return 0;
}

int block_disk_close()
{
    if (disk.fd == INVALID_FD) {
        cout << "No disk opened." << endl;
        return -1;
    }

    close(disk.fd);

    disk.fd = INVALID_FD;

    return 0;
}

int block_disk_count()
{
    if (disk.fd == INVALID_FD) {
        cout << "No disk opened." << endl;
        return -1;
    }

    return disk.bcount;
}

int block_read(size_t block, void *buf)
{
    if (disk.fd == INVALID_FD) {
        cout << "No disk opened." << endl;
    }

    if (block >= disk.bcount) {
        cout << "Block index out of bounds (" << block
            << "/" << disk.bcount << ").";
        return -1;
    }

    if (lseek(disk.fd, block * BLOCK_SIZE, SEEK_SET) < 0) {
        perror("lseek");
        return -1;
    }

    if (read(disk.fd, buf, BLOCK_SIZE) < 0) {
        perror("read");
        return -1;
    }

    return 0;
}

int block_write(size_t block, const void *buf) 
{
    if (disk.fd == INVALID_FD) {
        cout << "No disk opened." << endl;
    }

    if (block >= disk.bcount) {
        cout << "Block index out of bounds (" << block
            << "/" << disk.bcount << ").";
        return -1;
    }

    if (lseek(disk.fd, block * BLOCK_SIZE, SEEK_SET) < 0) {
        perror("lseek");
        return -1;
    }

    if (write(disk.fd, buf, BLOCK_SIZE) < 0) {
        perror("write");
        return -1;
    }

    return 0;
}

