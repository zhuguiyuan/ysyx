/**
 * Comment by Guiyuan Zhu:
 *
 *   This is not really a "ring" buffer. Feilds `start` and `end` only are set
 *   to 0 when the buffer is empty. So don't be suprised by the implementation
 *   of the macros (RingBuffer_available_data and RingBuffer_available_space).
 */
#ifndef _lcthw_RingBuffer_h
#define _lcthw_RingBuffer_h

#include <lcthw/bstrlib.h>

typedef struct {
  char *buffer;
  int length;
  int start;
  int end;
} RingBuffer;

RingBuffer *RingBuffer_create(int length);

void RingBuffer_destroy(RingBuffer *buffer);

int RingBuffer_read(RingBuffer *buffer, char *target, int amount);

int RingBuffer_write(RingBuffer *buffer, char *data, int length);

bstring RingBuffer_gets(RingBuffer *buffer, int amount);

/** Check how many space we could read from the ringbuffer. */
#define RingBuffer_available_data(B) ((B)->end - (B)->start)

/** Check how many space we could write before we clean the ringbuffer. */
#define RingBuffer_available_space(B) ((B)->length - (B)->end - 1)

/** Check whether buffer is full */
#define RingBuffer_full(B) (RingBuffer_available_space(B) == 0)

/** Check whether buffer is empty */
#define RingBuffer_empty(B) (RingBuffer_available_data((B)) == 0)

/** Puts char * data D into buffer B */
#define RingBuffer_puts(B, D) RingBuffer_write((B), bdata((D)), blength((D)))

/** Get all data from buffer B. The return value is bstring type. */
#define RingBuffer_get_all(B)                                                  \
  RingBuffer_gets((B), RingBuffer_available_data((B)))

/** Get the char* pointer to the buffer start address. */
#define RingBuffer_starts_at(B) ((B)->buffer + (B)->start)

/** Get the char* pointer to the buffer end address. */
#define RingBuffer_ends_at(B) ((B)->buffer + (B)->end)

/** Commit buffer B with a read action of amount A without any bound check. */
#define RingBuffer_commit_read(B, A)                                           \
  ((B)->start = ((B)->start + (A)) % (B)->length)

/** Commit buffer B with a write action of amount A without any bound check. */
#define RingBuffer_commit_write(B, A)                                          \
  ((B)->end = ((B)->end + (A)) % (B)->length)

#endif
