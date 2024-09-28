#include "lcthw/bstrlib.h"
#include "minunit.h"
#include <lcthw/ringbuffer.h>
#include <stdlib.h>

static RingBuffer *rb = NULL;

char *test_create() {
  rb = RingBuffer_create(1024);
  mu_assert(rb != NULL, "Failed to create ringbuffer.");
  return NULL;
}

char *test_destroy() {
  RingBuffer_destroy(rb);
  return NULL;
}

char *test_empty() {
  rb = RingBuffer_create(1024);
  mu_assert(1 == RingBuffer_empty(rb), "RingBuffer should be empty");
  RingBuffer_destroy(rb);
  return NULL;
}

char *test_gets_all() {
  rb = RingBuffer_create(1024);
  char *data = calloc(512, 1);
  RingBuffer_write(rb, data, 512);
  mu_assert(0 == RingBuffer_empty(rb), "RingBuffer should be empty");
  mu_assert(512 == RingBuffer_available_space(rb), "Error evailable space");
  bdestroy(RingBuffer_get_all(rb));
  mu_assert(1 == RingBuffer_empty(rb), "RingBuffer should be empty");
  mu_assert(1024 == RingBuffer_available_space(rb), "Error evailable space");
  RingBuffer_destroy(rb);
  return NULL;
}

char *test_write_then_read() {
  char *data = "Here is some data";
  char *recv = calloc(1024, 1);
  rb = RingBuffer_create(1024);
  int amount;
  amount = RingBuffer_write(rb, data, strlen(data));
  debug("Available #data is %d", RingBuffer_available_data(rb));
  debug("Available #space is %d", RingBuffer_available_space(rb));
  mu_assert(amount == (int)strlen(data), "Wrong amount to write");
  amount = RingBuffer_read(rb, recv, 7);
  mu_assert(amount == 7, "Wrong amount to read");
  RingBuffer_destroy(rb);
  return NULL;
}

char *all_tests() {
  mu_suite_start();

  mu_run_test(test_create);
  mu_run_test(test_destroy);
  mu_run_test(test_empty);
  mu_run_test(test_write_then_read);
  mu_run_test(test_gets_all);

  return NULL;
}

RUN_TESTS(all_tests);
