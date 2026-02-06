#ifndef CINEMA_UI_H
#define CINEMA_UI_H

#include "film.h"
#include "user.h"

#define CARD_WIDTH 24
#define CARD_INNER (CARD_WIDTH - 2)
#define CARD_INNER_BYTES (CARD_INNER * 4 + 8)
#define CARD_LINE_BYTES (CARD_INNER_BYTES + 4)

void ui_render_cards(FilmNode *current, const char *title, const User *user, int isFavorites);
void ui_render_film_details(const Film *film, const User *user);
char ui_read_command(void);
int ui_prompt_int(const char *prompt, int min, int max, int *out);
int ui_prompt_float(const char *prompt, float min, float max, float *out);

#endif 
