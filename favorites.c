#include "favorites.h"
#include "user.h"
#include "common.h"
#include <stdlib.h>
#include <string.h>

int favorites_load(Favorites *favorites, const char *path) {
  FILE *fp = FOPEN(path, "r");
  if (!fp) return 0;
  char title[MAX_LINE];
  char year_line[MAX_LINE];
  char country[MAX_LINE];
  char genre[MAX_LINE];
  char rating_line[MAX_LINE];
  while (read_line(fp, title, sizeof(title))) {
    if (title[0] == '\0') continue;
    if (!read_line(fp, year_line, sizeof(year_line))) break;
    if (!read_line(fp, country, sizeof(country))) break;
    if (!read_line(fp, genre, sizeof(genre))) break;
    if (!read_line(fp, rating_line, sizeof(rating_line))) break;
    int year = 0;
    float rating = 0.0f;
    if (!parse_int(year_line, &year)) continue;
    if (!parse_float(rating_line, &rating)) continue;
    Film film = film_create(title, year, country, genre, rating);
    favorites->list.append(&favorites->list, film);
  }
  fclose(fp);
  return 1;
}

int favorites_save(Favorites *favorites, const char *path) {
  FILE *fp = FOPEN(path, "w");
  if (!fp) return 0;
  if (favorites->list.head) {
    FilmNode *node = favorites->list.head;
    int remaining = favorites->list.size;
    while (remaining-- > 0) {
      fprintf(fp, "%s\n%d\n%s\n%s\n%.1f\n", node->film.title, node->film.year,
              node->film.country, node->film.genre, node->film.rating);
      node = node->next;
    }
  }
  fclose(fp);
  return 1;
}

int favorites_contains(Favorites *favorites, const Film *film) {
  if (!favorites->list.head) return 0;
  FilmNode *node = favorites->list.head;
  int remaining = favorites->list.size;
  while (remaining-- > 0) {
    if (film_equals(&node->film, film)) return 1;
    node = node->next;
  }
  return 0;
}

int favorites_remove_film(Favorites *favorites, const Film *film) {
  if (!favorites->list.head) return 0;
  FilmNode *node = favorites->list.head;
  int remaining = favorites->list.size;
  while (remaining-- > 0) {
    if (film_equals(&node->film, film)) {
      favorites->list.remove_node(&favorites->list, node);
      return 1;
    }
    node = node->next;
  }
  return 0;
}

void favorites_init(Favorites *favorites) {
  list_init(&favorites->list);
  favorites->load = favorites_load;
  favorites->save = favorites_save;
  favorites->contains = favorites_contains;
  favorites->remove_film = favorites_remove_film;
}

void favorites_file_for(const char *base_dir, const char *login, char *out, size_t out_size) {
  snprintf(out, out_size, "%s%cfavorites_%s.txt", base_dir, PATH_SEP, login);
}

void remove_film_from_all_favorites(const Film *film, UserManager *um, const char *skip_login) {
  for (int i = 0; i < um->count; i++) {
    User *user = &um->users[i];
    if (skip_login && strcmp(user->login, skip_login) == 0) continue;
    char path[256];
    favorites_file_for(g_base_dir, user->login, path, sizeof(path));
    Favorites tmp;
    favorites_init(&tmp);
    tmp.load(&tmp, path);
    if (tmp.remove_film(&tmp, film)) {
      tmp.save(&tmp, path);
      user->favCount = tmp.list.size;
    }
    tmp.list.clear(&tmp.list);
  }
  um->save(um, g_users_path);
}
