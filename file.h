#ifndef FILE_H_SENTRY
#define FILE_H_SENTRY

#include <sys/types.h>
#include <sys/stat.h>

enum { max_name_len = 254 };
enum file_type { ft_file, ft_dir, ft_slink, ft_undef, ft_count };

struct file_info {
    int uid;
    char name[max_name_len+1];
    long size;
    enum file_type type;
    short mode;
    time_t mtime;
};

void get_file_info(const char *file_name, struct file_info *info);

void file_size_str(long size, char *str, int str_size);
char *file_owner_name(int uid);
char *file_ftime(time_t time);

#endif
