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

/* Односвязный список: только добавление в конец и обход от начала (нет навигации "назад"). */
typedef struct UserNode {
  User user;
  struct UserNode *next;
} UserNode;

typedef struct UserList UserList;
struct UserList {
  UserNode *head;
  UserNode *tail;  /* для O(1) append */
  int size;
  UserNode *(*append)(UserList *, User);
  void (*clear)(UserList *);
};

typedef struct UserManager UserManager;
struct UserManager {
  UserList list;
  int (*load)(UserManager *, const char *path);
  int (*save)(UserManager *, const char *path);
  User *(*find)(UserManager *, const char *login);
};

void user_list_init(UserList *list);
UserNode *user_list_append(UserList *list, User user);
void user_list_clear(UserList *list);

void user_manager_init(UserManager *um);
int is_login_valid(const char *login);
int is_password_valid(const char *password);
int is_card_valid(const char *card);
User *user_register(UserManager *um);
User *user_login(UserManager *um);
User *login_menu(UserManager *um);
void profile_menu(UserManager *um, User *user, char *fav_path, size_t fav_path_size);

#endif /* CINEMA_USER_H */
