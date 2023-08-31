#include "file.h"

#include <math.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>

enum { open_file_mode = 0666 };
enum { buf_size = 4096 };

static enum file_type get_file_type(int mode)
{
    if (S_ISREG(mode)) {
        if (S_IXUSR & mode)
            return ft_exec;
        return ft_file;
    }

    if (S_ISDIR(mode))
        return ft_dir;

    if (S_ISLNK(mode))
        return ft_slink;

    return ft_undef;
}

void get_file_info(const char *file_name, struct file_info *info)
{
    struct stat buf;
    int status;
    status = lstat(file_name, &buf); /* check symlink */
    if (status == -1)
        stat(file_name, &buf);
    info->uid = buf.st_uid;
    strncpy(info->name, file_name, max_name_len);
    info->name[max_name_len] = '\0';
    info->size = buf.st_size;
    info->mode = buf.st_mode & 0x0fff;
    info->type = get_file_type(buf.st_mode);
    info->mtime = buf.st_mtime;
}

void file_size_str(long size, char *str, int str_size)
{
    char size_ch[] = {'B', 'K', 'M', 'G', 'T', 'P', 'E', 'Z', 'Y'};
    int i, sign_digs;
    double res;
    res = size;
    for (i = 0; i < sizeof(size_ch); i++) {
        if (res < 1000.0)
           break;
        res /= 1024.0; 
    }
    sign_digs = 0;
    if (res < 1.0) {
        sign_digs = 1;
        res = round(res);
        if (res == 0.0)
            sign_digs = 0;
    } else if (res < 10.0) {
        sign_digs = 1;
        res = round(res * 10.0) / 10.0;
    } else {
        res = round(res);
    }
    snprintf(str, str_size, "%3.*f%c", sign_digs, res, size_ch[i]);
}

int remove_file(const struct file_info *file)
{
    switch(file->type) {
    case ft_file:
        return unlink(file->name);
    case ft_dir:
        return rmdir(file->name);
    default:
        break;
        /* handle other types */
    }

    return 0;
}

int rename_file(const struct file_info *file, const char *new_name)
{
    return rename(file->name, new_name);
}

int move_file(const struct file_info *file, const char *new_path)
{
    return rename(file->name, new_path);
}

int copy_file(const struct file_info *file, const char *dest_path)
{
    int fd_source, fd_dest, res;
    char buf[buf_size];

    fd_source = open(file->name, O_RDONLY, open_file_mode);
    if (fd_source == -1)
        return -1;

    fd_dest = open(dest_path, O_CREAT|O_WRONLY, open_file_mode);
    if (fd_dest == -1)
        return -1;
    
    for (;;) {
        res = read(fd_source, buf, buf_size);
        if (res <= 0)
            break;
        res = write(fd_dest, buf, res);
        if (res == -1)
            break;
    }

    close(fd_source);
    close(fd_dest);
    return res;
}
