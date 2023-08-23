#include "directory.h"
#include "file.h"
#include "list_of_files.h"

#include <sys/types.h>
#include <dirent.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

enum { max_path_len = 254 };

/*
void get_dir_data(DIR *dirp, struct list_of_files *lst)
*/
void get_dir_data(const char *dir_path, struct list_of_files *lst)
{
    int n, i;
    struct dirent **dir_rec;

    n = scandir(dir_path, &dir_rec, NULL, &alphasort);
    if (n == -1) {
        perror("scandir");
        exit(1);
    }

    lof_init(lst);
    for (i = 0; i < n; i++) {
        struct file_info file;
        if (strcmp(dir_rec[i]->d_name, ".") != 0) {
            get_file_info(dir_rec[i]->d_name, &file);
            lof_add(lst, &file);
        }
        free(dir_rec[i]);
    }
    /*
    while ((dir_rec = readdir(dirp)) != NULL) {
        struct file_info file;
        if (strcmp(dir_rec->d_name, ".") == 0)
            continue;
        get_file_info(dir_rec->d_name, &file);
        lof_add(lst, &file);
    } 
    */
    free(dir_rec);
}
