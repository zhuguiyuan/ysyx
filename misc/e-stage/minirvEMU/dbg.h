#ifndef __dbg_h__
#define __dbg_h__

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef NDEBUG
#define debug(M, ...)
#else
#define debug(M, ...) log_debug(M, ##__VA_ARGS__)
#endif

#ifdef NDEBUG
#define invariant_debug(A, M, ...)
#else
#define invariant_debug(A, M, ...)                                             \
  if (!(A)) {                                                                  \
    log_debug(M, ##__VA_ARGS__);                                               \
    abort();                                                                   \
  }
#endif

#define clean_errno() (errno == 0 ? "None" : strerror(errno))

#define log_err(M, ...)                                                        \
  fprintf(stderr, "[ERROR] (%s:%d:%s errno: %s) " M "\n", __FILE__, __LINE__,  \
          __func__, clean_errno(), ##__VA_ARGS__)

#define log_warn(M, ...)                                                       \
  fprintf(stderr, "[WARN] (%s:%d:%s errno: %s) " M "\n", __FILE__, __LINE__,   \
          __func__, clean_errno(), ##__VA_ARGS__)

#define log_info(M, ...)                                                       \
  fprintf(stderr, "[INFO] (%s:%d:%s) " M "\n", __FILE__, __LINE__, __func__,   \
          ##__VA_ARGS__)

#define log_debug(M, ...)                                                      \
  fprintf(stderr, "[DEBUG] (%s:%d:%s) " M "\n", __FILE__, __LINE__, __func__,  \
          ##__VA_ARGS__)

#define check(A, M, ...)                                                       \
  if (!(A)) {                                                                  \
    log_err(M, ##__VA_ARGS__);                                                 \
    errno = 0;                                                                 \
    goto error;                                                                \
  }

/** 做一些不需要打印 check 信息的检查，在 NDEBUG 的时候静默，但还是检查 */
#define check_debug(A, M, ...)                                                 \
  if (!(A)) {                                                                  \
    debug(M, ##__VA_ARGS__);                                                   \
    errno = 0;                                                                 \
    goto error;                                                                \
  }

#define sentinel(M, ...)                                                       \
  {                                                                            \
    log_err(M, ##__VA_ARGS__);                                                 \
    errno = 0;                                                                 \
    goto error;                                                                \
  }

#endif
