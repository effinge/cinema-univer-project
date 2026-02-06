#include "common.h"
#include "catalog.h"
#include "favorites.h"
#include "user.h"
#include "menu.h"
#include "ui.h"
#include <locale.h>
#include <stdlib.h>
#if defined(_WIN32)
#include <windows.h>
#endif

int main(int argc, char **argv) {
#if defined(_WIN32)
  SetConsoleOutputCP(CP_UTF8);
  SetConsoleCP(CP_UTF8);
#endif
  setlocale(LC_ALL, "");
  setlocale(LC_NUMERIC, "C");
  (void)argc;
  init_paths(argv ? argv[0] : NULL);

  FilmCatalog catalog;
  catalog_init(&catalog);
  catalog.load(&catalog, g_films_path);

  UserManager um;
  user_manager_init(&um);
  um.load(&um, g_users_path);

  if (catalog.list.size == 0) {
    char path[256];
    while (1) {
      clear_screen();
      printf("Не удалось загрузить каталог из файла \"%s\".\n", g_films_path);
      printf("Проверьте, что файл существует в рабочей папке программы.\n\n");
      if (!read_line_stdin("Введите путь к файлу фильмов или пустую строку для выхода: ", path, sizeof(path))) {
        catalog.list.clear(&catalog.list);
        free(um.users);
        return 0;
      }
      if (path[0] == '\0') {
        catalog.list.clear(&catalog.list);
        free(um.users);
        return 0;
      }
      catalog.list.clear(&catalog.list);
      catalog.load(&catalog, path);
      if (catalog.list.size > 0) {
        snprintf(g_films_path, sizeof(g_films_path), "%s", path);
        break;
      }
      wait_enter("Файл не найден или пустой. Нажмите Enter, чтобы попробовать снова...");
    }
  }

  User *current_user = login_menu(&um);
  if (!current_user) {
    catalog.list.clear(&catalog.list);
    free(um.users);
    return 0;
  }

  Favorites favorites;
  favorites_init(&favorites);
  char fav_path[256];
  favorites_file_for(g_base_dir, current_user->login, fav_path, sizeof(fav_path));
  favorites.load(&favorites, fav_path);
  current_user->favCount = favorites.list.size;

  int running = 1;
  while (running) {
    clear_screen();
    printf("Главное меню | Пользователь: %s%s\n\n", current_user->login,
           current_user->isAdmin ? " (админ)" : "");
    printf("1) Каталог\n");
    printf("2) Избранное\n");
    printf("3) Профиль\n");
    printf("4) Выход\n\n");
    char cmd = ui_read_command();
    if (cmd == '1') {
      int action = menu_catalog(&catalog, &favorites, &um, current_user, fav_path);
      if (action == MENU_ACTION_TO_FAV) {
        int fav_action = menu_favorites(&catalog, &favorites, &um, current_user, fav_path);
        if (fav_action == MENU_ACTION_TO_CAT) {
          menu_catalog(&catalog, &favorites, &um, current_user, fav_path);
        } else if (fav_action == MENU_ACTION_PROFILE) {
          profile_menu(&um, current_user, fav_path, sizeof(fav_path));
        } else if (fav_action == MENU_ACTION_EXIT) {
          running = 0;
        }
      } else if (action == MENU_ACTION_PROFILE) {
        profile_menu(&um, current_user, fav_path, sizeof(fav_path));
      } else if (action == MENU_ACTION_EXIT) {
        running = 0;
      }
    } else if (cmd == '2') {
      int action = menu_favorites(&catalog, &favorites, &um, current_user, fav_path);
      if (action == MENU_ACTION_TO_CAT) {
        int next_action = menu_catalog(&catalog, &favorites, &um, current_user, fav_path);
        if (next_action == MENU_ACTION_PROFILE) {
          profile_menu(&um, current_user, fav_path, sizeof(fav_path));
        } else if (next_action == MENU_ACTION_EXIT) {
          running = 0;
        }
      } else if (action == MENU_ACTION_PROFILE) {
        profile_menu(&um, current_user, fav_path, sizeof(fav_path));
      } else if (action == MENU_ACTION_EXIT) {
        running = 0;
      }
    } else if (cmd == '3') {
      profile_menu(&um, current_user, fav_path, sizeof(fav_path));
    } else if (cmd == '4' || cmd == 'q') {
      running = 0;
    }
  }

  favorites.save(&favorites, fav_path);
  current_user->favCount = favorites.list.size;
  um.save(&um, g_users_path);

  favorites.list.clear(&favorites.list);
  catalog.list.clear(&catalog.list);
  free(um.users);
  return 0;
}
