#include "film.h"
#include "common.h"
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

Film film_create(const char *title, int year, const char *country, const char *genre, float rating) {
  Film film;
  film.title = str_dup(title);
  film.year = year;
  film.country = str_dup(country);
  film.genre = str_dup(genre);
  film.rating = rating;
  return film;
}

Film film_copy(const Film *src) {
  return film_create(src->title, src->year, src->country, src->genre, src->rating);
}

void film_free(Film *film) {
  if (!film) return;
  free(film->title);
  free(film->country);
  free(film->genre);
  film->title = NULL;
  film->country = NULL;
  film->genre = NULL;
}

int film_equals(const Film *a, const Film *b) {
  if (a->year != b->year) return 0;
  if (fabsf(a->rating - b->rating) > 0.0001f) return 0;
  if (strcmp(a->title, b->title) != 0) return 0;
  if (strcmp(a->country, b->country) != 0) return 0;
  if (strcmp(a->genre, b->genre) != 0) return 0;
  return 1;
}

FilmNode *list_append(FilmList *list, Film film) {
  FilmNode *node = (FilmNode *)malloc(sizeof(FilmNode));
  if (!node) return NULL;
  node->film = film;
  if (!list->head) {
    node->next = node;
    node->prev = node;
    list->head = node;
  } else {
    FilmNode *tail = list->head->prev;
    node->next = list->head;
    node->prev = tail;
    tail->next = node;
    list->head->prev = node;
  }
  list->size++;
  return node;
}

int list_remove_node(FilmList *list, FilmNode *node) {
  if (!list || !node || !list->head) return 0;
  if (list->size == 1) {
    list->head = NULL;
  } else {
    if (node == list->head) list->head = node->next;
    node->prev->next = node->next;
    node->next->prev = node->prev;
  }
  film_free(&node->film);
  free(node);
  list->size--;
  return 1;
}

void list_clear(FilmList *list) {
  if (!list || !list->head) { list->size = 0; return; }
  FilmNode *node = list->head;
  int remaining = list->size;
  while (remaining-- > 0) {
    FilmNode *next = node->next;
    film_free(&node->film);
    free(node);
    node = next;
  }
  list->head = NULL;
  list->size = 0;
}

void list_init(FilmList *list) {
  list->head = NULL;
  list->size = 0;
  list->append = list_append;
  list->remove_node = list_remove_node;
  list->clear = list_clear;
}

int film_list_load_from_file(FilmList *list, const char *path) {
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
    list->append(list, film);
  }
  fclose(fp);
  return 1;
}

int film_list_save_to_file(const FilmList *list, const char *path) {
  FILE *fp = FOPEN(path, "w");
  if (!fp) return 0;
  if (list->head) {
    FilmNode *node = list->head;
    int remaining = list->size;
    while (remaining-- > 0) {
      fprintf(fp, "%s\n%d\n%s\n%s\n%.1f\n", node->film.title, node->film.year,
              node->film.country, node->film.genre, node->film.rating);
      node = node->next;
    }
  }
  fclose(fp);
  return 1;
}

FilmNode *list_find_by_title_year(const FilmList *list, const char *title, int year) {
  if (!list || !list->head) return NULL;
  FilmNode *node = list->head;
  int remaining = list->size;
  while (remaining-- > 0) {
    if (node->film.year == year && strcmp(node->film.title, title) == 0) return node;
    node = node->next;
  }
  return NULL;
}

FilmNode *list_find_by_film(const FilmList *list, const Film *film) {
  if (!list || !list->head || !film) return NULL;
  FilmNode *node = list->head;
  int remaining = list->size;
  while (remaining-- > 0) {
    if (film_equals(&node->film, film)) return node;
    node = node->next;
  }
  return NULL;
}
