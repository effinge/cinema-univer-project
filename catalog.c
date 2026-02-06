#include "catalog.h"
#include "common.h"
#include <stdlib.h>
#include <string.h>

int catalog_load(FilmCatalog *catalog, const char *path) {
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
    catalog->list.append(&catalog->list, film);
  }
  fclose(fp);
  return 1;
}

int catalog_save(FilmCatalog *catalog, const char *path) {
  FILE *fp = FOPEN(path, "w");
  if (!fp) return 0;
  if (catalog->list.head) {
    FilmNode *node = catalog->list.head;
    int remaining = catalog->list.size;
    while (remaining-- > 0) {
      fprintf(fp, "%s\n%d\n%s\n%s\n%.1f\n", node->film.title, node->film.year,
              node->film.country, node->film.genre, node->film.rating);
      node = node->next;
    }
  }
  fclose(fp);
  return 1;
}

FilmNode *catalog_find(FilmCatalog *catalog, const char *title, int year) {
  if (!catalog->list.head) return NULL;
  FilmNode *node = catalog->list.head;
  int remaining = catalog->list.size;
  while (remaining-- > 0) {
    if (node->film.year == year && strcmp(node->film.title, title) == 0) return node;
    node = node->next;
  }
  return NULL;
}

void catalog_init(FilmCatalog *catalog) {
  list_init(&catalog->list);
  catalog->load = catalog_load;
  catalog->save = catalog_save;
  catalog->find = catalog_find;
}
