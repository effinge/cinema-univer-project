#ifndef CINEMA_FAVORITES_H
#define CINEMA_FAVORITES_H

#include "film.h"
#include <stddef.h>

typedef struct UserManager UserManager;

typedef struct Favorites Favorites;
struct Favorites {
  FilmList list;
  int (*load)(Favorites *, const char *path);
  int (*save)(Favorites *, const char *path);
  int (*contains)(Favorites *, const Film *film);
  int (*remove_film)(Favorites *, const Film *film);
};

void favorites_init(Favorites *favorites);
void favorites_file_for(const char *base_dir, const char *login, char *out, size_t out_size);
void remove_film_from_all_favorites(const Film *film, UserManager *um, const char *skip_login);

#endif /* CINEMA_FAVORITES_H */
