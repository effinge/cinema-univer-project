#ifndef CINEMA_MENU_H
#define CINEMA_MENU_H

#include "catalog.h"
#include "favorites.h"
#include "user.h"

enum MenuAction {
  MENU_ACTION_BACK = 0,
  MENU_ACTION_TO_FAV = 1,
  MENU_ACTION_PROFILE = 2,
  MENU_ACTION_TO_CAT = 3,
  MENU_ACTION_EXIT = -1
};

int menu_catalog(FilmCatalog *catalog, Favorites *favorites, UserManager *um,
                 User *current_user, const char *fav_path);
int menu_favorites(FilmCatalog *catalog, Favorites *favorites, UserManager *um,
                   User *current_user, const char *fav_path);

#endif
