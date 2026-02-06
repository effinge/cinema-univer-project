#include "user_input.h"
#include "common.h"
#include <stdio.h>

int read_until_valid(const char *prompt, char *buf, size_t size,
                     user_input_validator_fn validator,
                     const char *error_msg,
                     user_input_post_check_fn post_check, void *post_check_ctx,
                     const char *post_error_msg) {
  while (1) {
    if (!read_line_stdin(prompt, buf, size)) return 0;
    if (!validator(buf)) {
      printf("%s\n", error_msg);
      continue;
    }
    if (post_check && !post_check(buf, post_check_ctx)) {
      printf("%s\n", post_error_msg);
      continue;
    }
    return 1;
  }
}

int read_validated_once(const char *prompt, char *buf, size_t size,
                        user_input_validator_fn validator,
                        const char *error_msg) {
  if (!read_line_stdin(prompt, buf, size)) return 0;
  if (!validator(buf)) {
    printf("%s\n", error_msg);
    wait_enter("Нажмите Enter, чтобы продолжить...");
    return 0;
  }
  return 1;
}
