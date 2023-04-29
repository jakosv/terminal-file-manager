#ifndef FM_VIEW_H_SENTRY
#define FM_VIEW_H_SENTRY

#include "list_of_files.h"

#include <ncurses.h>

struct fm_view {
    int rows, cols;
    struct lof_item *first, *last, *selected; 
};

void view_init(struct fm_view *view, struct lof_item *first);
void view_update(struct fm_view *view, struct lof_item *first);
void view_close();
void view_draw(const struct fm_view *view);
void view_resize(struct fm_view *view);
void view_scroll_up(struct fm_view *view);
void view_scroll_down(struct fm_view *view);
void view_select_next(struct fm_view *view);
void view_select_prev(struct fm_view *view);

#endif
