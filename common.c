#include "common.h"
#include <errno.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>
#if defined(_WIN32)
#include <direct.h>
#include <windows.h>
#else
#include <libgen.h>
#include <unistd.h>
#endif

char g_base_dir[CINEMA_PATH_MAX];
char g_films_path[CINEMA_PATH_MAX];
char g_users_path[CINEMA_PATH_MAX];

#if defined(_WIN32)
#include <stdarg.h>
int utf8_to_wide(const char *src, wchar_t *dst, size_t dst_count) {
  if (!src || !dst || dst_count == 0) return 0;
  int needed = MultiByteToWideChar(CP_UTF8, 0, src, -1, NULL, 0);
  if (needed <= 0 || (size_t)needed > dst_count) return 0;
  return MultiByteToWideChar(CP_UTF8, 0, src, -1, dst, (int)dst_count) > 0;
}
int wide_to_utf8(const wchar_t *src, char *dst, size_t dst_size) {
  if (!src || !dst || dst_size == 0) return 0;
  int needed = WideCharToMultiByte(CP_UTF8, 0, src, -1, NULL, 0, NULL, NULL);
  if (needed <= 0 || (size_t)needed > dst_size) return 0;
  return WideCharToMultiByte(CP_UTF8, 0, src, -1, dst, (int)dst_size, NULL, NULL) > 0;
}
FILE *fopen_utf8(const char *path, const char *mode) {
  wchar_t wpath[PATH_MAX];
  wchar_t wmode[16];
  if (!utf8_to_wide(path, wpath, PATH_MAX) || !utf8_to_wide(mode, wmode, 16)) return NULL;
  return _wfopen(wpath, wmode);
}
int rename_utf8(const char *old_path, const char *new_path) {
  wchar_t wold[PATH_MAX];
  wchar_t wnew[PATH_MAX];
  if (!utf8_to_wide(old_path, wold, PATH_MAX) || !utf8_to_wide(new_path, wnew, PATH_MAX)) return -1;
  return _wrename(wold, wnew);
}
int win_vprintf(const char *fmt, va_list ap) {
  char buf[8192];
  int len = vsnprintf(buf, sizeof(buf), fmt, ap);
  if (len < 0) return len;
  if (len >= (int)sizeof(buf)) { len = (int)sizeof(buf) - 1; buf[len] = '\0'; }
  HANDLE out = GetStdHandle(STD_OUTPUT_HANDLE);
  DWORD mode = 0;
  if (out == INVALID_HANDLE_VALUE || !GetConsoleMode(out, &mode))
    return (int)fwrite(buf, 1, (size_t)len, stdout);
  int wlen = MultiByteToWideChar(CP_UTF8, 0, buf, len, NULL, 0);
  if (wlen <= 0) return (int)fwrite(buf, 1, (size_t)len, stdout);
  wchar_t *wbuf = (wchar_t *)malloc(sizeof(wchar_t) * (size_t)wlen);
  if (!wbuf) return (int)fwrite(buf, 1, (size_t)len, stdout);
  MultiByteToWideChar(CP_UTF8, 0, buf, len, wbuf, wlen);
  DWORD written = 0;
  WriteConsoleW(out, wbuf, (DWORD)wlen, &written, NULL);
  free(wbuf);
  return (int)written;
}
int win_printf(const char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  int ret = win_vprintf(fmt, ap);
  va_end(ap);
  return ret;
}
#endif

void clear_screen(void) {
  printf("\033[2J\033[H");
  fflush(stdout);
}

void wait_enter(const char *message) {
  char buf[8];
  printf("%s", message);
  fflush(stdout);
  fgets(buf, sizeof(buf), stdin);
}

void init_paths(const char *argv0) {
#if defined(_WIN32)
  (void)argv0;
  wchar_t wresolved[PATH_MAX];
  DWORD len = GetModuleFileNameW(NULL, wresolved, (DWORD)PATH_MAX);
  if (len > 0 && len < PATH_MAX) {
    wchar_t wdir[PATH_MAX];
    wcsncpy(wdir, wresolved, PATH_MAX);
    wdir[PATH_MAX - 1] = L'\0';
    wchar_t *last = wcsrchr(wdir, L'\\');
    if (!last) last = wcsrchr(wdir, L'/');
    if (last) {
      *last = L'\0';
      if (!wide_to_utf8(wdir, g_base_dir, sizeof(g_base_dir)))
        snprintf(g_base_dir, sizeof(g_base_dir), ".");
    } else {
      snprintf(g_base_dir, sizeof(g_base_dir), ".");
    }
  } else if (_getcwd(g_base_dir, sizeof(g_base_dir))) {
  } else {
    snprintf(g_base_dir, sizeof(g_base_dir), ".");
  }
#else
  char resolved[PATH_MAX];
  char dirbuf[PATH_MAX];
  if (argv0 && realpath(argv0, resolved)) {
    snprintf(dirbuf, sizeof(dirbuf), "%s", resolved);
    snprintf(g_base_dir, sizeof(g_base_dir), "%s", dirname(dirbuf));
  } else if (getcwd(g_base_dir, sizeof(g_base_dir))) {
  } else {
    snprintf(g_base_dir, sizeof(g_base_dir), ".");
  }
#endif
  snprintf(g_films_path, CINEMA_PATH_MAX, "%s%c%s", g_base_dir, PATH_SEP, FILMS_FILE);
  snprintf(g_users_path, CINEMA_PATH_MAX, "%s%c%s", g_base_dir, PATH_SEP, USERS_FILE);
}

void trim_newline(char *s) {
  size_t len = strlen(s);
  while (len > 0 && (s[len - 1] == '\n' || s[len - 1] == '\r')) {
    s[len - 1] = '\0';
    len--;
  }
}

int read_line(FILE *fp, char *buf, size_t size) {
  if (!fgets(buf, (int)size, fp)) return 0;
  trim_newline(buf);
  return 1;
}

int read_line_stdin(const char *prompt, char *buf, size_t size) {
  printf("%s", prompt);
  fflush(stdout);
  if (!fgets(buf, (int)size, stdin)) return 0;
  trim_newline(buf);
  return 1;
}

int parse_int(const char *s, int *out) {
  char *end = NULL;
  errno = 0;
  long v = strtol(s, &end, 10);
  if (s == end || *end != '\0' || errno == ERANGE) return 0;
  *out = (int)v;
  return 1;
}

int parse_float(const char *s, float *out) {
  char *end = NULL;
  errno = 0;
  float v = strtof(s, &end);
  if (s == end || *end != '\0' || errno == ERANGE) return 0;
  *out = v;
  return 1;
}

char *str_dup(const char *s) {
  size_t len = strlen(s);
  char *out = (char *)malloc(len + 1);
  if (!out) return NULL;
  memcpy(out, s, len + 1);
  return out;
}
