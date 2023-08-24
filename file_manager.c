#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/errno.h>
#include <sys/wait.h>
#include <stdlib.h>

#include "file_manager.h"
#include "directory.h"
#include "list_of_files.h"
#include "fm_view.h"
#include "config.h"

enum { key_space = ' ' };

static const char delete_file_alert[] = "Delete this file? (y/n)";

static void fm_update(struct file_manager *fm, const char *new_dir_path)
{
    int first_item_pos, selected_item_pos, view_cur_pos;
    struct lof_item *first_item;
    char cur_dir[] = ".";

    if (!new_dir_path) {
        first_item_pos = lof_get_item_pos(fm->view.first, &fm->files); 
        selected_item_pos = lof_get_item_pos(fm->view.selected, &fm->files);
    } else {
        chdir(new_dir_path);
        first_item_pos = 0;
        selected_item_pos = 0;
    }

    view_cur_pos = selected_item_pos - first_item_pos;

    lof_free(&fm->files);
    get_dir_data(cur_dir, &fm->files);

    first_item = lof_get_item_by_pos(first_item_pos, &fm->files);
    view_update(&fm->view, first_item, view_cur_pos);
}

static char *get_error_message(const char *s)
{
    char *err_msg, *full_err_msg;
    int str_len, err_len, res_len;

    str_len = strlen(s);
    err_msg = strerror(errno);
    err_len = strlen(err_msg);
    res_len = str_len + err_len + 3;

    full_err_msg = malloc(res_len);
    snprintf(full_err_msg, res_len, "%s: %s", s, err_msg);
    return full_err_msg;
}

static void fm_error(const char *err_title, struct file_manager *fm)
{
    char *err_msg;
    err_msg = get_error_message(err_title);
    view_show_message(&fm->view, err_msg);
    getch();
    fm_update(fm, NULL);
    free(err_msg);
}

void fm_init(struct file_manager *fm, const char *dir_path)
{
    chdir(dir_path);
    get_dir_data(".", &fm->files);
    view_init(&fm->view, fm->files.first);
}

void fm_close(struct file_manager *fm)
{
    view_close(&fm->view);
    lof_free(&fm->files);
}

/*
static void move_file_to_trash(const char *file_name)
{
    int res;
    char trash_file_path[max_name_len];
    memcpy(trash_file_path, trash_path, sizeof(trash_path));
    strncat(trash_file_path, file_name, strlen(file_name)); 
    res = link(file_name, trash_file_path);
    if (res == -1) {
        fm_error("delete", fm));
        return;
    }
}
*/

static void open_selected_dir(struct lof_item *selection,
                              struct file_manager *fm)
{
    if (fm->view.selected->data.type != ft_dir)
        return;
    fm_update(fm, selection->data.name);
}

static void wait_ignoring_interrupts(int *stat_loc)
{
    for (;;) {
        int wait_res;
        wait_res = wait(NULL);
        if (wait_res == -1 && errno != EINTR)
            break;
    }
}

static void str_trim(char *s)
{
    char *p;
    /* skip spaces at the begginig */
    for (p = s; *p && *p == ' '; p++)
        {}
    /* copy sting to the beggining */
    for (; *p && *p != ' '; p++) {
        *s = *p;
        s++;
    }
    *s = '\0';
}

static void run_program(const char *name, char* const *args, 
                        struct file_manager *fm)
{
    int pid;

    view_hide(&fm->view);

    pid = fork();
    if (pid == -1) {
        fm_error("fork", fm);
        return;
    }
    if (pid == 0) {
        execvp(name, args);
        fm_error(name, fm);
        return;
    } 
    wait_ignoring_interrupts(NULL);

    printf("\nPress any key to continue");
    fflush(stdout);
    cbreak();
    getch();
    printf("\n");

    fm_update(fm, NULL);
}

static void open_selected_file(struct lof_item *selection, 
                               struct file_manager *fm)
{
    char *prog_name;
    char *args[3];

    prog_name = view_get_input(&fm->view, "Open file with");
    if (!prog_name || prog_name[0] == ' ')
        return;
    str_trim(prog_name);

    args[0] = prog_name;
    args[1] = selection->data.name;
    args[2] = NULL;
    run_program(prog_name, args, fm);

    free(prog_name);
}

static void exec_selected_file(struct lof_item *selection, 
                               struct file_manager *fm)
{
    char *prog_name;
    char *args[2];

    prog_name = malloc(sizeof(selection->data.name)+3);
    strcpy(prog_name+2, selection->data.name);
    prog_name[0] = '.';
    prog_name[1] = '/';

    args[0] = prog_name;
    args[1] = NULL;
    run_program(prog_name, args, fm);

    free(prog_name);
}

static void handle_selected_file(struct lof_item *selection, 
                                 struct file_manager *fm)
{
    enum file_type type;
    type = selection->data.type;
    switch (type) {
        case ft_dir:
            open_selected_dir(selection, fm);
            break;
        case ft_exec:
            exec_selected_file(selection, fm);
            break;
        case ft_file:
        default:
            open_selected_file(selection, fm);
    }
}

static void fm_create_file(struct file_manager *fm)
{
    char *file_name;
    int fd;

    file_name = view_get_input(&fm->view, "Enter file name");
    if (!file_name || file_name[0] == ' ')
        return;
    str_trim(file_name);

    fd = open(file_name, O_CREAT|O_WRONLY|O_TRUNC, 0666);
    if (fd == -1) {
        fm_error(file_name, fm);
    } else {
        close(fd);
        fm_update(fm, NULL);
    }
    free(file_name);
}

static void fm_copy_file(struct lof_item *selection, 
                         struct file_manager *fm)
{
    int err;
    char *copy_path;

    copy_path = view_get_input(&fm->view, "Enter copy destination");
    if (!copy_path || copy_path[0] == ' ')
        return;
    str_trim(copy_path);

    err = copy_file(&selection->data, copy_path);
    if (err == -1)
        fm_error(copy_path, fm);
    else
        fm_update(fm, NULL);

    free(copy_path);
}

static void fm_move_file(struct lof_item *selection, 
                         struct file_manager *fm)
{
    int err;
    char *new_path;

    new_path = view_get_input(&fm->view, "Enter new file path");
    if (!new_path || new_path[0] == ' ')
        return;
    str_trim(new_path);

    err = move_file(&selection->data, new_path);
    if (err == -1)
        fm_error(new_path, fm);
    else
        fm_update(fm, NULL);

    free(new_path);
}

static void fm_rename_file(struct lof_item *selection, 
                           struct file_manager *fm)
{
    int err;
    char *new_name;

    new_name = view_get_input(&fm->view, "Enter new file name");
    if (!new_name || new_name[0] == ' ')
        return;
    str_trim(new_name);

    err = rename_file(&selection->data, new_name);
    if (err == -1)
        fm_error(new_name, fm);
    else
        fm_update(fm, NULL);

    free(new_name);
}

static void fm_create_dir(struct file_manager *fm)
{
    char *dir_name;
    int res;

    dir_name = view_get_input(&fm->view, "Enter directory name");
    if (!dir_name || dir_name[0] == ' ')
        return;
    str_trim(dir_name);

    res = mkdir(dir_name, 0777);
    if (res == -1) {
        fm_error(dir_name, fm);
        return;
    }

    free(dir_name);
    fm_update(fm, NULL);
}

static void fm_delete_file(struct lof_item *selection, 
                           struct file_manager *fm)
{
    int key, res;

    view_show_message(&fm->view, delete_file_alert);
    key = getch();
    if (key == 'n')
        return;

    res = remove_file(&selection->data);
    if (res == -1)
        fm_error(selection->data.name, fm);
    else
        fm_update(fm, NULL);
}

void fm_start(struct file_manager *fm)
{
    int key;
    view_draw(&fm->view);
    while ((key = getch()) != 'q') {
        switch(key) {
        case KEY_UP:
        case 'k':
            view_select_prev(&fm->view);
            break;
        case KEY_DOWN:
        case 'j':
            view_select_next(&fm->view);
            break;
        case 'f':
            fm_create_file(fm);
            break;
        case 'F':
            fm_create_dir(fm);
            break;
        case 'c':
            fm_copy_file(fm->view.selected, fm);
            break;
        case 'm':
            fm_move_file(fm->view.selected, fm);
            break;
        case 'r':
            fm_rename_file(fm->view.selected, fm);
            break;
        case 'd':
            fm_delete_file(fm->view.selected, fm);
            break;
        case KEY_ENTER:
        case 10:
        case key_space:
            handle_selected_file(fm->view.selected, fm);
            break;
        case 'n':
            view_page_down(&fm->view);
            break;
        case 'p':
            view_page_up(&fm->view);
            break;
        case KEY_RESIZE:
            view_resize(&fm->view);
            break;
        default:
            continue;
        }
        view_draw(&fm->view);
    }
}
