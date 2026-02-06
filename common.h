#ifndef CINEMA_COMMON_H
#define CINEMA_COMMON_H

#include <stdio.h>
#include <stddef.h>

#if defined(_WIN32)
int win_printf(const char *fmt, ...);
#define printf win_printf
#endif

#define FILMS_FILE "films.txt"
#define USERS_FILE "users.txt"
#define MAX_LINE 256

#if !defined(PATH_MAX)
#define PATH_MAX 4096
#endif
#define CINEMA_PATH_MAX 4096

#if defined(_WIN32)
#define PATH_SEP '\\'
#else
#define PATH_SEP '/'
#endif

extern char g_base_dir[CINEMA_PATH_MAX];
extern char g_films_path[CINEMA_PATH_MAX];
extern char g_users_path[CINEMA_PATH_MAX];

void init_paths(const char *argv0);
void clear_screen(void);
void wait_enter(const char *message);
void trim_newline(char *s);
int read_line(FILE *fp, char *buf, size_t size);
int read_line_stdin(const char *prompt, char *buf, size_t size);
int parse_int(const char *s, int *out);
int parse_float(const char *s, float *out);
char *str_dup(const char *s);

#if defined(_WIN32)
#define FOPEN(path, mode) fopen_utf8(path, mode)
#define RENAME(old_path, new_path) rename_utf8(old_path, new_path)
FILE *fopen_utf8(const char *path, const char *mode);
int rename_utf8(const char *old_path, const char *new_path);
#else
#define FOPEN(path, mode) fopen(path, mode)
#define RENAME(old_path, new_path) rename(old_path, new_path)
#endif

#endif /* CINEMA_COMMON_H */
