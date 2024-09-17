#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/** Our old friend die from ex17. */
void die(const char *message) {
  if (errno) {
    perror(message);
  } else {
    printf("ERROR: %s\n", message);
  }

  exit(1);
}

// a typedef creates a fake type, in this
// case for a function pointer
typedef int (*compare_cb)(int a, int b);

/**
 * A classic bubble sort function that uses the
 * compare_cb to do the sorting.
 */
int *bubble_sort(int *numbers, int count, compare_cb cmp) {
  int temp = 0;
  int i = 0;
  int j = 0;
  int *target = malloc(count * sizeof(int));

  if (!target)
    die("Memory error.");
  memcpy(target, numbers, count * sizeof(int));

  for (i = 0; i < count; i++) {
    for (j = 0; j < count - 1; j++) {
      if (cmp(target[j], target[j + 1]) > 0) {
        temp = target[j + 1];
        target[j + 1] = target[j];
        target[j] = temp;
      }
    }
  }

  return target;
}

int check_is_ordered(int *numbers, int count) {
  for (int i = 1; i < count; ++i) {
    if (numbers[i] < numbers[i - 1]) {
      return 0;
    }
  }
  return 1;
}

static void quick_sort_inplace(int *numbers, int count, compare_cb cmp) {
  if (count <= 1)
    return;
  // [left, right)
  int *numbers_tail = numbers + count - 1;
  int *left = numbers;
  int *right = numbers_tail;
  int tmp;
  while (left < right) {
    while (left < right && cmp(*right, *numbers) >= 0) {
      --right;
    }
    while (left < right && cmp(*left, *numbers) <= 0) {
      ++left;
    }
    tmp = *left, *left = *right, *right = tmp;
  }
  tmp = *left, *left = *numbers, *numbers = tmp;
  quick_sort_inplace(numbers, left - numbers, cmp);
  quick_sort_inplace(left + 1, numbers_tail - left, cmp);
}

int *quick_sort(int *numbers, int count, compare_cb cmp) {
  int *target = malloc(count * sizeof(int));

  if (!target)
    die("Memory error.");
  memcpy(target, numbers, count * sizeof(int));
  quick_sort_inplace(target, count, cmp);

  return target;
}

int sorted_order(int a, int b) { return a - b; }

int reverse_order(int a, int b) { return b - a; }

int strange_order(int a, int b) {
  if (a == 0 || b == 0) {
    return 0;
  } else {
    return a % b;
  }
}

typedef int *(*sort_cb)(int *numbers, int count, compare_cb cmp);

/**
 * Used to test that we are sorting things correctly
 * by doing the sort and printing it out.
 */
void test_sorting(sort_cb sort, const char *sort_name, int *numbers, int count,
                  compare_cb cmp) {
  int i = 0;
  int *sorted = sort(numbers, count, cmp);
  if (!sorted)
    die("Failed to sort as requested.");

  printf("%s: ", sort_name);
  for (i = 0; i < count; i++) {
    printf("%d ", sorted[i]);
  }
  printf("\n");
  free(sorted);

  unsigned char *data = (unsigned char *)cmp;

  for (i = 0; i < 25; i++) {
    printf("%02x:", data[i]);
  }
  printf("\n");
}

int main(int argc, char *argv[]) {
  if (argc < 2)
    die("USAGE: ex18 4 3 1 5 6");
  int count = argc - 1;
  int i = 0;
  char **inputs = argv + 1;
  int *numbers = malloc(count * sizeof(int));
  if (!numbers)
    die("Memory  error.");

  for (i = 0; i < count; i++) {
    numbers[i] = atoi(inputs[i]);
  }
  test_sorting(bubble_sort, "bubble_sort", numbers, count, sorted_order);
  test_sorting(bubble_sort, "bubble_sort", numbers, count, reverse_order);
  test_sorting(bubble_sort, "bubble_sort", numbers, count, strange_order);

  test_sorting(quick_sort, "quick_sort", numbers, count, sorted_order);
  test_sorting(quick_sort, "quick_sort", numbers, count, reverse_order);
  test_sorting(quick_sort, "quick_sort", numbers, count, strange_order);

  free(numbers);

  return 0;
}
