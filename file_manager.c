#include "file_manager.h"
#include "directory.h"
#include "list_of_files.h"
#include "fm_view.h"
#include "config.h"

#include <stdlib.h>
#include <string.h>
#include <unistd.h>

enum { key_space = ' ' };

static const char delete_file_alert[] = "Delete this file? (y/n)";

static void fm_open_dir(struct file_manager *fm, const char *dir_path)
{
    fm->dirp = opendir(dir_path); 
    if (!fm->dirp) {
        view_show_message(&fm->view, dir_path);
        perror(dir_path);
        exit(1);
    }
    chdir(dir_path);
}

void fm_init(struct file_manager *fm, const char *dir_path)
{
    fm_open_dir(fm, dir_path);
    get_dir_data(fm->dirp, &fm->files);
    view_init(&fm->view, fm->files.first);
}

void fm_close(struct file_manager *fm)
{
    view_close(&fm->view);
    lof_free(&fm->files);
    closedir(fm->dirp);
}

static void move_file_to_trash(const char *file_name)
{
    int res;
    char trash_file_path[max_name_len];
    memcpy(trash_file_path, trash_path, sizeof(trash_path));
    strncat(trash_file_path, file_name, strlen(file_name)); 
    res = link(file_name, trash_file_path);
    if (res == -1) {
        perror(file_name);
        exit(1);
    }
}

static void delete_file(struct file_manager *fm, struct lof_item *file)
{
    int res;
    struct lof_item *first, *selected;
    if (file->data.type != ft_file)
        return;
    /*
    move_file_to_trash(file->data.name);
    */
    res = unlink(file->data.name);
    if (res == -1) {
        perror(file->data.name);
        exit(1);
    }
    first = fm->view.first;
    selected = file->prev;
    if (file == first) {
        if (first->prev) {
            first = first->prev;
        } else {
            first = fm->files.first;
            selected = first;
        }
    }
    lof_remove_item(&fm->files, file);
    view_update(&fm->view, first);
    fm->view.selected = selected;
}

static void open_selected_dir(struct file_manager *fm)
{
    if (fm->view.selected->data.type != ft_dir)
        return;
    closedir(fm->dirp);
    fm_open_dir(fm, fm->view.selected->data.name);
    lof_free(&fm->files);
    get_dir_data(fm->dirp, &fm->files);
    view_update(&fm->view, fm->files.first);
}

static void handle_delete_key(struct file_manager *fm)
{
    int key;
    view_show_message(&fm->view, delete_file_alert);
    key = getch();
    if (key == 'y')
        delete_file(fm, fm->view.selected);
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
        case 'd':
            handle_delete_key(fm);
            break;
        case KEY_ENTER:
        case 10:
        case key_space:
            open_selected_dir(fm);
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
        }
        view_draw(&fm->view);
    }
}
