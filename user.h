#ifndef CINEMA_USER_H
#define CINEMA_USER_H

#include <stddef.h>

typedef struct User {
  char login[21];
  char password[21];
  char card[17];
  int favCount;
  int isAdmin;
} User;

typedef struct UserManager UserManager;
struct UserManager {
  User *users;
  int count;
  int cap;
  int (*load)(UserManager *, const char *path);
  int (*save)(UserManager *, const char *path);
  int (*find_index)(UserManager *, const char *login);
};

void user_manager_init(UserManager *um);
int is_login_valid(const char *login);
int is_password_valid(const char *password);
int is_card_valid(const char *card);
User *user_register(UserManager *um);
User *user_login(UserManager *um);
User *login_menu(UserManager *um);
void profile_menu(UserManager *um, User *user, char *fav_path, size_t fav_path_size);

#endif /* CINEMA_USER_H */
