#ifndef CINEMA_CATALOG_H
#define CINEMA_CATALOG_H

#include "film.h"

typedef struct FilmCatalog FilmCatalog;
struct FilmCatalog {
  FilmList list;
  int (*load)(FilmCatalog *, const char *path);
  int (*save)(FilmCatalog *, const char *path);
  FilmNode *(*find)(FilmCatalog *, const char *title, int year);
};

void catalog_init(FilmCatalog *catalog);

#endif /* CINEMA_CATALOG_H */
