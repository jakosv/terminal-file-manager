#ifndef FM_VIEW_H_SENTRY
#define FM_VIEW_H_SENTRY

#include "list_of_files.h"

#include <ncurses.h>

struct fm_view {
    WINDOW *win;
    int rows, cols;
    struct lof_item *first, *last, *selected; 
    int cur_pos;
};

void view_init(struct fm_view *view, struct lof_item *first);
void view_update(struct fm_view *view, struct lof_item *first, int cur_pos);
void view_close(struct fm_view *view);

void view_draw(const struct fm_view *view);
void view_resize(struct fm_view *view);

void view_hide(const struct fm_view *view);

void view_show_message(const struct fm_view *view, const char *msg);
char *view_get_input(const struct fm_view *view, const char *msg);

void view_scroll_up(struct fm_view *view);
void view_scroll_down(struct fm_view *view);
void view_page_down(struct fm_view *view);
void view_page_up(struct fm_view *view);
void view_select_next(struct fm_view *view);
void view_select_prev(struct fm_view *view);

int view_get_item_rows_count(const struct fm_view *view);

#endif
