#include "user.h"
#include "common.h"
#include "favorites.h"
#include "ui.h"
#include <ctype.h>
#include <stdlib.h>
#include <string.h>

void ensure_user_capacity(UserManager *um) {
  if (um->count < um->cap) return;
  int newCap = um->cap == 0 ? 8 : um->cap * 2;
  User *next = (User *)realloc(um->users, sizeof(User) * newCap);
  if (!next) return;
  um->users = next;
  um->cap = newCap;
}

int user_manager_find_index(UserManager *um, const char *login) {
  for (int i = 0; i < um->count; i++) {
    if (strcmp(um->users[i].login, login) == 0) return i;
  }
  return -1;
}

int user_manager_load(UserManager *um, const char *path) {
  FILE *fp = FOPEN(path, "r");
  if (!fp) return 0;
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
    if (um->count == um->cap) {
      int newCap = um->cap == 0 ? 8 : um->cap * 2;
      User *next = (User *)realloc(um->users, sizeof(User) * newCap);
      if (!next) { fclose(fp); return 0; }
      um->users = next;
      um->cap = newCap;
    }
    User *user = &um->users[um->count++];
    snprintf(user->login, sizeof(user->login), "%s", login);
    snprintf(user->password, sizeof(user->password), "%s", password);
    snprintf(user->card, sizeof(user->card), "%s", card);
    user->favCount = favCount;
    user->isAdmin = isAdmin;
  }
  fclose(fp);
  return 1;
}

int user_manager_save(UserManager *um, const char *path) {
  FILE *fp = FOPEN(path, "w");
  if (!fp) return 0;
  for (int i = 0; i < um->count; i++) {
    User *user = &um->users[i];
    fprintf(fp, "%s\n%s\n%s\n%d\n%d\n", user->login, user->password, user->card,
            user->favCount, user->isAdmin);
  }
  fclose(fp);
  return 1;
}

void user_manager_init(UserManager *um) {
  um->users = NULL;
  um->count = 0;
  um->cap = 0;
  um->load = user_manager_load;
  um->save = user_manager_save;
  um->find_index = user_manager_find_index;
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
  while (1) {
    if (!read_line_stdin("Новый логин (3-20 лат. букв/цифр): ", login, sizeof(login))) return NULL;
    if (!is_login_valid(login)) {
      printf("Логин должен быть 3-20 символов, только латиница и цифры.\n");
      continue;
    }
    if (um->find_index(um, login) >= 0) {
      printf("Такой логин уже существует.\n");
      continue;
    }
    break;
  }
  while (1) {
    if (!read_line_stdin("Новый пароль (6-20, верх/низ/цифра): ", password, sizeof(password))) return NULL;
    if (!is_password_valid(password)) {
      printf("Пароль должен содержать верхний/нижний регистр и цифру (только латиница/цифры).\n");
      continue;
    }
    break;
  }
  while (1) {
    if (!read_line_stdin("Номер карты (16 цифр): ", card, sizeof(card))) return NULL;
    if (!is_card_valid(card)) {
      printf("Номер карты должен содержать ровно 16 цифр.\n");
      continue;
    }
    break;
  }
  ensure_user_capacity(um);
  User *user = &um->users[um->count++];
  snprintf(user->login, sizeof(user->login), "%s", login);
  snprintf(user->password, sizeof(user->password), "%s", password);
  snprintf(user->card, sizeof(user->card), "%s", card);
  user->favCount = 0;
  user->isAdmin = 0;
  um->save(um, g_users_path);
  return user;
}

User *user_login(UserManager *um) {
  char login[32];
  char password[32];
  if (!read_line_stdin("Логин: ", login, sizeof(login))) return NULL;
  if (!read_line_stdin("Пароль: ", password, sizeof(password))) return NULL;
  int idx = um->find_index(um, login);
  if (idx < 0) { printf("Пользователь не найден.\n"); return NULL; }
  if (strcmp(um->users[idx].password, password) != 0) { printf("Неверный пароль.\n"); return NULL; }
  return &um->users[idx];
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
      if (!read_line_stdin("Новый логин: ", new_login, sizeof(new_login))) continue;
      if (!is_login_valid(new_login)) {
        printf("Логин должен быть 3-20 символов, только латиница и цифры.\n");
        wait_enter("Нажмите Enter, чтобы продолжить...");
        continue;
      }
      if (um->find_index(um, new_login) >= 0) {
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
      if (!read_line_stdin("Новый пароль: ", new_password, sizeof(new_password))) continue;
      if (!is_password_valid(new_password)) {
        printf("Пароль должен содержать верхний/нижний регистр и цифру (только латиница/цифры).\n");
        wait_enter("Нажмите Enter, чтобы продолжить...");
        continue;
      }
      snprintf(user->password, sizeof(user->password), "%s", new_password);
      um->save(um, g_users_path);
    } else if (cmd == '3') {
      char new_card[32];
      if (!read_line_stdin("Новая карта (16 цифр): ", new_card, sizeof(new_card))) continue;
      if (!is_card_valid(new_card)) {
        printf("Номер карты должен содержать ровно 16 цифр.\n");
        wait_enter("Нажмите Enter, чтобы продолжить...");
        continue;
      }
      snprintf(user->card, sizeof(user->card), "%s", new_card);
      um->save(um, g_users_path);
    } else if (cmd == '4' || cmd == 'q') running = 0;
  }
}
