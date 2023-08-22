#include "list_of_files.h"

#include <stdlib.h>

void lof_init(struct list_of_files *lst)
{
    lst->first = NULL;
    lst->last = NULL;
}

void lof_add(struct list_of_files *lst, const struct file_info *file)
{
    struct lof_item *tmp;
    tmp = malloc(sizeof(struct lof_item));
    tmp->data = *file;
    tmp->prev = lst->last;
    tmp->next = NULL;
    if (lst->last)
        lst->last->next = tmp;
    else
        lst->first = tmp;
    lst->last = tmp;
}

void lof_free(struct list_of_files *lst)
{
    if (lst->first) {
        lst->first = lst->first->next;
        while (lst->first) {
            free(lst->first->prev);
            lst->first = lst->first->next;
        }
        free(lst->last);
        lst->last = NULL;
    }
}

int lof_get_item_pos(struct lof_item *item, struct list_of_files *lst)
{
    struct lof_item *tmp;
    int pos = 1;
    for (tmp = lst->first; tmp != item; tmp = tmp->next)
        pos++;
    return pos;
}

struct lof_item *lof_get_item_by_pos(int pos, struct list_of_files *lst)
{
    struct lof_item *tmp;
    int i;
    tmp = lst->first;
    for (i = 1; i < pos && tmp; i++)
        tmp = tmp->next;
    return tmp;
}
