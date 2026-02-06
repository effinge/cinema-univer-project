#ifndef CINEMA_FILM_H
#define CINEMA_FILM_H

typedef struct Film {
  char *title;
  int year;
  char *country;
  char *genre;
  float rating;
} Film;

typedef struct FilmNode {
  Film film;
  struct FilmNode *next;
  struct FilmNode *prev;
} FilmNode;

typedef struct FilmList FilmList;
struct FilmList {
  FilmNode *head;
  int size;
  FilmNode *(*append)(FilmList *, Film);
  int (*remove_node)(FilmList *, FilmNode *);
  void (*clear)(FilmList *);
};

Film film_create(const char *title, int year, const char *country, const char *genre, float rating);
Film film_copy(const Film *src);
void film_free(Film *film);
int film_equals(const Film *a, const Film *b);

FilmNode *list_append(FilmList *list, Film film);
int list_remove_node(FilmList *list, FilmNode *node);
void list_clear(FilmList *list);
void list_init(FilmList *list);

int film_list_load_from_file(FilmList *list, const char *path);
int film_list_save_to_file(const FilmList *list, const char *path);
FilmNode *list_find_by_title_year(const FilmList *list, const char *title, int year);
FilmNode *list_find_by_film(const FilmList *list, const Film *film);

#endif /* CINEMA_FILM_H */
