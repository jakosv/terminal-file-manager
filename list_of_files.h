#ifndef LIST_OF_FILES_H_SENTRY
#define LIST_OF_FILES_H_SENTRY

#include "file.h"

struct lof_item {
    struct file_info data;
    struct lof_item *prev, *next;
};

struct list_of_files {
    struct lof_item *first, *last;
};

void lof_init(struct list_of_files *lst);
void lof_add(struct list_of_files *lst, const struct file_info *file);
void lof_remove_item(struct list_of_files *lst, struct lof_item *item);
void lof_free(struct list_of_files *lst);

#endif
