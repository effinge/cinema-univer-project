#include "film.h"
#include "common.h"
#include <math.h>
#include <stdlib.h>
#include <string.h>

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
