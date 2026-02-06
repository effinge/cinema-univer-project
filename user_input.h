#ifndef CINEMA_USER_INPUT_H
#define CINEMA_USER_INPUT_H

#include <stddef.h>

/* Тип функции-валидатора: возвращает 1, если строка допустима */
typedef int (*user_input_validator_fn)(const char *s);

/* Дополнительная проверка после валидации (например, "логин свободен").
   ctx передаётся вызывающим. Возвращает 1, если проверка пройдена. */
typedef int (*user_input_post_check_fn)(const char *buf, void *ctx);

/* Читает ввод с stdin в цикле, пока значение не пройдёт validator.
   При необходимости вызывает post_check (если не NULL); при post_check == 0 выводит post_error_msg.
   Возвращает 0 при ошибке чтения (EOF), 1 при успехе. */
int read_until_valid(const char *prompt, char *buf, size_t size,
                     user_input_validator_fn validator,
                     const char *error_msg,
                     user_input_post_check_fn post_check, void *post_check_ctx,
                     const char *post_error_msg);

/* Читает ввод один раз; если валидация не пройдена — выводит error_msg, wait_enter, возвращает 0.
   Возвращает 1 при успехе. Для использования в меню (один ввод, при ошибке — назад). */
int read_validated_once(const char *prompt, char *buf, size_t size,
                        user_input_validator_fn validator,
                        const char *error_msg);

#endif /* CINEMA_USER_INPUT_H */
