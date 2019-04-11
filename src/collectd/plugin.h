/*
 * Copyright (C) 2016-2018 "IoT.bzh"
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#define PLUGIN_H
#ifdef PLUGIN_H

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <pthread.h>
#include <dlfcn.h>
#include <string.h>
#include "config.h"

/* %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
                            Define
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% */

#define NOTIF_MAX_MSG_LEN 256

#define CLOCK_REALTIME			0

#define ERRBUF_SIZE 256

#define STRERROR(e) sstrerror((e), (char[ERRBUF_SIZE]){0}, ERRBUF_SIZE)

extern int *__errno_location (void) __THROW __attribute_const__;

#define errno (*__errno_location ())

#define STRERRNO STRERROR(errno)

#define LOG_ERR 3

#define ERROR(...) plugin_log(LOG_ERR, __VA_ARGS__)

#define NS_TO_CDTIME_T(ns)                                                     \
  (cdtime_t) {                                                                 \
    ((((cdtime_t)(ns)) / 1000000000) << 30) |                                  \
        ((((((cdtime_t)(ns)) % 1000000000) << 30) + 500000000) / 1000000000)   \
  }

#define TIMESPEC_TO_CDTIME_T(ts)                                               \
  NS_TO_CDTIME_T(1000000000ULL * (ts)->tv_sec + (ts)->tv_nsec)

#define CDTIME_T_TO_DOUBLE(t)                                                  \
  (double) { ((double)(t)) / 1073741824.0 }

#define TIME_T_TO_CDTIME_T_STATIC(t) (((cdtime_t)(t)) << 30)

#define TIME_T_TO_CDTIME_T(t)                                                  \
  (cdtime_t) { TIME_T_TO_CDTIME_T_STATIC(t) }

#define sfree(ptr)                                                             \
  do {                                                                         \
    free(ptr);                                                                 \
    (ptr) = NULL;                                                              \
  } while (0)

#define __ASSERT_FUNCTION	__extension__ __PRETTY_FUNCTION__

extern void __assert_fail (const char *__assertion, const char *__file,
			   unsigned int __line, const char *__function)
     __THROW __attribute__ ((__noreturn__));

#define assert(expr)							\
  ((void) sizeof ((expr) ? 1 : 0), __extension__ ({			\
      if (expr)								\
        ;						\
      else								\
        __assert_fail (#expr, __FILE__, __LINE__, __ASSERT_FUNCTION);	\
    }))

#define	EINVAL		22

#define	EAGAIN		11

#define	ENOMEM		12

#define DS_TYPE_COUNTER 0

#define DS_TYPE_GAUGE 1

#define DS_TYPE_DERIVE 2

#define DS_TYPE_ABSOLUTE 3

#define STATIC_ARRAY_SIZE(a) (sizeof(a) / sizeof(*(a)))

#define VALUE_LIST_INIT                                                        \
  { .values = NULL, .meta = NULL }

#define CPU_PATH "../build/src/collectd/./cpu.so"

/* %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
                            Variables
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% */

typedef struct user_data_s user_data_t;
struct user_data_s {
  void *data;
  void (*free_func)(void *);
};

typedef uint64_t counter_t;
typedef double gauge_t;
typedef uint64_t derive_t;
typedef uint64_t absolute_t;
typedef uint64_t cdtime_t;

typedef union value_u value_t;
union value_u {
  counter_t counter;
  gauge_t gauge;
  derive_t derive;
  absolute_t absolute;
};

typedef struct value_to_rate_state_s value_to_rate_state_t;
struct value_to_rate_state_s {
  value_t last_value;
  cdtime_t last_time;
};

typedef union meta_value_u meta_value_t;
union meta_value_u {
  char *mv_string;
  int64_t mv_signed_int;
  uint64_t mv_unsigned_int;
  double mv_double;
  bool mv_boolean;
};

typedef struct meta_entry_s meta_entry_t;
struct meta_entry_s {
  char *key;
  meta_value_t value;
  int type;
  meta_entry_t *next;
};

typedef struct meta_data_s meta_data_t;
struct meta_data_s {
  meta_entry_t *head;
  pthread_mutex_t lock;
};

typedef struct value_list_s value_list_t;
struct value_list_s {
  value_t *values;
  size_t values_len;
  cdtime_t time;
  cdtime_t interval;
  char host[DATA_MAX_NAME_LEN];
  char plugin[DATA_MAX_NAME_LEN];
  char plugin_instance[DATA_MAX_NAME_LEN];
  char type[DATA_MAX_NAME_LEN];
  char type_instance[DATA_MAX_NAME_LEN];
  meta_data_t *meta;
};

/* %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
                            Callbacks
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% */

typedef int (*plugin_init_cb)(void);
typedef int (*plugin_config_cb)(char const*, char const*);
typedef int (*plugin_read_cb)(user_data_t *);
typedef void (*module_register_t)(void);

/* %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
                            Plugin structure
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% */

typedef struct plugin_s plugin_t;
struct plugin_s
{
    plugin_init_cb init;
    plugin_config_cb config;
    plugin_read_cb read;
};

typedef struct metrics_s metrics_t;
struct metrics_s
{
    value_list_t *metrics;
    size_t size;
};

/* %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
                            Global variables functions
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% */

typedef int (*plugin_init_t)(void);
typedef int (*plugin_deinit_t)(void);

typedef int (*metrics_init_t)(void);
typedef void (*metrics_deinit_t)(void);

/* %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
                            Plugin Functions
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% */

/* PLUGIN INIT */
int plugin_init(void);

/* PLUGIN DEINIT */
int plugin_deinit(void);

/* METRICS INIT */
int metrics_init(value_list_t list);

/* METRICS ADD */
int metrics_add(value_list_t list);

/* METRICS DEINIT */
void metrics_deinit(void);

/* INITIALIZATION */
int plugin_register_init(const char *name, int (*callback)(void));

/* CONFIGURATION */
int plugin_register_config(const char *name,
                           int (*callback)(const char *key,
                           const char *val),
                           const char **keys, int keys_num);

/* LOG */
void plugin_log(int level, const char *format, ...);

/* DISPATCH VALUES */
int plugin_dispatch_values(value_list_t *vl);

/* READ */
int plugin_register_read(const char *name, int (*callback)(user_data_t *));

/* %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
                            Additional Functions
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% */

/* strsplit */
int strsplit(char *string, char **fields, size_t size);

/* sstrncpy */
char *sstrncpy(char *dest, const char *src, size_t n);

/* strncpy */
extern char *strncpy (char *__restrict __dest,
		      const char *__restrict __src, size_t __n)
     __THROW __nonnull ((1, 2));

/* value_to_rate */
int value_to_rate(gauge_t *ret_rate,
                  value_t value, int ds_type, cdtime_t t,
                  value_to_rate_state_t *state);

/* cdtime */
cdtime_t cdtime(void);

/* sstrerror */
char *sstrerror(int errnum, char *buf, size_t buflen);

/* counter_diff */
counter_t counter_diff(counter_t old_value, counter_t new_value);

#endif
