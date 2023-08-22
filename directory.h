#ifndef DIRECTORY_H_SENTRY
#define DIRECTORY_H_SENTRY

#include "list_of_files.h"

#include <sys/types.h>
#include <dirent.h>

/*
void get_dir_data(DIR *dirp, struct list_of_files *lst);
*/
void get_dir_data(const char *dir_path, struct list_of_files *lst);

#endif
