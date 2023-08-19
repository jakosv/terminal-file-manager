#include "file.h"

#include <math.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

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
    info->size = buf.st_size;
    info->name[max_name_len] = '\0';
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

char *file_owner_name(int uid)
{
    return 0;
}

char *file_ftime(time_t time)
{
    return 0;
}
