#include "favorites.h"
#include "user.h"
#include "common.h"
#include <stdlib.h>
#include <string.h>

int favorites_load(Favorites *favorites, const char *path) {
  return film_list_load_from_file(&favorites->list, path);
}

int favorites_save(Favorites *favorites, const char *path) {
  return film_list_save_to_file(&favorites->list, path);
}

int favorites_contains(Favorites *favorites, const Film *film) {
  return list_find_by_film(&favorites->list, film) != NULL;
}

int favorites_remove_film(Favorites *favorites, const Film *film) {
  FilmNode *node = list_find_by_film(&favorites->list, film);
  if (!node) return 0;
  favorites->list.remove_node(&favorites->list, node);
  return 1;
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
  if (!um->list.head) return;
  UserNode *node = um->list.head;
  int remaining = um->list.size;
  while (remaining-- > 0) {
    User *user = &node->user;
    if (skip_login && strcmp(user->login, skip_login) == 0) { node = node->next; continue; }
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
    node = node->next;
  }
  um->save(um, g_users_path);
}
