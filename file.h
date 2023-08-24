#ifndef FILE_H_SENTRY
#define FILE_H_SENTRY

#include <sys/types.h>
#include <sys/stat.h>

enum { max_name_len = 254 };
enum file_type { ft_file, ft_dir, ft_slink, ft_exec, ft_undef, ft_count };

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

int remove_file(const struct file_info *file);
int rename_file(const struct file_info *file, const char *new_name);
int move_file(const struct file_info *file, const char *new_path);
int copy_file(const struct file_info *file, const char *dest_path);

#endif
