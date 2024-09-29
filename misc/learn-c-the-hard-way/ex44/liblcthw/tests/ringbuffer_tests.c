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
  mu_assert(1024 == RingBuffer_available_space(rb), "Wrong available #space");
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
  mu_assert(amount == (int)strlen(data), "Wrong amount to write");
  mu_assert(amount == RingBuffer_available_data(rb), "Wrong available #data");
  mu_assert(1024 - amount == RingBuffer_available_space(rb),
            "Wrong available #space");
  amount = RingBuffer_read(rb, recv, 7);
  mu_assert(amount == 7, "Wrong amount to read");
  for (int i = 0; i < amount; ++i) {
    mu_assert(recv[i] == data[i], "Wrong read data");
  }
  mu_assert(0 == recv[amount], "Wrong data boundary");
  RingBuffer_destroy(rb);
  return NULL;
}

char *test_write_to_full() {
  rb = RingBuffer_create(1024);
  char *data = calloc(1024, 1);
  RingBuffer_write(rb, data, 1024);
  debug("FUCK YOU %d", RingBuffer_available_data(rb));
  mu_assert(1024 == RingBuffer_available_data(rb), "Wrong available #data");
  mu_assert(0 == RingBuffer_available_space(rb), "Wrong available #space");
  mu_assert(0 == RingBuffer_empty(rb), "RingBuffer should not be empty");
  mu_assert(1 == RingBuffer_full(rb), "RingBuffer should be full");
  return NULL;
}

char *all_tests() {
  mu_suite_start();

  mu_run_test(test_create);
  mu_run_test(test_destroy);
  mu_run_test(test_empty);
  mu_run_test(test_write_then_read);
  mu_run_test(test_gets_all);
  mu_run_test(test_write_to_full);

  return NULL;
}

RUN_TESTS(all_tests);
