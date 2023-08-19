#include <time.h>
#include <string.h>
#include <stdlib.h>
#include <locale.h>
#include <ncurses.h>
#include <form.h>

#include "fm_view.h"
#include "file.h"

enum { reserved_rows = 4, header_height = 3 };
enum { fsize_width = 8, ftime_width = 17 };
enum { key_escape = 27 };
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
    setlocale(LC_ALL, "");
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
    /* wclear(view->win); */
    view->first = first;
    view->selected = first;
    view->last = get_view_last_item(view, first);
}

void view_close(struct fm_view *view)
{
    delwin(view->win);
    endwin();
}

static char *form_file_name(const char *name, enum file_type ftype,
                            int max_len)
{
    int pos;
    char *new_name;
    new_name = malloc(max_len+1);
    pos = 0;
    if (ftype == ft_dir) {
        new_name[0] = '/';
        pos++;
    } else
    if (ftype == ft_exec) {
        new_name[0] = '>';
        pos++;
    }
    strncpy(new_name + pos, name, max_len - pos);
    if (new_name[max_len-1] != 0) {
        int i;
        for (i = 3; i > 0; i--)
            new_name[max_len - i] = '.';
    }
    new_name[max_len] = '\0';
    return new_name;
}

static char *form_time_str(time_t ftime)
{
    struct tm *ltime;
    char *str;
    str = malloc(ftime_width+1);
    ltime = localtime(&ftime);
    strftime(str, ftime_width, "%b %d %H:%M %Y", ltime);
    str[ftime_width] = '\0';
    return str;
}

static int get_file_info_max_width(int win_width)
{
    return fsize_width + ftime_width + 5;
}

static int get_file_name_max_width(int win_width)
{
    return win_width-2 - get_file_info_max_width(win_width);
}

static void draw_item_str(WINDOW *win, 
                          int row, int win_width, int is_selected,
                          const char *name_str, const char *size_str,
                          const char *time_str)
{
    int finfo_x;

    finfo_x = win_width-1 - get_file_info_max_width(win_width);
    
    if (is_selected)
        wattron(win, A_BOLD|A_UNDERLINE);
    mvwprintw(win, row, 1, "%s", name_str);
    mvwprintw(win, row, finfo_x, "| %*s | %*s", 
              fsize_width, size_str, ftime_width, time_str);
    if (is_selected)
        wattroff(win, A_BOLD|A_UNDERLINE);
}

static void draw_item(WINDOW *win, struct lof_item *cur, int row, 
                      int max_width, int selected)
{
    int fname_width;
    char *file_name;
    char size_str[fsize_width + 1];
    char *time_str;

    fname_width = get_file_name_max_width(max_width);

    file_size_str(cur->data.size, size_str, fsize_width+1);
    file_name = form_file_name(cur->data.name, cur->data.type, fname_width);
    time_str = form_time_str(cur->data.mtime);

    draw_item_str(win, row, max_width, selected, 
                  file_name, size_str, time_str); 
    free(file_name);
    free(time_str);
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

static void draw_header(WINDOW *win, int row, int max_width)
{
    int fname_width;
    char *name_str;
    char size_str[fsize_width + 1];
    char time_str[ftime_width + 1];

    fname_width = get_file_name_max_width(max_width);
    name_str = malloc(fname_width + 1);

    copy_str_to_buf_center(name_str, fname_width, fname_col);
    copy_str_to_buf_center(size_str, fsize_width, fsize_col);
    copy_str_to_buf_center(time_str, ftime_width, ftime_col);
    draw_item_str(win, row, max_width, 0,
                  name_str, size_str, time_str);
    mvwhline(win, row+1, 1, 0, max_width-2);
    free(name_str);
}

void view_draw(const struct fm_view *view)
{
    struct lof_item *tmp;
    int row;
    wclear(view->win);
    box(view->win, 0, 0);
    draw_header(view->win, 1, view->cols);
    row = header_height;
    for (tmp = view->first; tmp != view->last->next; tmp = tmp->next, row++)
        draw_item(view->win, tmp, row, view->cols, tmp == view->selected);
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

static void print_in_middle(WINDOW *win, int starty, int startx, int width,
                            const char *str)
{
    int len, x;

    if (win == NULL)
        win = stdscr;

    len = strlen(str);
    x = startx + (width - len) / 2;
    mvwprintw(win, starty, x, "%s", str);
    refresh();
}

char *view_get_input(const struct fm_view *view, const char *msg)
{
    FIELD *field[2];
    FORM *input_form;
    WINDOW *input_win;
    int ch, win_x, win_y, win_height, win_width, msg_len;
    char *answer, *buf;

    answer = NULL;
    msg_len = strlen(msg);
    field[0] = new_field(1, msg_len, 1, 1, 0, 0);
    field[1] = NULL;

    set_field_back(field[0], A_UNDERLINE);
    field_opts_off(field[0], O_AUTOSKIP);

    input_form = new_form(field);
    scale_form(input_form, &win_height, &win_width); 

    win_x = (view->cols - (win_width+5)) / 2;
    win_y = (view->rows - (win_height+5)) / 2;
    input_win = newwin(win_height+5, win_width+5, win_y, win_x);
    keypad(input_win, 1);
    
    set_form_win(input_form, input_win);
    set_form_sub(input_form, derwin(input_win, win_height, win_width, 2, 2));
    box(input_win, 0, 0);

    print_in_middle(input_win, 1, 0, win_width+5, msg);
    post_form(input_form);
    wrefresh(input_win);
    refresh();

    form_driver(input_form, REQ_FIRST_FIELD);

    while ((ch = wgetch(input_win)) != key_escape) {
        switch (ch) {
        case KEY_ENTER:
        case 10:
            form_driver(input_form, REQ_VALIDATION);
            buf = field_buffer(field[0], 0);
            answer = malloc(strlen(buf)+1);
            strcpy(answer, buf);
            goto quit;
        case KEY_BACKSPACE:
        case 127:
            form_driver(input_form, REQ_DEL_PREV);
            break;
        default:
            form_driver(input_form, ch);
            break;
        }
    }

quit:
    unpost_form(input_form);
    free_form(input_form);
    free_field(field[0]);

    return answer;
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
