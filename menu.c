#include "menu.h"
#include "common.h"
#include "ui.h"
#include "film.h"
#include <stdlib.h>
#include <string.h>

int add_film_prompt(FilmCatalog *catalog) {
  char title[MAX_LINE];
  char country[MAX_LINE];
  char genre[MAX_LINE];
  int year = 0;
  float rating = 0.0f;
  if (!read_line_stdin("Название: ", title, sizeof(title))) return 0;
  if (title[0] == '\0') { printf("Название не может быть пустым.\n"); return 0; }
  if (!ui_prompt_int("Год (1888-2100): ", 1888, 2100, &year)) return 0;
  if (!read_line_stdin("Страна: ", country, sizeof(country))) return 0;
  if (country[0] == '\0') { printf("Страна не может быть пустой.\n"); return 0; }
  if (!read_line_stdin("Жанр: ", genre, sizeof(genre))) return 0;
  if (genre[0] == '\0') { printf("Жанр не может быть пустым.\n"); return 0; }
  if (!ui_prompt_float("Рейтинг (0.0-10.0): ", 0.0f, 10.0f, &rating)) return 0;
  if (catalog->find(catalog, title, year)) { printf("Фильм уже есть в каталоге.\n"); return 0; }
  Film film = film_create(title, year, country, genre, rating);
  catalog->list.append(&catalog->list, film);
  catalog->save(catalog, g_films_path);
  printf("Фильм добавлен в каталог.\n");
  return 1;
}

int delete_current_film(FilmCatalog *catalog, FilmNode **current) {
  if (!catalog->list.head || !current || !*current) return 0;
  FilmNode *node = *current;
  FilmNode *next = node->next;
  catalog->list.remove_node(&catalog->list, node);
  if (catalog->list.size == 0) *current = NULL;
  else *current = next;
  catalog->save(catalog, g_films_path);
  return 1;
}

int delete_film_from_catalog(FilmCatalog *catalog, const Film *film) {
  FilmNode *node = list_find_by_film(&catalog->list, film);
  if (!node) return 0;
  catalog->list.remove_node(&catalog->list, node);
  catalog->save(catalog, g_films_path);
  return 1;
}

void sync_favorites_and_user(Favorites *favorites, const char *fav_path, User *current_user, UserManager *um) {
  favorites->save(favorites, fav_path);
  current_user->favCount = favorites->list.size;
  um->save(um, g_users_path);
}

int menu_catalog(FilmCatalog *catalog, Favorites *favorites, UserManager *um,
                User *current_user, const char *fav_path) {
  FilmNode *current = catalog->list.head;
  while (1) {
    ui_render_cards(current, "Каталог", current_user, 0);
    printf("a/d: листать   i: инфо   f: в избранное   v: избранное   p: профиль   m: меню   q: выход\n");
    if (current_user->isAdmin) printf("n: новый фильм   x: удалить фильм\n");
    char cmd = ui_read_command();

    if (cmd == 'a') { if (current) current = current->prev; }
    else if (cmd == 'd') { if (current) current = current->next; }
    else if (cmd == 'i') {
      if (!current) continue;
      while (1) {
        ui_render_film_details(&current->film, current_user);
        int inFav = favorites->contains(favorites, &current->film);
        printf("f: %s избранное   b: назад\n", inFav ? "убрать из" : "добавить в");
        if (current_user->isAdmin) printf("x: удалить из каталога\n");
        char sub = ui_read_command();
        if (sub == 'b') break;
        else if (sub == 'f') {
          if (inFav) favorites->remove_film(favorites, &current->film);
          else { Film copied = film_copy(&current->film); favorites->list.append(&favorites->list, copied); }
          sync_favorites_and_user(favorites, fav_path, current_user, um);
        } else if (sub == 'x' && current_user->isAdmin) {
          Film removed = film_copy(&current->film);
          delete_current_film(catalog, &current);
          favorites->remove_film(favorites, &removed);
          sync_favorites_and_user(favorites, fav_path, current_user, um);
          remove_film_from_all_favorites(&removed, um, current_user->login);
          film_free(&removed);
          break;
        }
      }
    } else if (cmd == 'f') {
      if (!current) continue;
      if (!favorites->contains(favorites, &current->film)) {
        Film copied = film_copy(&current->film);
        favorites->list.append(&favorites->list, copied);
        sync_favorites_and_user(favorites, fav_path, current_user, um);
      }
    } else if (cmd == 'v') return MENU_ACTION_TO_FAV;
    else if (cmd == 'p') return MENU_ACTION_PROFILE;
    else if (cmd == 'm') return MENU_ACTION_BACK;
    else if (cmd == 'n' && current_user->isAdmin) {
      clear_screen();
      printf("Добавление нового фильма\n\n");
      add_film_prompt(catalog);
      if (!current) current = catalog->list.head;
      wait_enter("Нажмите Enter, чтобы продолжить...");
    } else if (cmd == 'x' && current_user->isAdmin) {
      if (!current) continue;
      Film removed = film_copy(&current->film);
      delete_current_film(catalog, &current);
      favorites->remove_film(favorites, &removed);
      sync_favorites_and_user(favorites, fav_path, current_user, um);
      remove_film_from_all_favorites(&removed, um, current_user->login);
      film_free(&removed);
    } else if (cmd == 'q') return MENU_ACTION_EXIT;
  }
}

int menu_favorites(FilmCatalog *catalog, Favorites *favorites, UserManager *um,
                  User *current_user, const char *fav_path) {
  FilmNode *current = favorites->list.head;
  while (1) {
    ui_render_cards(current, "Избранное", current_user, 1);
    printf("a/d: листать   i: инфо   r: убрать   c: каталог   p: профиль   m: меню   q: выход\n");
    char cmd = ui_read_command();
    if (cmd == 'a') { if (current) current = current->prev; }
    else if (cmd == 'd') { if (current) current = current->next; }
    else if (cmd == 'i') {
      if (!current) continue;
      while (1) {
        ui_render_film_details(&current->film, current_user);
        printf("r: убрать из избранного   b: назад\n");
        if (current_user->isAdmin) printf("x: удалить из каталога\n");
        char sub = ui_read_command();
        if (sub == 'b') break;
        else if (sub == 'r') {
          FilmNode *next = current->next;
          favorites->list.remove_node(&favorites->list, current);
          current = favorites->list.size ? next : NULL;
          sync_favorites_and_user(favorites, fav_path, current_user, um);
          break;
        } else if (sub == 'x' && current_user->isAdmin) {
          Film removed = film_copy(&current->film);
          FilmNode *next = current->next;
          favorites->list.remove_node(&favorites->list, current);
          current = favorites->list.size ? next : NULL;
          sync_favorites_and_user(favorites, fav_path, current_user, um);
          remove_film_from_all_favorites(&removed, um, current_user->login);
          delete_film_from_catalog(catalog, &removed);
          film_free(&removed);
          break;
        }
      }
    } else if (cmd == 'r') {
      if (!current) continue;
      FilmNode *next = current->next;
      favorites->list.remove_node(&favorites->list, current);
      current = favorites->list.size ? next : NULL;
      sync_favorites_and_user(favorites, fav_path, current_user, um);
    } else if (cmd == 'c') return MENU_ACTION_TO_CAT;
    else if (cmd == 'p') return MENU_ACTION_PROFILE;
    else if (cmd == 'm') return MENU_ACTION_BACK;
    else if (cmd == 'q') return MENU_ACTION_EXIT;
  }
}
