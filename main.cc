#include "fs.h"
#include "user.h"

int main()
{
    char diskname[] = "disk.txt";
    fs_mount(diskname);

    user_info();


    //dir("/");
    //open_file("/a/d.tx", 0);
    //open_file("c.tx", 0);

    //write_file("/a/d.tx", "love is the important thing in the universe, it can produce the power the step into the fourth demension", 120);
    //write_file("/a/d.tx", "learning english need some patient, it is the key to live.", 120);
    //close_file("/a/d.tx");

    //typefile("/a/d.tx");

    //delete_file("/a/d.tx");
    //change("/a/d.tx", 5);

    //dir("/a");
    //rd("/a/j/k");

    fs_umount(diskname);
    return 0;
}