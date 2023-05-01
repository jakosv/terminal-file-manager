#include "fm_view.h"
#include "file.h"

#include <time.h>
#include <string.h>
#include <stdlib.h>

enum { reserved_rows = 4, header_height = 3 };
enum { fsize_width = 8, ftime_width = 17, separator_width = 3 };
static const char fname_col[] = "Name";
static const char fsize_col[] = "Size";
static const char ftime_col[] = "Modify time";

static struct lof_item *get_view_last_item(struct fm_view *view, 
                                           struct lof_item *first)
{
    int row;
    struct lof_item *last;
    last = first;
    for (row = 1; row < view->rows - reserved_rows; row++)
        if (last->next)
            last = last->next;
        else
            break;
    return last;
}

static struct lof_item *get_view_first_item(struct fm_view *view, 
                                            struct lof_item *last)
{
    int row;
    struct lof_item *first;
    first = last;
    for (row = 1; row < view->rows - reserved_rows; row++)
        if (first->prev)
            first = first->prev;
        else
            break;
    return first;
}

void view_init(struct fm_view *view, struct lof_item *first)
{
    initscr();
    getmaxyx(stdscr, view->rows, view->cols);
    cbreak();
    noecho();
    keypad(stdscr, 1);
    curs_set(0);
    view->win = newwin(view->rows, view->cols, 0, 0);
    view->first = first;
    view->selected = first;
    view->last = get_view_last_item(view, first);
    refresh();
}

void view_update(struct fm_view *view, struct lof_item *first)
{
    wclear(view->win);
    view->first = first;
    view->selected = first;
    view->last = get_view_last_item(view, first);
}

void view_close(struct fm_view *view)
{
    delwin(view->win);
    endwin();
}

static void form_file_name(char *new_name, const char *name, 
                             enum file_type ftype, int fname_width)
{
    int max_len;
    max_len = fname_width;
    if (ftype == ft_dir) {
        new_name[0] = '/';
        new_name++;
        max_len--;
    }
    strncpy(new_name, name, max_len);
    if (new_name[max_len-1] != 0) {
        int i;
        for (i = 3; i > 0; i--)
            new_name[max_len - i] = '.';
    }
    new_name[max_len] = '\0';
}

static void form_time_str(char *str, time_t ftime)
{
    struct tm *ltime;
    ltime = localtime(&ftime);
    strftime(str, ftime_width, "%b %d %H:%M %Y", ltime);
    str[ftime_width] = '\0';
}

static void draw_item(WINDOW *win, struct lof_item *cur, int row, 
                      int fname_width, int selected)
{
    char size_str[fsize_width + 1];
    char file_name[max_name_len];
    char time_str[ftime_width + 1];
    file_size_str(cur->data.size, size_str, fsize_width+1);
    form_file_name(file_name, cur->data.name, cur->data.type, fname_width);
    form_time_str(time_str, cur->data.mtime);
    if (selected)
        wattron(win, A_BOLD|A_UNDERLINE);
    mvwprintw(win, row, 1, "%-*s | %*s | %*s", fname_width, file_name,
             fsize_width, size_str, ftime_width, time_str);
    if (selected)
        wattroff(win, A_BOLD|A_UNDERLINE);
}

static void copy_str_to_buf_center(char *buf, int buf_size, const char *str)
{
    int i, center, str_size;
    str_size = strlen(str);
    center = (buf_size - str_size) / 2;
    for (i = 0; i < center; i++)
        buf[i] = ' ';
    strncpy(buf + center, str, str_size);
    buf[center + str_size] = '\0';
}

static void draw_header(WINDOW *win, int row, int fname_width)
{
    char *name_str;
    char size_str[fsize_width + 1];
    char time_str[ftime_width + 1];
    name_str = malloc(fname_width + 1);
    copy_str_to_buf_center(name_str, fname_width, fname_col);
    copy_str_to_buf_center(size_str, fsize_width, fsize_col);
    copy_str_to_buf_center(time_str, ftime_width, ftime_col);
    mvwhline(win, row + 1, 1, 0, 
            fname_width + fsize_width + ftime_width + 2*separator_width); 
    mvwprintw(win, row, 1, "%-*s | %-*s | %-*s", fname_width, name_str, 
             fsize_width, size_str, ftime_width, time_str);
    free(name_str);
}

void view_draw(const struct fm_view *view)
{
    struct lof_item *tmp;
    int row, fname_width;
    fname_width = 
        view->cols - 2 - fsize_width - ftime_width - 2*separator_width;
    box(view->win, 0, 0);
    draw_header(view->win, 1, fname_width);
    row = header_height;
    for (tmp = view->first; tmp != view->last->next; tmp = tmp->next, row++)
        draw_item(view->win, tmp, row, fname_width, tmp == view->selected);
    wrefresh(view->win);
}

void view_show_message(const struct fm_view *view, const char *msg)
{
    int win_x, win_y, win_height, win_width, msg_len;
    WINDOW *msg_win;
    msg_len = strlen(msg);
    win_height = 3;
    win_width = msg_len + 2;
    win_x = (view->cols - win_width) / 2;
    win_y = (view->rows - win_height) / 2;
    msg_win = newwin(win_height, win_width, win_y, win_x);
    box(msg_win, 0, 0);
    refresh();
    mvwprintw(msg_win, 1, 1, "%s", msg);
    wrefresh(msg_win);
    delwin(msg_win);
}

void view_resize(struct fm_view *view)
{
    getmaxyx(stdscr, view->rows, view->cols);
    wresize(view->win, view->rows, view->cols);
    view_update(view, view->first);
    refresh();
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

void view_page_down(struct fm_view *view)
{
    if (!view->last->next) {
        view->selected = view->last;
        return;
    }
    view->last = get_view_last_item(view, view->last->next);
    view->first = get_view_first_item(view, view->last);
    view->selected = view->first;
}

void view_page_up(struct fm_view *view)
{
    if (!view->first->prev) {
        view->selected = view->first;
        return;
    }
    view->first = get_view_first_item(view, view->first->prev);
    view->last = get_view_last_item(view, view->first);
    view->selected = view->last;
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
