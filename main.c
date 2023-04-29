#include "file_manager.h"
#include "file.h"
#include "directory.h"
#include "list_of_files.h"

#include <stdio.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <dirent.h>

int main(int argc, char **argv)
{
    struct file_manager fm;
    const char *dir_name = ".";
    if (argc > 1)
        dir_name = argv[1];
    fm_init(&fm, dir_name);
    fm_start(&fm);
    fm_close(&fm);
    return 0;
}
