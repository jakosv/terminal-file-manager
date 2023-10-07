#include <time.h>
#include <string.h>
#include <stdlib.h>
#include <locale.h>
#include <ncurses.h>
#include <form.h>

#include "fm_view.h"
#include "file.h"

enum { reserved_rows = 4, header_height = 3 };
enum { fsize_width = 8, ftime_width = 18 };
enum { key_escape = 27 };
enum { max_input_field_width = 50, input_win_padding = 5 };
static const char fname_col[] = "Name";
static const char fsize_col[] = "Size";
static const char ftime_col[] = "Modify time";
static const char input_form_regex[] = "^[A-z.]+";

int view_get_item_rows_count(const struct fm_view *view)
{
    return view->rows - reserved_rows;
}

static void set_view_by_last_item(struct lof_item *last, 
                                  struct fm_view *view)
{
    int row, item_rows_cnt;
    struct lof_item *first;

    item_rows_cnt = view_get_item_rows_count(view);
    first = last;
    view->selected = last;
    for (row = 1; row < item_rows_cnt; row++)
        if (first->prev)
            first = first->prev;
        else
            break;
    for (; row < item_rows_cnt; row++)
        if (last->next)
            last = last->next;
        else
            break;

    view->first = first;
    view->last = last;
}

static void set_view_by_first_item(struct lof_item *first, 
                                   struct fm_view *view)
{
    int row, item_rows_cnt;
    struct lof_item *last;

    item_rows_cnt = view_get_item_rows_count(view);
    view->selected = first;

    last = first;
    for (row = 1; row < item_rows_cnt; row++)
        if (last->next)
            last = last->next;
        else
            break;
    for (; row < item_rows_cnt; row++)
        if (first->prev)
            first = first->prev;
        else
            break;

    view->first = first;
    view->last = last;
}

void view_init(struct fm_view *view, struct lof_item *first)
{
    setlocale(LC_CTYPE, "");
    initscr();
    getmaxyx(stdscr, view->rows, view->cols);
    cbreak();
    noecho();
    keypad(stdscr, 1);
    curs_set(0);
    view->win = newwin(view->rows, view->cols, 0, 0);
    set_view_by_first_item(first, view);
    refresh();
}

static void select_item_by_pos(int pos, struct fm_view *view)
{
    int i;
    struct lof_item *tmp;
    tmp = view->first;
    for (i = 0; i < pos; i++) {
        if (tmp == view->last)
            break;
        tmp = tmp->next;
    }
    view->selected = tmp;
}

void view_update(struct fm_view *view, struct lof_item *first, int pos)
{
    wclear(view->win);
    set_view_by_first_item(first, view);
    select_item_by_pos(pos, view);
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
    int finfo_x, x;

    finfo_x = win_width-1 - get_file_info_max_width(win_width);
    
    if (is_selected)
        wattron(win, A_BOLD|A_UNDERLINE);
    mvwprintw(win, row, 1, "%s", name_str);
    x = getcurx(win);
    for (; x < finfo_x; x++)
        waddch(win, ' ');
    mvwprintw(win, row, finfo_x, "| %*s | %-*s", 
              fsize_width, size_str, ftime_width, time_str);
    if (is_selected)
        wattroff(win, A_BOLD|A_UNDERLINE);
}

void view_hide(const struct fm_view *view)
{
    endwin();
}

static void draw_item(WINDOW *win, struct lof_item *cur, int row, 
                      int max_width, int selected)
{
    int fname_width;
    char *file_name;
    char size_str[fsize_width + 1];
    char *time_str;

    fname_width = get_file_name_max_width(max_width);

    file_size_str(cur->data->size, size_str, fsize_width+1);
    file_name = form_file_name(cur->data->name, cur->data->type, fname_width);
    time_str = form_time_str(cur->data->mtime);

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
    box(view->win, 0, 0);
    draw_header(view->win, 1, view->cols);
    row = header_height;
    for (tmp = view->first; tmp != view->last->next; tmp = tmp->next, row++)
        draw_item(view->win, tmp, row, view->cols, tmp == view->selected);
    wrefresh(view->win);
}

static WINDOW *create_window_in_center(int height, int width, 
                                       int cols, int rows)
{
    int win_x, win_y;
    WINDOW *win;
    win_x = (cols - width) / 2;
    win_y = (rows - height) / 2;
    win = newwin(height, width, win_y, win_x);
    box(win, 0, 0);
    return win;
}

void view_show_message(const struct fm_view *view, const char *msg)
{
    int win_height, win_width, msg_len;
    WINDOW *msg_win;
    msg_len = strlen(msg);
    win_height = 3;
    win_width = msg_len + 2;
    msg_win = create_window_in_center(win_height, win_width, 
                                      view->cols, view->rows);
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

static char *get_form_input(FIELD *field[2], FORM *form, WINDOW *win)
{
    int ch;
    char *answer = NULL;

    form_driver(form, REQ_FIRST_FIELD);

    while ((ch = wgetch(win)) != key_escape) {
        int err;
        switch (ch) {
        case KEY_LEFT:
            form_driver(form, REQ_PREV_CHAR);
            break;
        case KEY_RIGHT:
            form_driver(form, REQ_NEXT_CHAR);
            break;
        case KEY_ENTER:
        case '\n':
            err = form_driver(form, REQ_VALIDATION);
            if (err != E_OK)
                break;
            answer = malloc(max_input_field_width + 1);
            strncpy(answer, field_buffer(field[0], 0), max_input_field_width);
            answer[max_input_field_width] = '\0';
            return answer;
        case KEY_BACKSPACE:
        case 127:
            form_driver(form, REQ_DEL_PREV);
            break;
        default:
            form_driver(form, ch);
            break;
        }
    }

    return NULL;
}

char *view_get_input(const struct fm_view *view, const char *msg)
{
    FIELD *field[2];
    FORM *input_form;
    WINDOW *input_win;
    int win_height, win_width, msg_len;
    char *answer;

    answer = NULL;
    msg_len = strlen(msg);
    field[0] = new_field(1, msg_len, 1, 1, 0, 0);
    field[1] = NULL;

    set_field_back(field[0], A_UNDERLINE);
    field_opts_off(field[0], O_AUTOSKIP);
    field_opts_off(field[0], O_STATIC);

    set_field_type(field[0], TYPE_REGEXP, input_form_regex);
    set_max_field(field[0], max_input_field_width);

    input_form = new_form(field);
    scale_form(input_form, &win_height, &win_width); 

    input_win = create_window_in_center(win_height + input_win_padding, 
                                        win_width + input_win_padding, 
                                        view->cols, view->rows);
    keypad(input_win, 1);
    curs_set(1);
    
    set_form_win(input_form, input_win);
    set_form_sub(input_form, derwin(input_win, win_height, win_width, 2, 2));

    print_in_middle(input_win, 1, 0, win_width+input_win_padding, msg);
    post_form(input_form);
    wrefresh(input_win);
    refresh();

    answer = get_form_input(field, input_form, input_win);

    unpost_form(input_form);
    free_form(input_form);
    free_field(field[0]);
    delwin(input_win);
    curs_set(0);

    return answer;
}

static int get_selected_item_pos(const struct fm_view *view)
{
    struct lof_item *tmp;
    int pos;

    pos = 0;
    for (tmp = view->first; tmp != view->selected; tmp = tmp->next)
        pos++;

    return pos;
}

void view_resize(struct fm_view *view)
{
    getmaxyx(stdscr, view->rows, view->cols);
    wresize(view->win, view->rows, view->cols);
    view_update(view, view->first, get_selected_item_pos(view));
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
    set_view_by_first_item(view->last->next, view);
}

void view_page_up(struct fm_view *view)
{
    if (!view->first->prev) {
        view->selected = view->first;
        return;
    }
    set_view_by_last_item(view->first->prev, view);
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
