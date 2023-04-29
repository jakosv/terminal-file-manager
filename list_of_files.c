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

void lof_remove_item(struct list_of_files *lst, struct lof_item *item)
{
    if (item->prev)
        item->prev->next = item->next;
    else
        lst->first = item->next;    
    if (item->next)
        item->next->prev = item->prev;
    else
        lst->last = item->prev;
    free(item);
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
