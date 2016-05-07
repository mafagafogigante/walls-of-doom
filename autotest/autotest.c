#include "unity.h"

#include "data.h"
#include "random.h"
#include "sort.h"

int compare_unsigned_char(const void *pointer_to_uchar_a, const void *pointer_to_uchar_b) {
    unsigned char a = *(unsigned char *)(pointer_to_uchar_a);
    unsigned char b = *(unsigned char *)(pointer_to_uchar_b);
    return a < b ? -1 : a == b ? 0 : 1;
}

char *to_string_unsigned_char(const void *pointer_to_uchar) {
    char *buffer = malloc(16);
    sprintf(buffer, "%d", *(unsigned char *)(pointer_to_uchar));
    return buffer;
}

void test_read_integers(void) {
    char filename[] = "test_read_integers_file.txt";
    FILE *file = fopen(filename, "w+");

    const int expected_count = 1;
    const int expected_value = 65535;

    fprintf(file, "%d", expected_value);
    fflush(file);

    int actual_values[expected_count];
    const int actual_count = read_integers(filename, actual_values);

    TEST_ASSERT_EQUAL(expected_count, actual_count);
    TEST_ASSERT_EQUAL(expected_value, actual_values[0]);

    remove(filename);
}

void test_compare_unsigned_char(void) {
    const unsigned char a = 0;
    const unsigned char b = 1;
    const unsigned char c = 2;
    TEST_ASSERT_EQUAL_INT8(-1, compare_unsigned_char((void *)(&a), (void *)(&b)));
    TEST_ASSERT_EQUAL_INT8(-1, compare_unsigned_char((void *)(&a), (void *)(&c)));
    TEST_ASSERT_EQUAL_INT8(-1, compare_unsigned_char((void *)(&b), (void *)(&c)));

    TEST_ASSERT_EQUAL_INT8(0, compare_unsigned_char((void *)(&a), (void *)(&a)));
    TEST_ASSERT_EQUAL_INT8(0, compare_unsigned_char((void *)(&b), (void *)(&b)));
    TEST_ASSERT_EQUAL_INT8(0, compare_unsigned_char((void *)(&c), (void *)(&c)));

    TEST_ASSERT_EQUAL_INT8(1, compare_unsigned_char((void *)(&b), (void *)(&a)));
    TEST_ASSERT_EQUAL_INT8(1, compare_unsigned_char((void *)(&c), (void *)(&a)));
    TEST_ASSERT_EQUAL_INT8(1, compare_unsigned_char((void *)(&c), (void *)(&b)));
}

void test_insertion_sort_with_single_bytes(void) {
    unsigned char source[] = {4, 2, 3, 1};
    const unsigned char sorted[] = {1, 2, 3, 4};
    insertion_sort(source, sizeof(source) / sizeof(unsigned char), sizeof(unsigned char), compare_unsigned_char);
    TEST_ASSERT_EQUAL_INT8_ARRAY_MESSAGE(sorted, source, sizeof(source) / sizeof(unsigned char), "insertion sort failed to sort the input");
}

void test_find_next_power_of_two_works_for_all_integers(void) {
    TEST_ASSERT_EQUAL_INT(find_next_power_of_two(0), 1);
    TEST_ASSERT_EQUAL_INT(find_next_power_of_two(1), 2);
    TEST_ASSERT_EQUAL_INT(find_next_power_of_two(2), 4);
    TEST_ASSERT_EQUAL_INT(find_next_power_of_two(3), 4);
    TEST_ASSERT_EQUAL_INT(find_next_power_of_two(4), 8);
}

void test_random_integer_respects_the_provided_range(void) {
    const int min_min = -3;
    const int max_min = 0;
    const int min_max = 1;
    const int max_max = 4;
    int min;
    int max;
    for (min = min_min; min < max_min; min++) {
        for (max = min_max; max < max_max; max++) {
            int integer = random_integer(min, max);
            TEST_ASSERT_TRUE(min <= integer && integer <= max);
        }
    }
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_read_integers);
    RUN_TEST(test_compare_unsigned_char);
    RUN_TEST(test_insertion_sort_with_single_bytes);
    RUN_TEST(test_find_next_power_of_two_works_for_all_integers);
    RUN_TEST(test_random_integer_respects_the_provided_range);
    return UNITY_END();
}
