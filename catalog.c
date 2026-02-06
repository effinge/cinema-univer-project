#include "catalog.h"
#include "film.h"
#include <stdlib.h>

int catalog_load(FilmCatalog *catalog, const char *path) {
  return film_list_load_from_file(&catalog->list, path);
}

int catalog_save(FilmCatalog *catalog, const char *path) {
  return film_list_save_to_file(&catalog->list, path);
}

FilmNode *catalog_find(FilmCatalog *catalog, const char *title, int year) {
  return list_find_by_title_year(&catalog->list, title, year);
}

void catalog_init(FilmCatalog *catalog) {
  list_init(&catalog->list);
  catalog->load = catalog_load;
  catalog->save = catalog_save;
  catalog->find = catalog_find;
}
