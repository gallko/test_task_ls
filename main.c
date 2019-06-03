#include <stdio.h>
#include <dirent.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <grp.h>
#include <pwd.h>
#include <time.h>
#include <unistd.h>

extern char *__progname;

char type_file(struct stat *);
char access_to_ch(unsigned int f, char ch);
char *get_user_name(uid_t);
char *get_user_group(uid_t);
char *get_timestamp(struct timespec *, char *);
char *check_link(int, char *, struct stat *, char *);

int main(int argc, char *argv[]) {
    struct dirent *dir_entry = NULL;
    int fd;
    char pathName[PATH_MAX] = {0};
    char timeStamp[MAX_INPUT] = {0};
    char pathLink[PATH_MAX] = {0};
    struct stat info_f;

    if (argc == 1)
        strncpy(pathName, getenv("PWD"), PATH_MAX);
    else
        strncpy(pathName, argv[1], PATH_MAX);

    fd = open(pathName, O_RDONLY);
    if (fd < 0) {
        perror(pathName);
        return EXIT_FAILURE;
    }
    DIR *dir = opendir(pathName);
    if (!dir) {
        perror(__progname);
        return EXIT_FAILURE;
    }

    while ((dir_entry = readdir(dir)) != NULL) {
        if (!strcmp(dir_entry->d_name, ".") || !strcmp(dir_entry->d_name, ".."))
            continue;
        realpath(dir_entry->d_name, pathName);
        if (fstatat(fd, dir_entry->d_name, &info_f, AT_SYMLINK_NOFOLLOW)) {
            perror(dir_entry->d_name);
            continue;
        }

        printf("%c%c%c%c%c%c%c%c%c%c %5lu %s %s %9li %s %s",
               type_file(&info_f),

               access_to_ch(S_IRUSR & info_f.st_mode,'r'),
               access_to_ch(S_IWUSR & info_f.st_mode,'w'),
               access_to_ch(S_IXUSR & info_f.st_mode,'x'),

               access_to_ch(S_IRGRP & info_f.st_mode,'r'),
               access_to_ch(S_IWGRP & info_f.st_mode,'w'),
               access_to_ch(S_IXGRP & info_f.st_mode,'x'),

               access_to_ch(S_IROTH & info_f.st_mode,'r'),
               access_to_ch(S_IWOTH & info_f.st_mode,'w'),
               access_to_ch(S_IXOTH & info_f.st_mode,'x'),

               info_f.st_nlink,

               get_user_name(info_f.st_uid),
               get_user_group(info_f.st_gid),

               info_f.st_size,

               get_timestamp(&info_f.st_mtim, timeStamp),

               dir_entry->d_name
        );
        if (S_ISLNK(info_f.st_mode)) {
            readlinkat(fd, dir_entry->d_name, pathLink, PATH_MAX);
            printf(" --> %s\n", pathLink);
        } else {
            printf("\n");
        }
    }

    closedir(dir);
    close(fd);
    return EXIT_SUCCESS;
}

char type_file(struct stat *s) {
    if (S_ISREG(s->st_mode))
        return '-';
    if (S_ISDIR(s->st_mode))
        return 'd';
    if (S_ISCHR(s->st_mode))
        return 'c';
    if (S_ISBLK(s->st_mode))
        return 'b';
    if (S_ISFIFO(s->st_mode))
        return 'f';
    if (S_ISLNK(s->st_mode))
        return 'l';
    if (S_ISSOCK(s->st_mode))
        return 's';

    return '*';
}

char access_to_ch(unsigned int f, char ch) {
    return f ? ch : '-';
}

char *get_user_name(uid_t uid) {
    struct passwd *pws = getpwuid(uid);
    return pws ? pws->pw_name : "*unknown";

}

char *get_user_group(uid_t uid) {
    struct group *g = getgrgid(uid);
    return g ? g->gr_name : "*unknown";
}

char *get_timestamp(struct timespec *ts, char *buf) {
    struct tm start = {.tm_sec=ts->tv_sec};
    mktime(&start);
    strftime(buf, PATH_MAX, "%b %d %R", &start);
    return buf;
}