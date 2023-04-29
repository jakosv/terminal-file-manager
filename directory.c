#include "directory.h"
#include "file.h"
#include "list_of_files.h"

#include <sys/types.h>
#include <dirent.h>
#include <string.h>

enum { max_path_len = 254 };

void get_dir_data(DIR *dirp, struct list_of_files *lst)
{
    struct dirent *dir_rec;
    lof_init(lst);
    while ((dir_rec = readdir(dirp)) != NULL) {
        struct file_info file;
        get_file_info(dir_rec->d_name, &file);
        lof_add(lst, &file);
    } 
}
