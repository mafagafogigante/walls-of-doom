#define CATCH_CONFIG_MAIN

#include "catch/catch.hpp"
#include "sources/data.hpp"
#include "sources/high-io.hpp"
#include "sources/logger.hpp"
#include "sources/memory.hpp"
#include "sources/numeric.hpp"
#include "sources/random.hpp"
#include "sources/sort.hpp"
#include "sources/text.hpp"
#include <climits>
#include <cstdlib>
#include <cstring>
#include <ctime>

#define SMALL_STRING_BUFFER_SIZE 64

#define LARGE_STRING_BUFFER_SIZE 2048

#define RESIZE_MEMORY_SIZE 4096

#define WRAP_TEST_SOURCE "assets/tests/wrap-test-source.txt"
#define WRAP_TEST_WIDTH_10 "assets/tests/wrap-test-width-10.txt"
#define WRAP_TEST_WIDTH_20 "assets/tests/wrap-test-width-20.txt"
#define WRAP_TEST_WIDTH_40 "assets/tests/wrap-test-width-40.txt"
#define WRAP_TEST_WIDTH_80 "assets/tests/wrap-test-width-80.txt"

TEST_CASE("resize_memory()") {
  unsigned char *chunk = nullptr;
  REQUIRE(chunk == NULL);
  chunk = reinterpret_cast<unsigned char *>(resize_memory(chunk, RESIZE_MEMORY_SIZE));
  REQUIRE(chunk != NULL);
  /* If allocation worked, we can write to this chunk. */
  memset(chunk, 255, RESIZE_MEMORY_SIZE);
  chunk = reinterpret_cast<unsigned char *>(resize_memory(chunk, 0));
  REQUIRE(chunk == NULL);
}

TEST_CASE("normalize()") {
  REQUIRE(-1 == normalize(INT_MIN));
  REQUIRE(-1 == normalize(-1));
  REQUIRE(0 == normalize(0));
  REQUIRE(1 == normalize(1));
  REQUIRE(1 == normalize(INT_MAX));
}

TEST_CASE("get_random_perk() is well distributed") {
  const int maximum_allowed_deviation = 1 << 10;
  const int average = 1 << 16;
  const int test_count = average * PERK_COUNT;

  int counters[PERK_COUNT] = {0};

  int minimum = INT_MAX;
  int maximum = INT_MIN;
  int maximum_deviation = 0;
  double total = 0;

  int count;
  int i;

  seed_random();

  for (i = 0; i < test_count; i++) {
    counters[get_random_perk()]++;
  }
  /* Assess the distribution of the values. */
  for (i = 0; i < PERK_COUNT; i++) {
    count = counters[i];
    minimum = min_int(minimum, count);
    maximum = max_int(maximum, count);
    maximum_deviation = max_int(maximum_deviation, abs(count - average));
    total += count;
  }
  if (maximum_deviation > maximum_allowed_deviation) {
    FAIL("Maximum deviation is bigger than the allowed maximum.");
  }
}

TEST_CASE("get_full_path() checks for buffer overflow") {
  char buffer[MAXIMUM_PATH_SIZE];
  char big_filename[MAXIMUM_PATH_SIZE];
  Code code;
  memset(big_filename, 'A', MAXIMUM_PATH_SIZE);
  big_filename[MAXIMUM_PATH_SIZE - 1] = '\0';
  code = get_full_path(buffer, big_filename);
  REQUIRE(CODE_ERROR == code);
}

TEST_CASE("trim_string() works with empty strings") {
  const char *input = "";
  const char *expected = input;
  char buffer[SMALL_STRING_BUFFER_SIZE];
  copy_string(buffer, input, SMALL_STRING_BUFFER_SIZE);
  trim_string(buffer);
  REQUIRE(string_equals(expected, buffer));
}

TEST_CASE("trim_string() works with already trimmed strings") {
  const char *input = "a b";
  const char *expected = input;
  char buffer[SMALL_STRING_BUFFER_SIZE];
  copy_string(buffer, input, SMALL_STRING_BUFFER_SIZE);
  trim_string(buffer);
  REQUIRE(string_equals(expected, buffer));
}

TEST_CASE("trim_string() properly trims preceding spaces") {
  const char *input = "  a b";
  const char *expected = "a b";
  char buffer[SMALL_STRING_BUFFER_SIZE];
  copy_string(buffer, input, SMALL_STRING_BUFFER_SIZE);
  trim_string(buffer);
  REQUIRE(string_equals(expected, buffer));
}

TEST_CASE("trim_string() properly trims trailing spaces") {
  const char *input = "a b  ";
  const char *expected = "a b";
  char buffer[SMALL_STRING_BUFFER_SIZE];
  copy_string(buffer, input, SMALL_STRING_BUFFER_SIZE);
  trim_string(buffer);
  REQUIRE(string_equals(expected, buffer));
}

TEST_CASE("trim_string() properly trims space padded strings") {
  const char *input = "  a b  ";
  const char *expected = "a b";
  char buffer[SMALL_STRING_BUFFER_SIZE];
  copy_string(buffer, input, SMALL_STRING_BUFFER_SIZE);
  trim_string(buffer);
  REQUIRE(string_equals(expected, buffer));
}

TEST_CASE("trim_string() works with strings of whitespaces") {
  const char *input = " \t \n ";
  const char *expected = "";
  char buffer[SMALL_STRING_BUFFER_SIZE];
  copy_string(buffer, input, SMALL_STRING_BUFFER_SIZE);
  trim_string(buffer);
  REQUIRE(string_equals(expected, buffer));
}

TEST_CASE("wrap_at_right_margin() with width 10") {
  char expected[LARGE_STRING_BUFFER_SIZE];
  char actual[LARGE_STRING_BUFFER_SIZE];
  read_characters(WRAP_TEST_SOURCE, actual, LARGE_STRING_BUFFER_SIZE);
  REQUIRE(strlen(actual) != 0);
  read_characters(WRAP_TEST_WIDTH_10, expected, LARGE_STRING_BUFFER_SIZE);
  wrap_at_right_margin(actual, 10);
  REQUIRE(string_equals(expected, actual));
}

TEST_CASE("wrap_at_right_margin() with width 20") {
  char expected[LARGE_STRING_BUFFER_SIZE];
  char actual[LARGE_STRING_BUFFER_SIZE];
  read_characters(WRAP_TEST_SOURCE, actual, LARGE_STRING_BUFFER_SIZE);
  REQUIRE(strlen(actual) != 0);
  read_characters(WRAP_TEST_WIDTH_20, expected, LARGE_STRING_BUFFER_SIZE);
  wrap_at_right_margin(actual, 20);
  REQUIRE(string_equals(expected, actual));
}

TEST_CASE("wrap_at_right_margin() with width 40") {
  char expected[LARGE_STRING_BUFFER_SIZE];
  char actual[LARGE_STRING_BUFFER_SIZE];
  read_characters(WRAP_TEST_SOURCE, actual, LARGE_STRING_BUFFER_SIZE);
  REQUIRE(strlen(actual) != 0);
  read_characters(WRAP_TEST_WIDTH_40, expected, LARGE_STRING_BUFFER_SIZE);
  wrap_at_right_margin(actual, 40);
  REQUIRE(string_equals(expected, actual));
}

TEST_CASE("wrap_at_right_margin() with width 80") {
  char expected[LARGE_STRING_BUFFER_SIZE];
  char actual[LARGE_STRING_BUFFER_SIZE];
  read_characters(WRAP_TEST_SOURCE, actual, LARGE_STRING_BUFFER_SIZE);
  REQUIRE(strlen(actual) != 0);
  read_characters(WRAP_TEST_WIDTH_80, expected, LARGE_STRING_BUFFER_SIZE);
  wrap_at_right_margin(actual, 80);
  REQUIRE(string_equals(expected, actual));
}

TEST_CASE("wrap_at_right_margin() with maximum width") {
  char expected[LARGE_STRING_BUFFER_SIZE];
  char actual[LARGE_STRING_BUFFER_SIZE];
  read_characters(WRAP_TEST_SOURCE, actual, LARGE_STRING_BUFFER_SIZE);
  REQUIRE(strlen(actual) != 0);
  /* This should NOT wrap the input, even on 32-bit implementations. */
  /* This is wrapping to (at least) 4,294,967,295 columns. */
  copy_string(expected, actual, LARGE_STRING_BUFFER_SIZE);
  wrap_at_right_margin(actual, SIZE_MAX);
  REQUIRE(string_equals(expected, actual));
}

TEST_CASE("find_start_of_text() with an empty string") {
  char string[] = "";
  REQUIRE(string == find_start_of_text(string));
}

TEST_CASE("find_start_of_text() with only spaces") {
  char string[] = " \t\r\n";
  REQUIRE(string + strlen(string) == find_start_of_text(string));
}

TEST_CASE("find_start_of_text() with a single word") {
  char string[] = "string";
  REQUIRE(string == find_start_of_text(string));
}

TEST_CASE("find_start_of_text() with leading spaces") {
  char string[] = " \t\r\n string";
  REQUIRE(string + 5 == find_start_of_text(string));
}

TEST_CASE("find_end_of_text() with an empty string") {
  char string[] = "";
  REQUIRE(string == find_end_of_text(string));
}

TEST_CASE("find_end_of_text() with only spaces") {
  char string[] = " \t\r\n";
  REQUIRE(string == find_end_of_text(string));
}

TEST_CASE("find_end_of_text() with a single word") {
  char string[] = "string";
  REQUIRE(string + 6 == find_end_of_text(string));
}

TEST_CASE("find_end_of_text() with trailing spaces") {
  char string[] = "string \t\r\n ";
  REQUIRE(string + 6 == find_end_of_text(string));
}

TEST_CASE("read_integers()") {
  char filename[] = "test_read_integers_file.txt";
  const int expected_count = 1;
  const int expected_value = 65535;
  int actual_values[1];
  int actual_count;
  FILE *file;
  file = fopen(filename, "w+");
  if (file == nullptr) {
    FAIL("Failed to create helper test file.");
  }
  fprintf(file, "%d", expected_value);
  fclose(file);
  actual_count = read_integers(filename, actual_values, expected_count);
  REQUIRE(expected_count == actual_count);
  REQUIRE(expected_value == actual_values[0]);
  remove(filename);
}

TEST_CASE("bounding_box_equals()") {
  const int combinations = 1 << 4;
  BoundingBox box_i;
  BoundingBox box_j;
  int i;
  int j;
  for (i = 0; i < combinations; i++) {
    box_i.min_x = i >> 0 & 1;
    box_i.max_x = i >> 1 & 1;
    box_i.min_y = i >> 2 & 1;
    box_i.max_y = i >> 3 & 1;
    for (j = 0; j < combinations; j++) {
      box_j.min_x = j >> 0 & 1;
      box_j.max_x = j >> 1 & 1;
      box_j.min_y = j >> 2 & 1;
      box_j.max_y = j >> 3 & 1;
      if (i == j) {
        REQUIRE(box_i == box_j);
      } else {
        REQUIRE(box_i != box_j);
      }
    }
  }
}

TEST_CASE("generate_platforms() avoids multiple platforms on the same line") {
  const size_t platform_count = 128;
  int *y_counter = nullptr;
  BoundingBox box;
  size_t i;
  int y;
  y_counter = reinterpret_cast<int *>(resize_memory(y_counter, sizeof(int) * platform_count));
  for (i = 0; i < platform_count; i++) {
    y_counter[i] = 0;
  }
  box.min_x = 0;
  box.min_y = 0;
  box.max_x = platform_count - 1;
  box.max_y = platform_count - 1;
  auto platforms = generate_platforms(box, platform_count, 1, 1);
  /* Each platform in platforms should have a different y coordinate. */
  for (i = 0; i < platform_count; i++) {
    y = platforms[i].y;
    /* Casting is safe because y is nonnegative at that point. */
    if (y >= 0 && static_cast<size_t>(y) < platform_count) {
      if (y_counter[y] != 0) {
        FAIL("Two of more platforms on the same line.");
      } else {
        y_counter[y]++;
      }
    } else {
      FAIL("Platform has invalid y coordinate.");
    }
  }
  resize_memory(y_counter, 0);
}

TEST_CASE("find_next_power_of_two() works for zero") { REQUIRE(find_next_power_of_two(0) == 1); }

TEST_CASE("find_next_power_of_two() works for positive integers") {
  const unsigned long one = 1;
  unsigned long input;
  unsigned long expected;
  int i;
  /* Do not get to 31, because then the loop evaluates 1 << 32. */
  for (i = 0; i < 31; i++) {
    expected = one << (i + 1);
    /* Test the next power for the current power. */
    input = one << i;
    REQUIRE(expected == find_next_power_of_two(input));
    /* Test the next power for the next power minus one. */
    input = (one << (i + 1)) - 1;
    REQUIRE(expected == find_next_power_of_two(input));
  }
}

TEST_CASE("random_integer() respects the provided range") {
  const int min_min = -3;
  const int max_min = 0;
  const int min_max = 0;
  const int max_max = 4;
  int min;
  int max;
  int random_result;
  for (min = min_min; min < max_min; min++) {
    for (max = min_max; max < max_max; max++) {
      random_result = random_integer(min, max);
      REQUIRE((min <= random_result && random_result <= max));
    }
  }
}

TEST_CASE("random_integer() is evenly distributed") {
  /* Use primes for the boundaries as this may help show biases. */
  const int minimum = -227;
  const int maximum = 233;
  const int values = 227 + 1 + 233;
  const int expected_count = 1 << 8;
  const int minimum_allowed_count = 1 << 7;
  const int maximum_allowed_count = expected_count + minimum_allowed_count;
  int counters[227 + 1 + 233] = {0};
  int random_result;
  int i;
  for (i = 0; i < values * expected_count; i++) {
    random_result = random_integer(minimum, maximum);
    counters[random_result - minimum]++;
  }
  for (i = 0; i < values; i++) {
    if (counters[i] < minimum_allowed_count) {
      FAIL("Counter is below minimum allowed count.");
    } else if (counters[i] > maximum_allowed_count) {
      FAIL("Counter is above maximum allowed count.");
    }
  }
}

TEST_CASE("select_random_line_blindly() with one empty line") {
  const int tests = 1 << 8;
  const unsigned char array[1] = {0};
  int i;
  for (i = 0; i < tests; i++) {
    REQUIRE(0 == select_random_line_blindly(array, 1));
  }
}

TEST_CASE("select_random_line_blindly() with two empty lines") {
  /* Should not overflow after a multiplication by 3. */
  const int tests = 1 << 10;
  const int seven_sixteenths = 7 * tests / 16;
  const unsigned char array[2] = {0, 0};
  int counters[2] = {0, 0};
  int i;
  for (i = 0; i < tests; i++) {
    counters[select_random_line_blindly(array, 2)] += 1;
  }
  /* Counters should be roughly the same. */
  REQUIRE(counters[0] > seven_sixteenths);
  REQUIRE(counters[1] > seven_sixteenths);
}

TEST_CASE("select_random_line_blindly() with three empty lines") {
  const int tests = 1 << 12;
  const int five_sixteenths = 5 * tests / 16;
  const unsigned char array[3] = {0, 0, 0};
  int counters[3] = {0, 0, 0};
  int i;
  for (i = 0; i < tests; i++) {
    counters[select_random_line_blindly(array, 3)] += 1;
  }
  /* Counters should be roughly the same. */
  REQUIRE(counters[0] > five_sixteenths);
  REQUIRE(counters[1] > five_sixteenths);
  REQUIRE(counters[2] > five_sixteenths);
}

TEST_CASE("select_random_line_blindly() with one occupied line") {
  const int tests = 1 << 8;
  const unsigned char array[1] = {1};
  int i;
  for (i = 0; i < tests; i++) {
    /* There is only one line to select, must select this one. */
    REQUIRE(0 == select_random_line_blindly(array, 1));
  }
}

TEST_CASE("select_random_line_blindly() with occupied middle line") {
  const int tests = 1 << 10;
  const int seven_sixteenths = 7 * tests / 16;
  const unsigned char array[3] = {0, 1, 0};
  int counters[3] = {0, 0, 0};
  int i;
  for (i = 0; i < tests; i++) {
    counters[select_random_line_blindly(array, 3)] += 1;
  }
  REQUIRE(0 == counters[1]);
  REQUIRE(counters[0] > seven_sixteenths);
  REQUIRE(counters[2] > seven_sixteenths);
}

TEST_CASE("select_random_line_awarely() with one empty line") {
  const int tests = 1 << 8;
  const unsigned char array[1] = {0};
  int i;
  for (i = 0; i < tests; i++) {
    /* There is only one line to select, must select this one. */
    REQUIRE(0 == select_random_line_awarely(array, 1));
  }
}

TEST_CASE("select_random_line_awarely() with two empty lines") {
  /* Should not overflow after a multiplication by 3. */
  const int tests = 1 << 10;
  const int seven_sixteenths = 7 * tests / 16;
  const unsigned char array[2] = {0, 0};
  int counters[2] = {0, 0};
  int i;
  for (i = 0; i < tests; i++) {
    counters[select_random_line_awarely(array, 2)] += 1;
  }
  /* Counters should be roughly the same. */
  REQUIRE(counters[0] > seven_sixteenths);
  REQUIRE(counters[1] > seven_sixteenths);
}

TEST_CASE("select_random_line_awarely() with three empty lines") {
  const int tests = 1 << 10;
  const unsigned char array[3] = {0, 0, 0};
  int counters[3] = {0, 0, 0};
  int i;
  for (i = 0; i < tests; i++) {
    counters[select_random_line_awarely(array, 3)] += 1;
  }
  /* The middle line is the most distant one. */
  REQUIRE(counters[1] == tests);
}

TEST_CASE("select_random_line_awarely() with one occupied line") {
  const int tests = 1 << 8;
  const unsigned char array[1] = {1};
  int i;
  for (i = 0; i < tests; i++) {
    /* There is only one line to select, must select this one. */
    REQUIRE(0 == select_random_line_awarely(array, 1));
  }
}

TEST_CASE("select_random_line_awarely() with occupied middle line") {
  const int tests = 1 << 10;
  const int seven_sixteenths = 7 * tests / 16;
  const unsigned char array[3] = {0, 1, 0};
  int counters[3] = {0, 0, 0};
  int i;
  for (i = 0; i < tests; i++) {
    counters[select_random_line_awarely(array, 3)] += 1;
  }
  REQUIRE(0 == counters[1]);
  REQUIRE(counters[0] > seven_sixteenths);
  REQUIRE(counters[2] > seven_sixteenths);
}
