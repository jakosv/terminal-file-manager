#ifndef FILE_MANAGER_H_SENTRY
#define FILE_MANAGER_H_SENTRY

#include "list_of_files.h"
#include "fm_view.h"

#include <stdio.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <dirent.h>

struct file_manager {
    DIR *dirp;
    struct list_of_files files;
    struct fm_view view;
};

void fm_init(struct file_manager *fm, const char *dir_path);
void fm_close(struct file_manager *fm);
void fm_start(struct file_manager *fm);

#endif
