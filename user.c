#include "user.h"
#include "user_input.h"
#include "common.h"
#include "favorites.h"
#include "ui.h"
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

/* Проверка для регистрации: логин должен быть свободен (post_check в read_until_valid). */
static int login_free_check(const char *buf, void *ctx) {
  return ((UserManager *)ctx)->find((UserManager *)ctx, buf) == NULL;
}

UserNode *user_list_append(UserList *list, User user) {
  UserNode *node = (UserNode *)malloc(sizeof(UserNode));
  if (!node) return NULL;
  node->user = user;
  if (!list->head) {
    node->next = node;
    node->prev = node;
    list->head = node;
  } else {
    UserNode *tail = list->head->prev;
    node->next = list->head;
    node->prev = tail;
    tail->next = node;
    list->head->prev = node;
  }
  list->size++;
  return node;
}

void user_list_clear(UserList *list) {
  if (!list || !list->head) { list->size = 0; return; }
  UserNode *node = list->head;
  int remaining = list->size;
  while (remaining-- > 0) {
    UserNode *next = node->next;
    free(node);
    node = next;
  }
  list->head = NULL;
  list->size = 0;
}

void user_list_init(UserList *list) {
  list->head = NULL;
  list->size = 0;
  list->append = user_list_append;
  list->clear = user_list_clear;
}

User *user_manager_find(UserManager *um, const char *login) {
  if (!um || !um->list.head) return NULL;
  UserNode *node = um->list.head;
  int remaining = um->list.size;
  while (remaining-- > 0) {
    if (strcmp(node->user.login, login) == 0) return &node->user;
    node = node->next;
  }
  return NULL;
}

int user_manager_load(UserManager *um, const char *path) {
  FILE *fp = FOPEN(path, "r");
  if (!fp) return 0;
  user_list_clear(&um->list);
  char login[MAX_LINE];
  char password[MAX_LINE];
  char card[MAX_LINE];
  char fav_line[MAX_LINE];
  char admin_line[MAX_LINE];
  while (read_line(fp, login, sizeof(login))) {
    if (login[0] == '\0') continue;
    if (!read_line(fp, password, sizeof(password))) break;
    if (!read_line(fp, card, sizeof(card))) break;
    if (!read_line(fp, fav_line, sizeof(fav_line))) break;
    if (!read_line(fp, admin_line, sizeof(admin_line))) break;
    int favCount = 0, isAdmin = 0;
    if (!parse_int(fav_line, &favCount)) favCount = 0;
    if (!parse_int(admin_line, &isAdmin)) isAdmin = 0;
    User user;
    snprintf(user.login, sizeof(user.login), "%s", login);
    snprintf(user.password, sizeof(user.password), "%s", password);
    snprintf(user.card, sizeof(user.card), "%s", card);
    user.favCount = favCount;
    user.isAdmin = isAdmin;
    if (!um->list.append(&um->list, user)) { fclose(fp); return 0; }
  }
  fclose(fp);
  return 1;
}

int user_manager_save(UserManager *um, const char *path) {
  FILE *fp = FOPEN(path, "w");
  if (!fp) return 0;
  if (um->list.head) {
    UserNode *node = um->list.head;
    int remaining = um->list.size;
    while (remaining-- > 0) {
      User *user = &node->user;
      fprintf(fp, "%s\n%s\n%s\n%d\n%d\n", user->login, user->password, user->card,
              user->favCount, user->isAdmin);
      node = node->next;
    }
  }
  fclose(fp);
  return 1;
}

void user_manager_init(UserManager *um) {
  user_list_init(&um->list);
  um->load = user_manager_load;
  um->save = user_manager_save;
  um->find = user_manager_find;
}

int is_login_valid(const char *login) {
  size_t len = strlen(login);
  if (len < 3 || len > 20) return 0;
  for (size_t i = 0; i < len; i++) {
    if (!isalnum((unsigned char)login[i])) return 0;
  }
  return 1;
}

int is_password_valid(const char *password) {
  size_t len = strlen(password);
  if (len < 6 || len > 20) return 0;
  int hasUpper = 0, hasLower = 0, hasDigit = 0;
  for (size_t i = 0; i < len; i++) {
    if (!isalnum((unsigned char)password[i])) return 0;
    if (isupper((unsigned char)password[i])) hasUpper = 1;
    else if (islower((unsigned char)password[i])) hasLower = 1;
    else if (isdigit((unsigned char)password[i])) hasDigit = 1;
  }
  return hasUpper && hasLower && hasDigit;
}

int is_card_valid(const char *card) {
  size_t len = strlen(card);
  if (len != 16) return 0;
  for (size_t i = 0; i < len; i++) {
    if (!isdigit((unsigned char)card[i])) return 0;
  }
  return 1;
}

User *user_register(UserManager *um) {
  char login[32];
  char password[32];
  char card[32];
  if (!read_until_valid("Новый логин (3-20 лат. букв/цифр): ", login, sizeof(login),
                        is_login_valid, "Логин должен быть 3-20 символов, только латиница и цифры.",
                        login_free_check, um, "Такой логин уже существует."))
    return NULL;
  if (!read_until_valid("Новый пароль (6-20, верх/низ/цифра): ", password, sizeof(password),
                        is_password_valid, "Пароль должен содержать верхний/нижний регистр и цифру (только латиница/цифры).",
                        NULL, NULL, NULL))
    return NULL;
  if (!read_until_valid("Номер карты (16 цифр): ", card, sizeof(card),
                        is_card_valid, "Номер карты должен содержать ровно 16 цифр.",
                        NULL, NULL, NULL))
    return NULL;
  User user;
  snprintf(user.login, sizeof(user.login), "%s", login);
  snprintf(user.password, sizeof(user.password), "%s", password);
  snprintf(user.card, sizeof(user.card), "%s", card);
  user.favCount = 0;
  user.isAdmin = 0;
  UserNode *node = um->list.append(&um->list, user);
  if (!node) return NULL;
  um->save(um, g_users_path);
  return &node->user;
}

User *user_login(UserManager *um) {
  char login[32];
  char password[32];
  if (!read_line_stdin("Логин: ", login, sizeof(login))) return NULL;
  if (!read_line_stdin("Пароль: ", password, sizeof(password))) return NULL;
  User *u = um->find(um, login);
  if (!u) { printf("Пользователь не найден.\n"); return NULL; }
  if (strcmp(u->password, password) != 0) { printf("Неверный пароль.\n"); return NULL; }
  return u;
}

User *login_menu(UserManager *um) {
  while (1) {
    clear_screen();
    printf("Добро пожаловать в Консольный кинотеатр\n\n");
    printf("1) Войти\n");
    printf("2) Регистрация\n");
    printf("3) Выход\n\n");
    char cmd = ui_read_command();
    if (cmd == '1') {
      User *user = user_login(um);
      if (user) return user;
      wait_enter("Нажмите Enter, чтобы продолжить...");
    } else if (cmd == '2') {
      User *user = user_register(um);
      if (user) return user;
      wait_enter("Нажмите Enter, чтобы продолжить...");
    } else if (cmd == '3' || cmd == 'q') return NULL;
  }
}

void profile_menu(UserManager *um, User *user, char *fav_path, size_t fav_path_size) {
  int running = 1;
  while (running) {
    clear_screen();
    printf("Профиль | Пользователь: %s%s\n\n", user->login, user->isAdmin ? " (админ)" : "");
    printf("1) Изменить логин\n");
    printf("2) Изменить пароль\n");
    printf("3) Изменить номер карты\n");
    printf("4) Назад\n\n");
    char cmd = ui_read_command();
    if (cmd == '1') {
      char new_login[32];
      if (!read_validated_once("Новый логин: ", new_login, sizeof(new_login), is_login_valid,
                               "Логин должен быть 3-20 символов, только латиница и цифры."))
        continue;
      if (um->find(um, new_login) != NULL) {
        printf("Такой логин уже существует.\n");
        wait_enter("Нажмите Enter, чтобы продолжить...");
        continue;
      }
      char old_fav_path[256];
      snprintf(old_fav_path, sizeof(old_fav_path), "%s", fav_path);
      snprintf(user->login, sizeof(user->login), "%s", new_login);
      favorites_file_for(g_base_dir, user->login, fav_path, fav_path_size);
      RENAME(old_fav_path, fav_path);
      um->save(um, g_users_path);
    } else if (cmd == '2') {
      char new_password[32];
      if (!read_validated_once("Новый пароль: ", new_password, sizeof(new_password), is_password_valid,
                               "Пароль должен содержать верхний/нижний регистр и цифру (только латиница/цифры)."))
        continue;
      snprintf(user->password, sizeof(user->password), "%s", new_password);
      um->save(um, g_users_path);
    } else if (cmd == '3') {
      char new_card[32];
      if (!read_validated_once("Новая карта (16 цифр): ", new_card, sizeof(new_card), is_card_valid,
                               "Номер карты должен содержать ровно 16 цифр."))
        continue;
      snprintf(user->card, sizeof(user->card), "%s", new_card);
      um->save(um, g_users_path);
    } else if (cmd == '4' || cmd == 'q') running = 0;
  }
}
