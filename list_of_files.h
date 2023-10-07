#ifndef LIST_OF_FILES_H_SENTRY
#define LIST_OF_FILES_H_SENTRY

#include "file.h"

struct lof_item {
    struct file_info *data;
    struct lof_item *prev, *next;
};

struct list_of_files {
    struct lof_item *first, *last;
};

void lof_init(struct list_of_files *lst);
void lof_add(struct list_of_files *lst, struct file_info *file);
void lof_free(struct list_of_files *lst);

int lof_get_item_pos(struct lof_item *item, struct list_of_files *lst);
struct lof_item *lof_get_item_by_pos(int pos, struct list_of_files *lst);

#endif
