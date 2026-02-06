#include "ui.h"
#include "common.h"
#include <ctype.h>
#include <stdlib.h>
#include <string.h>

int utf8_codepoint_len(unsigned char c) {
  if ((c & 0x80) == 0x00) return 1;
  if ((c & 0xE0) == 0xC0) return 2;
  if ((c & 0xF0) == 0xE0) return 3;
  if ((c & 0xF8) == 0xF0) return 4;
  return 1;
}

int utf8_valid_continuation(const char *s, int len) {
  for (int i = 1; i < len; i++) {
    if ((s[i] & 0xC0) != 0x80) return 0;
  }
  return 1;
}

int display_width(const char *src) {
  if (!src || src[0] == '\0') return 0;
  int width = 0;
  const unsigned char *p = (const unsigned char *)src;
  while (*p) {
    int len = utf8_codepoint_len(*p);
    if (len == 1) { width += 1; p += 1; continue; }
    if (!utf8_valid_continuation((const char *)p, len)) { width += 1; p += 1; continue; }
    width += 1;
    p += len;
  }
  return width;
}

int utf8_truncate_to_width(const char *src, int max_cols, int ellipsis, char *out, size_t out_size) {
  if (!src || !out || out_size == 0) return 0;
  out[0] = '\0';
  if (max_cols <= 0) return 0;
  int reserved = (ellipsis && max_cols >= 3) ? 3 : 0;
  int limit = max_cols - reserved;
  const unsigned char *p = (const unsigned char *)src;
  size_t out_len = 0;
  int used = 0;
  int truncated = 0;
  while (*p) {
    int bytes = utf8_codepoint_len(*p);
    if (bytes < 1) bytes = 1;
    if (bytes > 1 && !utf8_valid_continuation((const char *)p, bytes)) bytes = 1;
    if (used + 1 > limit) { truncated = 1; break; }
    if (out_len + (size_t)bytes + 4 >= out_size) { truncated = 1; break; }
    memcpy(out + out_len, p, (size_t)bytes);
    out_len += (size_t)bytes;
    out[out_len] = '\0';
    used += 1;
    p += bytes;
  }
  if (truncated && reserved > 0) {
    if (out_len + 3 < out_size) {
      memcpy(out + out_len, "...", 3);
      out_len += 3;
      out[out_len] = '\0';
    }
  }
  return truncated;
}

void fit_text(const char *src, int width, int ellipsis, char *out, size_t out_size) {
  char temp[512];
  utf8_truncate_to_width(src, width, ellipsis, temp, sizeof(temp));
  snprintf(out, out_size, "%s", temp);
  int w = display_width(out);
  int pad = width - w;
  if (pad > 0) {
    size_t len = strlen(out);
    size_t max_pad = out_size - len - 1;
    if (max_pad > (size_t)pad) max_pad = (size_t)pad;
    for (size_t i = 0; i < max_pad; i++) out[len + i] = ' ';
    out[len + max_pad] = '\0';
  }
}

void build_card_lines(const Film *film, int active, char lines[5][CARD_LINE_BYTES]) {
  char top[CARD_LINE_BYTES];
  snprintf(top, sizeof(top), "+%.*s+", CARD_INNER, "==============================");
  if (!active) snprintf(top, sizeof(top), "+%.*s+", CARD_INNER, "------------------------------");
  snprintf(lines[0], sizeof(lines[0]), "%s", top);
  char title_line[CARD_INNER_BYTES];
  fit_text(film->title, CARD_INNER, 1, title_line, sizeof(title_line));
  snprintf(lines[1], sizeof(lines[1]), "|%s|", title_line);
  char rating_text[CARD_INNER_BYTES];
  snprintf(rating_text, sizeof(rating_text), "Рейтинг: %.1f", film->rating);
  char rating_line[CARD_INNER_BYTES];
  fit_text(rating_text, CARD_INNER, 0, rating_line, sizeof(rating_line));
  snprintf(lines[2], sizeof(lines[2]), "|%s|", rating_line);
  char year_text[CARD_INNER_BYTES];
  snprintf(year_text, sizeof(year_text), "Год: %d", film->year);
  char year_line[CARD_INNER_BYTES];
  fit_text(year_text, CARD_INNER, 0, year_line, sizeof(year_line));
  snprintf(lines[3], sizeof(lines[3]), "|%s|", year_line);
  snprintf(lines[4], sizeof(lines[4]), "%s", top);
}

void ui_render_cards(FilmNode *current, const char *title, const User *user, int isFavorites) {
  clear_screen();
  printf("%s | Пользователь: %s%s\n", title, user->login, user->isAdmin ? " (админ)" : "");
  printf("Избранное: %d%s\n\n", user->favCount, isFavorites ? " (просмотр)" : "");
  if (!current) {
    printf("Нет фильмов для отображения.\n\n");
    return;
  }
  FilmNode *prev = current->prev;
  FilmNode *next = current->next;
  char left[5][CARD_LINE_BYTES];
  char mid[5][CARD_LINE_BYTES];
  char right[5][CARD_LINE_BYTES];
  build_card_lines(&prev->film, 0, left);
  build_card_lines(&current->film, 1, mid);
  build_card_lines(&next->film, 0, right);
  for (int i = 0; i < 5; i++) {
    printf("%s  %s  %s\n", left[i], mid[i], right[i]);
  }
  printf("\n");
}

void ui_render_film_details(const Film *film, const User *user) {
  clear_screen();
  printf("Информация о фильме | Пользователь: %s%s\n\n", user->login, user->isAdmin ? " (админ)" : "");
  printf("Название : %s\n", film->title);
  printf("Год      : %d\n", film->year);
  printf("Страна   : %s\n", film->country);
  printf("Жанр     : %s\n", film->genre);
  printf("Рейтинг  : %.1f\n\n", film->rating);
}

char ui_read_command(void) {
  char buf[64];
  printf("Команда: ");
  fflush(stdout);
  if (!fgets(buf, sizeof(buf), stdin)) return 'q';
  for (size_t i = 0; i < strlen(buf); i++) {
    if (!isspace((unsigned char)buf[i])) return (char)tolower((unsigned char)buf[i]);
  }
  return '\0';
}

int ui_prompt_int(const char *prompt, int min, int max, int *out) {
  char buf[64];
  while (1) {
    if (!read_line_stdin(prompt, buf, sizeof(buf))) return 0;
    int value = 0;
    if (!parse_int(buf, &value)) { printf("Некорректное число.\n"); continue; }
    if (value < min || value > max) { printf("Значение должно быть от %d до %d.\n", min, max); continue; }
    *out = value;
    return 1;
  }
}

int ui_prompt_float(const char *prompt, float min, float max, float *out) {
  char buf[64];
  while (1) {
    if (!read_line_stdin(prompt, buf, sizeof(buf))) return 0;
    float value = 0.0f;
    if (!parse_float(buf, &value)) { printf("Некорректное число.\n"); continue; }
    if (value < min || value > max) { printf("Значение должно быть от %.1f до %.1f.\n", min, max); continue; }
    *out = value;
    return 1;
  }
}
