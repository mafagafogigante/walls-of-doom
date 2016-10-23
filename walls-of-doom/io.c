#include "io.h"

#include "clock.h"
#include "constants.h"
#include "game.h"
#include "logger.h"
#include "memory.h"
#include "numeric.h"
#include "physics.h"
#include "player.h"
#include "profiler.h"
#include "random.h"
#include "settings.h"
#include "text.h"

#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>

#include <ctype.h>
#include <stdio.h>
#include <string.h>

#define GAME_NAME "Walls of Doom"

#define ELLIPSIS_STRING "..."
#define ELLIPSIS_LENGTH (strlen(ELLIPSIS_STRING))
#define MINIMUM_STRING_SIZE_FOR_ELLIPSIS (2 * ELLIPSIS_LENGTH)

#define MINIMUM_BAR_HEIGHT 20

#define IMG_FLAGS IMG_INIT_PNG

static TTF_Font *global_monospaced_font = NULL;
static SDL_Texture *borders_texture = NULL;

/* Default integers to one to prevent divisions by zero. */
static int bar_height = 1;
static int window_width = 1;
static int window_height = 1;
static int global_monospaced_font_width = 1;
static int global_monospaced_font_height = 1;

/**
 * Clears the screen.
 */
void clear(SDL_Renderer *renderer) { SDL_RenderClear(renderer); }

/**
 * Updates the screen with what has been rendered.
 */
void present(SDL_Renderer *renderer) { SDL_RenderPresent(renderer); }

/**
 * In the future we may support window resizing.
 *
 * These functions encapsulate how window metrics are actually obtained.
 */
static int get_window_width(void) { return window_width; }

static int get_window_height(void) { return window_height; }

/**
 * Returns the height of the top and bottom bars.
 */
static int get_bar_height(void) { return bar_height; }

static int get_tile_width(void) { return get_window_width() / get_columns(); }

static int get_tile_height(void) {
  return (get_window_height() - get_bar_height()) / get_lines();
}

/**
 * Initializes the global fonts.
 *
 * Returns 0 in case of success.
 */
static int initialize_fonts(void) {
  char log_buffer[MAXIMUM_STRING_SIZE];
  TTF_Font *font = NULL;
  if (global_monospaced_font != NULL) {
    return 0;
  }
  /* We try to open the font if we need to initialize. */
  font = TTF_OpenFont(MONOSPACED_FONT_PATH, get_font_size());
  /* If it failed, we log an error. */
  if (font == NULL) {
    sprintf(log_buffer, "TTF font opening error: %s", SDL_GetError());
    log_message(log_buffer);
    return 1;
  } else {
    global_monospaced_font = font;
  }
  return 0;
}

/**
 * Initializes the required font metrics.
 *
 * Returns 0 in case of success.
 */
static int initialize_font_metrics(void) {
  int width;
  int height;
  TTF_Font *font = global_monospaced_font;
  if (TTF_GlyphMetrics(font, 'A', NULL, NULL, NULL, NULL, &width)) {
    log_message("Could not assess the width of a font");
    return 1;
  }
  height = TTF_FontHeight(font);
  global_monospaced_font_width = width;
  global_monospaced_font_height = height;
  return 0;
}

/**
 * Creates a new fullscreen window.
 *
 * Updates the pointers with the window width, window height, and bar height.
 */
static SDL_Window *create_window(int *width, int *height, int *bar_height) {
  const char *title = GAME_NAME;
  const int x = SDL_WINDOWPOS_CENTERED;
  const int y = SDL_WINDOWPOS_CENTERED;
  Uint32 flags = SDL_WINDOW_FULLSCREEN_DESKTOP;
  int line_height;
  /* Width and height do not matter because it is fullscreen. */
  SDL_Window *window = SDL_CreateWindow(title, x, y, 1, 1, flags);
  SDL_GetWindowSize(window, width, height);
  line_height = (*height - 2 * MINIMUM_BAR_HEIGHT) / get_lines();
  *bar_height = (*height - get_lines() * line_height) / 2;
  return window;
}

int set_window_title_and_icon(SDL_Window *window) {
  SDL_Surface *icon_surface = IMG_Load(ICON_PATH);
  SDL_SetWindowTitle(window, GAME_NAME);
  if (icon_surface == NULL) {
    log_message("Failed to load the window icon");
    return 1;
  }
  SDL_SetWindowIcon(window, icon_surface);
  SDL_FreeSurface(icon_surface);
  return 0;
}

static void set_render_color(SDL_Renderer *renderer, Color color) {
  SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
}

/**
 * Initializes the required resources.
 *
 * Should only be called once, right after starting.
 *
 * Returns 0 in case of success.
 */
int initialize(SDL_Window **window, SDL_Renderer **renderer) {
  char log_buffer[MAXIMUM_STRING_SIZE];
  int width = 1;
  int height = 1;
  initialize_logger();
  initialize_profiler();
  initialize_settings();
  /* Initialize SDL. */
  if (SDL_Init(SDL_INIT_VIDEO)) {
    sprintf(log_buffer, "SDL initialization error: %s", SDL_GetError());
    log_message(log_buffer);
    return 1;
  }
  /* Initialize TTF. */
  if (!TTF_WasInit()) {
    if (TTF_Init()) {
      sprintf(log_buffer, "TTF initialization error: %s", SDL_GetError());
      log_message(log_buffer);
      return 1;
    }
  }
  if (initialize_fonts()) {
    sprintf(log_buffer, "Failed to initialize fonts");
    log_message(log_buffer);
    return 1;
  }
  if (initialize_font_metrics()) {
    sprintf(log_buffer, "Failed to initialize font metrics");
    log_message(log_buffer);
    return 1;
  }
  if ((IMG_Init(IMG_FLAGS) & IMG_FLAGS) != IMG_FLAGS) {
    sprintf(log_buffer, "Failed to initialize required image support");
    log_message(log_buffer);
    return 1;
  }
  /**
   * The number of columns and the number of lines are fixed. However, the
   * number of pixels we need for the screen is not. We find this number by
   * experimenting before creating the window.
   */
  /* Log the size of the window we are going to create. */
  sprintf(log_buffer, "Creating a %dx%d window", width, height);
  log_message(log_buffer);
  *window = create_window(&window_width, &window_height, &bar_height);
  if (*window == NULL) {
    sprintf(log_buffer, "SDL initialization error: %s", SDL_GetError());
    log_message(log_buffer);
    return 1;
  }
  /* Must disable text input to prevent a name capture bug. */
  SDL_StopTextInput();
  set_window_title_and_icon(*window);
  *renderer = SDL_CreateRenderer(*window, -1, SDL_RENDERER_ACCELERATED);
  set_render_color(*renderer, COLOR_DEFAULT_BACKGROUND);
  clear(*renderer);
  return 0;
}

static void finalize_cached_textures(void) {
  SDL_DestroyTexture(borders_texture);
  borders_texture = NULL;
}

/**
 * Finalizes the global fonts.
 */
static void finalize_fonts(void) {
  if (global_monospaced_font != NULL) {
    TTF_CloseFont(global_monospaced_font);
    global_monospaced_font = NULL;
  }
}

/**
 * Finalizes the acquired resources.
 *
 * Should only be called once, right before exiting.
 *
 * Returns 0 in case of success.
 */
int finalize(SDL_Window **window, SDL_Renderer **renderer) {
  finalize_cached_textures();
  finalize_fonts();
  SDL_DestroyRenderer(*renderer);
  SDL_DestroyWindow(*window);
  *window = NULL;
  if (TTF_WasInit()) {
    TTF_Quit();
  }
  /* This could be called earlier, but we only do it here to organize things. */
  IMG_Quit();
  SDL_Quit();
  finalize_profiler();
  finalize_logger();
  return 0;
}

/**
 * Initializes the color schemes used to render the game.
 */
int initialize_color_schemes(void) {
  /* Nothing to do for now. */
  return 0;
}

/**
 * Evaluates whether or not a Player name is a valid name.
 *
 * A name is considered to be valid if it has at least two characters after
 * being trimmed.
 */
int is_valid_player_name(const char *player_name) {
  char buffer[MAXIMUM_PLAYER_NAME_SIZE];
  copy_string(buffer, player_name, MAXIMUM_PLAYER_NAME_SIZE);
  trim_string(buffer);
  return strlen(buffer) >= 2;
}

/**
 * Attempts to read a player name.
 *
 * Returns a Code, which may indicate that the player tried to quit.
 */
Code read_player_name(char *destination, const size_t maximum_size,
                      SDL_Renderer *renderer) {
  int x;
  int y;
  Code code = CODE_ERROR;
  int valid_name = 0;
  const char message[] = "Name your character: ";
  char log_buffer[MAXIMUM_STRING_SIZE];
  random_name(destination);
  /* While there is not a read error or a valid name. */
  while (code != CODE_OK || !valid_name) {
    x = get_padding();
    y = get_lines() / 2;
    code = read_string(x, y, message, destination, maximum_size, renderer);
    if (code == CODE_QUIT) {
      return CODE_QUIT;
    } else if (code == CODE_OK) {
      sprintf(log_buffer, "Read '%s' from the user", destination);
      log_message(log_buffer);
      /* Trim the name the user entered. */
      trim_string(destination);
      sprintf(log_buffer, "Trimmed the input to '%s'", destination);
      log_message(log_buffer);
      valid_name = is_valid_player_name(destination);
    }
  }
  return code;
}

Code print_absolute(const int x, const int y, const char *string,
                    const ColorPair color_pair, SDL_Renderer *renderer) {
  const SDL_Color foreground = to_sdl_color(color_pair.foreground);
  const SDL_Color background = to_sdl_color(color_pair.background);
  TTF_Font *font = global_monospaced_font;
  SDL_Surface *surface;
  SDL_Texture *texture;
  SDL_Rect position;
  position.x = x;
  position.y = y;
  if (string == NULL || string[0] == '\0') {
    return CODE_OK;
  }
  /* Validate that x and y are nonnegative. */
  if (x < 0 || y < 0) {
    return CODE_ERROR;
  }
  surface = TTF_RenderText_Shaded(font, string, foreground, background);
  if (surface == NULL) {
    log_message("Failed to allocate text surface in print()");
    return CODE_ERROR;
  }
  texture = SDL_CreateTextureFromSurface(renderer, surface);
  if (texture == NULL) {
    log_message("Failed to create texture from surface in print()");
    return CODE_ERROR;
  }
  /* Copy destination width and height from the texture. */
  SDL_QueryTexture(texture, NULL, NULL, &position.w, &position.h);
  SDL_RenderCopy(renderer, texture, NULL, &position);
  SDL_DestroyTexture(texture);
  SDL_FreeSurface(surface);
  return CODE_OK;
}

/**
 * Prints the provided string on the screen starting at (x, y).
 *
 * Returns 0 in case of success.
 */
int print(const int x, const int y, const char *string,
          const ColorPair color_pair, SDL_Renderer *renderer) {
  const int absolute_x = get_tile_width() * x;
  const int absolute_y = get_bar_height() + get_tile_height() * (y - 1);
  return print_absolute(absolute_x, absolute_y, string, color_pair, renderer);
}

static SDL_Texture *renderable_texture(int w, int h, SDL_Renderer *renderer) {
  const int access = SDL_TEXTUREACCESS_TARGET;
  return SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, access, w, h);
}

static Code cache_borders_texture(BoundingBox borders, SDL_Renderer *renderer) {
  const SDL_Color foreground = to_sdl_color(COLOR_PAIR_DEFAULT.foreground);
  const SDL_Color background = to_sdl_color(COLOR_PAIR_DEFAULT.background);
  const int x_step = get_tile_width();
  const int y_step = get_tile_height();
  const int min_x = borders.min_x;
  const int max_x = borders.max_x;
  const int min_y = borders.min_y;
  const int max_y = borders.max_y;
  const int full_width = (max_x - min_x + 1) * x_step;
  const int full_height = (max_y - min_y + 1) * y_step;
  SDL_Surface *surface;
  SDL_Texture *glyph_texture;
  SDL_Texture *full_texture;
  SDL_Rect position;
  TTF_Font *font = global_monospaced_font;
  int x;
  int y;
  if (borders_texture != NULL) {
    return CODE_ERROR;
  }
  if (min_x < 0 || min_y < 0 || min_x > max_x || min_y > max_y) {
    log_message("Got invalid border limits");
    return CODE_ERROR;
  }
  /* All preconditions are valid. */
  /* Now we create a texture for a single glyph. */
  surface = TTF_RenderGlyph_Shaded(font, '+', foreground, background);
  glyph_texture = SDL_CreateTextureFromSurface(renderer, surface);
  /* Free surface now because we may return before the end. */
  SDL_FreeSurface(surface);
  /* Create the target texture. */
  full_texture = renderable_texture(full_width, full_height, renderer);
  if (!full_texture) {
    log_message("Failed to create cached borders texture");
    return CODE_ERROR;
  }
  /* Change the renderer target to the texture which will be cached. */
  SDL_SetRenderTarget(renderer, full_texture);
  SDL_RenderClear(renderer);
  /* Copy destination width and height from the texture. */
  SDL_QueryTexture(glyph_texture, NULL, NULL, &position.w, &position.h);
  /*
   * This is the optimized step.
   *
   * We have a texture of a single symbol, so we just copy it to all the places
   * where the symbol should appear.
   */
  /* Write the top and bottom borders. */
  for (x = 0; x < full_width; x += x_step) {
    position.x = x;
    position.y = 0;
    SDL_RenderCopy(renderer, glyph_texture, NULL, &position);
    position.y = full_height - y_step;
    SDL_RenderCopy(renderer, glyph_texture, NULL, &position);
  }
  /* Write the left and right borders. */
  for (y = 0; y < full_height; y += y_step) {
    position.y = y;
    position.x = 0;
    SDL_RenderCopy(renderer, glyph_texture, NULL, &position);
    position.x = full_width - x_step;
    SDL_RenderCopy(renderer, glyph_texture, NULL, &position);
  }
  /* Change the renderer target back to the window. */
  SDL_DestroyTexture(glyph_texture);
  SDL_SetRenderTarget(renderer, NULL);
  /* Note that the texture is not destroyed here (obviously). */
  borders_texture = full_texture;
  return CODE_OK;
}

static Code render_borders(BoundingBox borders, SDL_Renderer *renderer) {
  const int x_step = get_tile_width();
  const int y_step = get_tile_height();
  SDL_Rect pos;
  pos.x = borders.min_x * x_step;
  pos.y = bar_height + borders.min_y * y_step;
  if (borders_texture == NULL) {
    cache_borders_texture(borders, renderer);
    if (borders_texture == NULL) {
      /* Failed to cache the texture. */
      return CODE_ERROR;
    }
  }
  SDL_QueryTexture(borders_texture, NULL, NULL, &pos.w, &pos.h);
  SDL_RenderCopy(renderer, borders_texture, NULL, &pos);
  return CODE_OK;
}

/**
 * Prints the provided string centered on the screen at the provided line.
 */
Code print_centered(const int y, const char *string, const ColorPair color_pair,
                    SDL_Renderer *renderer) {
  const SDL_Color foreground = to_sdl_color(color_pair.foreground);
  const SDL_Color background = to_sdl_color(color_pair.background);
  TTF_Font *font = global_monospaced_font;
  SDL_Surface *surface;
  SDL_Texture *texture;
  SDL_Rect position;
  position.x = 0;
  position.y = get_tile_height() * y;
  /* Validate that x and y are nonnegative. */
  if (y < 0) {
    return CODE_ERROR;
  }
  surface = TTF_RenderText_Shaded(font, string, foreground, background);
  if (surface == NULL) {
    log_message("Failed to allocate text surface in print()");
    return CODE_ERROR;
  }
  texture = SDL_CreateTextureFromSurface(renderer, surface);
  if (texture == NULL) {
    log_message("Failed to create texture from surface in print()");
    return CODE_ERROR;
  }
  /* Copy destination width and height from the texture. */
  SDL_QueryTexture(texture, NULL, NULL, &position.w, &position.h);
  position.x = (get_window_width() - position.w) / 2;
  SDL_RenderCopy(renderer, texture, NULL, &position);
  SDL_DestroyTexture(texture);
  SDL_FreeSurface(surface);
  return CODE_OK;
}

/**
 * Prints the provided strings centered at the specified absolute line.
 */
Code print_centered_horizontally(const int y, const int string_count,
                                 const char *const *strings,
                                 const ColorPair color_pair,
                                 SDL_Renderer *renderer) {
  const SDL_Color foreground = to_sdl_color(color_pair.foreground);
  const SDL_Color background = to_sdl_color(color_pair.background);
  const int slice_size = get_window_width() / string_count;
  TTF_Font *font = global_monospaced_font;
  SDL_Surface *surface;
  SDL_Texture *texture;
  SDL_Rect position;
  int i;
  position.x = 0;
  position.y = y;
  /* Validate that x and y are nonnegative. */
  if (y < 0) {
    return CODE_ERROR;
  }
  for (i = 0; i < string_count; i++) {
    surface = TTF_RenderText_Shaded(font, strings[i], foreground, background);
    if (surface == NULL) {
      log_message("Failed to allocate text surface in print()");
      return CODE_ERROR;
    }
    texture = SDL_CreateTextureFromSurface(renderer, surface);
    if (texture == NULL) {
      log_message("Failed to create texture from surface in print()");
      return CODE_ERROR;
    }
    /* Copy destination width and height from the texture. */
    SDL_QueryTexture(texture, NULL, NULL, &position.w, &position.h);
    position.x = i * slice_size + (slice_size - position.w) / 2;
    SDL_RenderCopy(renderer, texture, NULL, &position);
    SDL_DestroyTexture(texture);
    SDL_FreeSurface(surface);
  }
  return CODE_OK;
}

/**
 * Prints the provided strings centered in the middle of the screen.
 */
Code print_centered_vertically(int string_count, const char *const *strings,
                               const ColorPair color_pair,
                               SDL_Renderer *renderer) {
  const int text_line_height = global_monospaced_font_height;
  const int text_lines_limit = get_window_height() / text_line_height;
  int y;
  int i;
  if (string_count > text_lines_limit) {
    string_count = text_lines_limit;
  }
  y = (get_window_height() - string_count * text_line_height) / 2;
  for (i = 0; i < string_count; i++) {
    print_centered_horizontally(y, 1, strings + i, color_pair, renderer);
    y += text_line_height;
  }
  return CODE_OK;
}

/**
 * Replaces the first line break of any sequence of line breaks by a space.
 */
static void remove_first_breaks(char *string) {
  char c;
  size_t i;
  int preserve = 0;
  for (i = 0; string[i] != '\0'; i++) {
    c = string[i];
    if (c == '\n') {
      if (!preserve) {
        string[i] = ' ';
        /* Stop erasing newlines. */
        preserve = 1;
      }
    } else {
      /* Not a newline, we can erase again. */
      preserve = 0;
    }
  }
}

int count_lines(char *const buffer) {
  size_t counter = 0;
  size_t i = 0;
  while (buffer[i] != '\0') {
    if (buffer[i] == '\n') {
      counter++;
    }
    i++;
  }
  return counter;
}

char *copy_first_line(char *source, char *destination) {
  while (*source != '\0' && *source != '\n') {
    *destination++ = *source++;
  }
  *destination = '\0';
  if (*source == '\0') {
    return source;
  } else {
    return source + 1;
  }
}

/**
 * Prints the provided string after formatting it to increase readability.
 */
void print_long_text(char *string, SDL_Renderer *renderer) {
  const int font_width = global_monospaced_font_width;
  const int width = get_window_width() - 2 * get_padding() * font_width;
  TTF_Font *font = global_monospaced_font;
  SDL_Surface *surface;
  SDL_Texture *texture;
  SDL_Color color = to_sdl_color(COLOR_DEFAULT_FOREGROUND);
  SDL_Rect position;
  position.x = get_padding() * font_width;
  position.y = get_padding() * font_width;
  remove_first_breaks(string);
  clear(renderer);
  /* Validate that the string is not empty and that x and y are nonnegative. */
  if (string == NULL || string[0] == '\0') {
    return;
  }
  surface = TTF_RenderText_Blended_Wrapped(font, string, color, width);
  if (surface == NULL) {
    log_message("Failed to allocate text surface in print()");
    return;
  }
  texture = SDL_CreateTextureFromSurface(renderer, surface);
  if (texture == NULL) {
    log_message("Failed to create texture from surface in print()");
    return;
  }
  /* Copy destination width and height from the texture. */
  SDL_QueryTexture(texture, NULL, NULL, &position.w, &position.h);
  SDL_RenderCopy(renderer, texture, NULL, &position);
  SDL_DestroyTexture(texture);
  SDL_FreeSurface(surface);
  present(renderer);
}

/**
 * Draws an absolute rectangle based on the provided coordinates.
 */
static void draw_absolute_rectangle(const int x, const int y, const int w,
                                    const int h, Color color,
                                    SDL_Renderer *renderer) {
  SDL_Color helper;
  SDL_Rect rectangle;
  rectangle.x = x;
  rectangle.y = y;
  rectangle.w = w;
  rectangle.h = h;
  SDL_GetRenderDrawColor(renderer, &helper.r, &helper.g, &helper.b, &helper.a);
  SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
  SDL_RenderFillRect(renderer, &rectangle);
  SDL_SetRenderDrawColor(renderer, helper.r, helper.g, helper.b, helper.a);
}

/**
 * Draws a relative rectangle based on the provided coordinates.
 */
static void draw_rectangle(int x, int y, int w, int h, Color color,
                           SDL_Renderer *renderer) {
  x = x * get_tile_width();
  y = get_bar_height() + y * get_tile_height();
  w = w * get_tile_width();
  h = h * get_tile_height();
  draw_absolute_rectangle(x, y, w, h, color, renderer);
}

static void write_top_bar_strings(const char *strings[],
                                  SDL_Renderer *renderer) {
  const ColorPair color_pair = COLOR_PAIR_TOP_BAR;
  const int string_count = TOP_BAR_STRING_COUNT;
  const int y = (get_bar_height() - global_monospaced_font_height) / 2;
  int h = get_bar_height();
  int w = get_window_width();
  draw_absolute_rectangle(0, 0, w, h, color_pair.background, renderer);
  print_centered_horizontally(y, string_count, strings, color_pair, renderer);
}

/**
 * Draws the top status bar on the screen for a given Player.
 *
 * Returns 0 if successful.
 */
int draw_top_bar(const Player *const player, SDL_Renderer *renderer) {
  char lives_buffer[MAXIMUM_STRING_SIZE];
  char score_buffer[MAXIMUM_STRING_SIZE];
  const char *strings[TOP_BAR_STRING_COUNT];
  const char *perk_name = "No Power";
  if (player->perk != PERK_NONE) {
    perk_name = get_perk_name(player->perk);
  }
  sprintf(lives_buffer, "Lives: %d", player->lives);
  sprintf(score_buffer, "Score: %d", player->score);
  strings[0] = GAME_NAME;
  strings[1] = perk_name;
  strings[2] = lives_buffer;
  strings[3] = score_buffer;
  write_top_bar_strings((const char **)strings, renderer);
  return 0;
}

static void write_bottom_bar_string(const char *string,
                                    SDL_Renderer *renderer) {
  /* Use half a character for horizontal padding. */
  const int x = global_monospaced_font_width / 2;
  const int bar_start = get_window_height() - get_bar_height();
  const int padding = (get_bar_height() - global_monospaced_font_height) / 2;
  const int y = bar_start + padding;
  print_absolute(x, y, string, COLOR_PAIR_BOTTOM_BAR, renderer);
}

/*
 * Draws the bottom status bar on the screen for a given Player.
 */
void draw_bottom_bar(const char *message, SDL_Renderer *renderer) {
  const Color color = COLOR_PAIR_BOTTOM_BAR.background;
  const int y = get_window_height() - bar_height;
  const int w = get_window_width();
  const int h = get_bar_height();
  draw_absolute_rectangle(0, y, w, h, color, renderer);
  write_bottom_bar_string(message, renderer);
}

/**
 * Draws the borders of the screen.
 */
void draw_borders(SDL_Renderer *renderer) {
  BoundingBox borders;
  borders.min_x = 0;
  borders.max_x = get_columns() - 1;
  borders.min_y = 0;
  borders.max_y = get_lines() - 1;
  render_borders(borders, renderer);
}

int draw_platforms(const Platform *platforms, const size_t platform_count,
                   const BoundingBox *box, SDL_Renderer *renderer) {
  Platform p;
  int x;
  int y;
  int w;
  size_t i;
  for (i = 0; i < platform_count; i++) {
    p = platforms[i];
    x = max(box->min_x, p.x);
    y = p.y;
    w = min(box->max_x, p.x + p.width - 1) - x + 1;
    draw_rectangle(x, y, w, 1, COLOR_PAIR_PLATFORM.foreground, renderer);
  }
  return 0;
}

int has_active_perk(const Game *const game) { return game->perk != PERK_NONE; }

int draw_perk(const Game *const game, SDL_Renderer *renderer) {
  const Color color = COLOR_PAIR_PERK.background;
  if (has_active_perk(game)) {
    draw_rectangle(game->perk_x, game->perk_y, 1, 1, color, renderer);
  }
  return 0;
}

Code draw_player(const Player *const player, SDL_Renderer *renderer) {
  const int x = player->x;
  const int y = player->y;
  draw_rectangle(x, y, 1, 1, COLOR_PAIR_PLAYER.foreground, renderer);
  return CODE_OK;
}

/**
 * Draws a full game to the screen.
 *
 * Returns a Milliseconds approximation of the time this function took.
 */
Milliseconds draw_game(const Game *const game, SDL_Renderer *renderer) {
  Milliseconds draw_game_start = get_milliseconds();
  Milliseconds start;

  start = get_milliseconds();
  clear(renderer);
  update_profiler("draw_game:clear", get_milliseconds() - start);

  start = get_milliseconds();
  draw_top_bar(game->player, renderer);
  update_profiler("draw_game:draw_top_bar", get_milliseconds() - start);

  start = get_milliseconds();
  draw_bottom_bar(game->message, renderer);
  update_profiler("draw_game:draw_bottom_bar", get_milliseconds() - start);

  start = get_milliseconds();
  draw_borders(renderer);
  update_profiler("draw_game:draw_borders", get_milliseconds() - start);

  start = get_milliseconds();
  draw_platforms(game->platforms, game->platform_count, game->box, renderer);
  update_profiler("draw_game:draw_platforms", get_milliseconds() - start);

  start = get_milliseconds();
  draw_perk(game, renderer);
  update_profiler("draw_game:draw_perk", get_milliseconds() - start);

  start = get_milliseconds();
  draw_player(game->player, renderer);
  update_profiler("draw_game:draw_player", get_milliseconds() - start);

  start = get_milliseconds();
  present(renderer);
  update_profiler("draw_game:present", get_milliseconds() - start);

  update_profiler("draw_game", get_milliseconds() - draw_game_start);
  return get_milliseconds() - draw_game_start;
}

void print_game_result(const char *name, const unsigned int score,
                       const int position, SDL_Renderer *renderer) {
  const ColorPair color = COLOR_PAIR_DEFAULT;
  char first_line[MAXIMUM_STRING_SIZE];
  char second_line[MAXIMUM_STRING_SIZE];
  sprintf(first_line, "%s died after making %d points.", name, score);
  if (position > 0) {
    sprintf(second_line, "%s got to position %d!", name, position);
  } else {
    sprintf(second_line, "%s didn't make it to the top scores.", name);
  }
  clear(renderer);
  print_centered(get_lines() / 2 - 1, first_line, color, renderer);
  print_centered(get_lines() / 2 + 1, second_line, color, renderer);
  present(renderer);
}

/**
 * Returns a BoundingBox that represents the playable box.
 */
BoundingBox bounding_box_from_screen(void) {
  BoundingBox box;
  box.min_x = 1;
  box.min_y = 1;
  box.max_x = get_columns() - 2;
  box.max_y = get_lines() - 2;
  return box;
}

/**
 * Returns the Command value corresponding to the provided input code.
 */
Command command_from_event(const SDL_Event event) {
  SDL_Keycode keycode;
  if (event.type == SDL_QUIT) {
    return COMMAND_CLOSE;
  }
  if (event.type == SDL_KEYDOWN) {
    keycode = event.key.keysym.sym;
    if (keycode == SDLK_KP_8 || keycode == SDLK_UP) {
      return COMMAND_UP;
    } else if (keycode == SDLK_KP_4 || keycode == SDLK_LEFT) {
      return COMMAND_LEFT;
    } else if (keycode == SDLK_KP_5) {
      return COMMAND_CENTER;
    } else if (keycode == SDLK_KP_6 || keycode == SDLK_RIGHT) {
      return COMMAND_RIGHT;
    } else if (keycode == SDLK_KP_2 || keycode == SDLK_DOWN) {
      return COMMAND_DOWN;
    } else if (keycode == SDLK_SPACE) {
      return COMMAND_JUMP;
    } else if (keycode == SDLK_RETURN || keycode == SDLK_KP_ENTER) {
      return COMMAND_ENTER;
    } else if (keycode == SDLK_c) {
      return COMMAND_CONVERT;
    } else if (keycode == SDLK_q) {
      return COMMAND_QUIT;
    }
  }
  return COMMAND_NONE;
}

/**
 * Asserts whether or not a character is a valid input character.
 *
 * For simplicity, the user should only be able to enter letters and numbers.
 */
int is_valid_input_character(char c) { return isalnum(c); }

/**
 * Prints a string starting from (x, y) but limits to its first limit
 * characters.
 */
static void print_limited(const int x, const int y, const char *string,
                          const size_t limit, SDL_Renderer *renderer) {
  const size_t string_length = strlen(string);
  /* No-op. */
  if (limit < 1) {
    return;
  }
  /*
   * As only two calls to print suffice to solve this problem for any possible
   * input size, we avoid using a dynamically allocated buffer to prepare the
   * output.
   */
  /* String length is less than the limit. */
  if (string_length < limit) {
    print(x, y, string, COLOR_PAIR_DEFAULT, renderer);
    return;
  }
  /* String is longer than the limit. */
  /* Write the ellipsis if we need to. */
  if (limit >= MINIMUM_STRING_SIZE_FOR_ELLIPSIS) {
    print(x, y, ELLIPSIS_STRING, COLOR_PAIR_DEFAULT, renderer);
  }
  /* Write the tail of the input string. */
  string += string_length - limit + ELLIPSIS_LENGTH;
  print(x + ELLIPSIS_LENGTH, y, string, COLOR_PAIR_DEFAULT, renderer);
}

/**
 * Reads a string from the user of up to size characters (including NUL).
 *
 * The string will be echoed after the prompt, which starts at (x, y).
 */
Code read_string(const int x, const int y, const char *prompt,
                 char *destination, const size_t size, SDL_Renderer *renderer) {
  const int buffer_x = x + strlen(prompt) + 1;
  const int buffer_view_limit = get_columns() - get_padding() - buffer_x;
  int is_done = 0;
  int should_rerender = 1;
  /* The x coordinate of the user input buffer. */
  size_t written = strlen(destination);
  char character = '\0';
  char *write = destination + written;
  SDL_Event event;
  /* Start listening for text input. */
  SDL_StartTextInput();
  while (!is_done) {
    if (should_rerender) {
      clear(renderer);
      print(x, y, prompt, COLOR_PAIR_DEFAULT, renderer);
      if (written == 0) {
        /* We must write a single space, or SDL will not render anything. */
        print(buffer_x, y, " ", COLOR_PAIR_DEFAULT, renderer);
      } else {
        /*
         * Must care about how much we write, padding should be respected.
         *
         * Simply enough, we print the tail of the string and omit the
         * beginning with an ellipsis.
         *
         */
        print_limited(buffer_x, y, destination, buffer_view_limit, renderer);
      }
      present(renderer);
      should_rerender = 0;
    }
    /* Throughout the loop, the write pointer always points to a '\0'. */
    if (SDL_WaitEvent(&event)) {
      /* Check for user quit and return 1. */
      /* This is OK because the destination string is always a valid C string.
       */
      if (event.type == SDL_QUIT) {
        return CODE_QUIT;
      } else if (event.type == SDL_KEYDOWN) {
        /* Handle backspace. */
        if (event.key.keysym.sym == SDLK_BACKSPACE && written > 0) {
          write--;
          *write = '\0';
          written--;
          should_rerender = 1;
        } else if (event.key.keysym.sym == SDLK_RETURN) {
          is_done = 1;
        }
      } else if (event.type == SDL_TEXTINPUT) {
        character = event.text.text[0];
        if (is_valid_input_character(character) && written + 1 < size) {
          *write = character;
          write++;
          written++;
          /* Terminate the string with NIL to ensure it is valid. */
          *write = '\0';
          should_rerender = 1;
        }
        /* We could handle copying and pasting by checking for Control-C. */
        /* SDL_*ClipboardText() allows us to access the clipboard text. */
      }
    }
  }
  /* Stop listening for text input. */
  SDL_StopTextInput();
  return CODE_OK;
}

/**
 * Reads the next command that needs to be processed.
 *
 * This is the last pending command.
 *
 * This function consumes the whole input buffer and returns either
 * COMMAND_NONE (if no other Command could be produced by what was in the input
 * buffer) or the last Command different than COMMAND_NONE that could be
 * produced by what was in the input buffer.
 */
Command read_next_command(void) {
  Command last_valid_command = COMMAND_NONE;
  Command current;
  SDL_Event event;
  while (SDL_PollEvent(&event)) {
    current = command_from_event(event);
    if (current != COMMAND_NONE) {
      last_valid_command = current;
    }
  }
  return last_valid_command;
}

/**
 * Waits for the next command, blocking indefinitely.
 */
Command wait_for_next_command(void) {
  Command command;
  SDL_Event event;
  int got_command = 0;
  while (!got_command) {
    if (SDL_WaitEvent(&event)) {
      command = command_from_event(event);
      got_command = command != COMMAND_NONE;
    }
  }
  return command;
}

/**
 * Waits for any user input, blocking indefinitely.
 */
Code wait_for_input(void) {
  SDL_Event event;
  while (1) {
    if (SDL_WaitEvent(&event)) {
      if (event.type == SDL_QUIT) {
        return CODE_QUIT;
      }
      if (event.type == SDL_KEYDOWN) {
        return CODE_OK;
      }
    } else {
      /* WaitEvent returns 0 to indicate errors. */
      return CODE_ERROR;
    }
  }
}
