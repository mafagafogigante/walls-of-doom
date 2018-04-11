#include "io.hpp"
#include "logger.hpp"
#include "menu.hpp"
#include "random.hpp"
#include "text.hpp"
#include "version.hpp"
#include <SDL.h>
#include <cstdio>
#include <iostream>
#include <stdexcept>

enum ParserResult { PARSER_RESULT_CONTINUE, PARSER_RESULT_QUIT };

void log_unrecognized_argument(const std::string &argument) {
  const auto string = "Unrecognized argument: " + argument + ".";
  log_message(string);
}

ParserResult parse_argument(const char *argument) {
  if (string_equals(argument, "--version")) {
    printf("%s\n", WALLS_OF_DOOM_VERSION);
    return PARSER_RESULT_QUIT;
  }
  log_unrecognized_argument(argument);
  return PARSER_RESULT_QUIT;
}

/* Must be declared with parameters because of SDL 2. */
int main(int argc, char *argv[]) {
  int quit = 0;
  int result = 0;
  if (argc > 1) {
    for (int i = 1; i < argc && (quit == 0); i++) {
      if (parse_argument(argv[i]) == PARSER_RESULT_QUIT) {
        quit = 1;
      }
    }
  }
  if (quit != 0) {
    return result;
  }
  try {
    Profiler profiler(true);
    Settings settings(settings_filename);
    RandomNumberGenerator generator(default_random_number_generator_seed);
    Context context(settings, profiler, generator);
    Renderer renderer(context);
    result = main_menu(context, renderer);
  } catch (std::exception &exception) {
    std::cout << "Caught an exception." << ' ' << exception.what() << '\n';
  }
  return result;
}
