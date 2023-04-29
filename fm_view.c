#include "fm_view.h"
#include "file.h"

enum { fname_width = 20, fsize_width = 8, ftype_width = 10 };

void view_init(struct fm_view *view, struct lof_item *first)
{
    int row;
    initscr();
    getmaxyx(stdscr, view->rows, view->cols);
    view->rows--;
    cbreak();
    noecho();
    keypad(stdscr, 1);
    curs_set(0);
    view->first = first;
    view->selected = first;
    view->last = first;
    for (row = 2; row <= view->rows; row++)
        if (view->last->next)
            view->last = view->last->next;
        else
            break;
}

void view_update(struct fm_view *view, struct lof_item *first)
{
    int row;
    view->first = first;
    view->selected = first;
    view->last = first;
    for (row = 2; row <= view->rows; row++)
        if (view->last->next)
            view->last = view->last->next;
        else
            break;
    clear();
}

void view_close()
{
    endwin();
}

static void draw_item(struct lof_item *cur, int row, int selected)
{
    char size_str[fsize_width+1];
    file_size_str(cur->data.size, size_str, fsize_width+1);
    if (selected)
        attron(A_UNDERLINE);
    mvprintw(row, 0, "%-*.*s %*s %-*s\n", fname_width, fname_width, 
             cur->data.name, fsize_width, size_str, ftype_width, 
             file_type_str[cur->data.type]);
    if (selected)
        attroff(A_UNDERLINE);
}

void view_draw(const struct fm_view *view)
{
    struct lof_item *tmp;
    int row = 0;
    for (tmp = view->first; tmp != view->last->next; tmp = tmp->next, row++) {
        draw_item(tmp, row, tmp == view->selected);
    }
    refresh();
}

void view_resize(struct fm_view *view)
{

}

void view_scroll_up(struct fm_view *view)
{
    if (!view->first->prev)
        return;
    view->first = view->first->prev;
    view->last = view->last->prev;
}

void view_scroll_down(struct fm_view *view)
{
    if (!view->last->next)
        return;
    view->first = view->first->next;
    view->last = view->last->next;
}

void view_select_next(struct fm_view *view)
{
    if (!view->selected->next) 
        return;
    if (view->selected == view->last)
        view_scroll_down(view);
    view->selected = view->selected->next;
}

void view_select_prev(struct fm_view *view)
{
    if (!view->selected->prev) 
        return;
    if (view->selected == view->first)
        view_scroll_up(view);
    view->selected = view->selected->prev;
}
