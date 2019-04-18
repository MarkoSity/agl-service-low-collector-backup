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

#ifndef COLLECTD_GLUE_H
#define COLLECTD_GLUE_H

#include "sources/src/daemon/plugin.h"
#include "sources/src/utils/common/common.h"

/* %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
                            Define
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% */

#if 0
#define NOTIF_MAX_MSG_LEN 256

#define CLOCK_REALTIME			0

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

/* #define assert(expr)							\
  ((void) sizeof ((expr) ? 1 : 0), __extension__ ({			\
      if (expr)								\
        ;						\
      else								\
        __assert_fail (#expr, __FILE__, __LINE__, __ASSERT_FUNCTION);	\
    })) */

#define IS_TRUE(s)                                                             \
  ((strcasecmp("true", (s)) == 0) || (strcasecmp("yes", (s)) == 0) ||          \
   (strcasecmp("on", (s)) == 0))

#define IS_FALSE(s)                                                            \
  ((strcasecmp("false", (s)) == 0) || (strcasecmp("no", (s)) == 0) ||          \
   (strcasecmp("off", (s)) == 0))

#ifndef ERRBUF_SIZE
#define ERRBUF_SIZE 256
#endif

#define STRERROR(e) sstrerror((e), (char[ERRBUF_SIZE]){0}, ERRBUF_SIZE)
#define STRERRNO STRERROR(errno)

#ifndef LOG_ERR
#define LOG_ERR 3
#endif

#define ERROR(...) plugin_log(LOG_ERR, __VA_ARGS__)

#define	EINVAL		22

#define	EAGAIN		11

#define DS_TYPE_COUNTER 0

#define DS_TYPE_GAUGE 1

#define DS_TYPE_DERIVE 2

#define DS_TYPE_ABSOLUTE 3

#define OCONFIG_TYPE_STRING 0

#define OCONFIG_TYPE_NUMBER 1

#define OCONFIG_TYPE_BOOLEAN 2

#define STATIC_ARRAY_SIZE(a) (sizeof(a) / sizeof(*(a)))

#define VALUE_LIST_INIT                                                        \
  { .values = NULL, .meta = NULL }

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

typedef struct oconfig_value_s oconfig_value_t;
struct oconfig_value_s {
  union {
    char *string;
    double number;
    int boolean;
  } value;
  int type;
};

typedef struct oconfig_item_s oconfig_item_t;
struct oconfig_item_s {
  char *key;
  oconfig_value_t *values;
  int values_num;

  oconfig_item_t *parent;
  oconfig_item_t *children;
  int children_num;
};

typedef struct plugin_ctx_s plugin_ctx_t;
struct plugin_ctx_s {
  char *name;
  cdtime_t interval;
  cdtime_t flush_interval;
  cdtime_t flush_timeout;
};

/* MEM PLUGIN SETTINGS VARIABLES */

bool values_absolute;
bool values_percentage;
#endif

typedef struct cf_complex_callback_s {
  char *type;
  int (*callback)(oconfig_item_t *);
  plugin_ctx_t ctx;
  struct cf_complex_callback_s *next;
} cf_complex_callback_t;

cf_complex_callback_t *complex_callback_head;
bool plugin_ctx_key_initialized;

/* %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
                            Callbacks
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% */

/* typedef int (*plugin_init_cb)(void); */
typedef int (*plugin_config_cb)(char const*, char const*);
typedef int (*plugin_complex_config_cb)(oconfig_item_t *);
/* typedef int (*plugin_read_cb)(user_data_t *); */
typedef void (*module_register_t)(void);

/* %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
                            Plugin structure
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% */

typedef struct plugin_callback_s plugin_callback_t;
struct plugin_callback_s
{
    char *name;
    plugin_init_cb init;
    plugin_config_cb config;
    plugin_complex_config_cb complex_config;
    plugin_read_cb read;
};

typedef struct plugin_s plugin_t;
struct plugin_s
{
    plugin_callback_t *plugin_callback;
    size_t size;
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

/* PLUGIN */
typedef int (*plugin_init_t)(char *);
typedef int (*plugin_add_t)(char *);
typedef int (*plugin_deinit_t)(size_t);

/* METRICS */
typedef int (*metrics_init_t)(value_list_t *);
typedef int (*metrics_add_t)(value_list_t *);
typedef void (*metrics_deinit_t)(void);

/* SIZE */
typedef size_t (*max_size_t)(size_t a, size_t b);

/* %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
                            PLUGIN LIST FUNCTIONS
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% */

/* PLUGIN INIT */
int plugin_init(const char *plugin_label);

/* PLUGIN ADD */
int plugin_add(const char *plugin_label);

/* PLUGIN DEINIT */
int plugin_deinit(size_t plugin_index);

/* INDEX PLUGIN LABEL */
int index_plugin_label(plugin_t *plugin_list, const char *plugin_label);

/* METRICS INIT */
int metrics_init(value_list_t const *list);

/* METRICS ADD */
int metrics_add(value_list_t const *list);

/* METRICS DEINIT */
void metrics_deinit(void);

/* CPU SETTINGS RESET */
void cpu_settings_reset(void);

/* MEM SETTINGS RESET */
void mem_settings_reset(void);

#if 0
/* INITIALIZATION */
int plugin_register_init(const char *name, plugin_init_cb callback);

/* CONFIGURATION */
int plugin_register_config(const char *name,
                          int (*callback)(const char *key,
                                          const char *val),
                          const char **keys, int keys_num);

/* COMPLEX CONFIG */
int plugin_register_complex_config(const char *type,
                                   int (*callback)(oconfig_item_t *),
                                   size_t plugin_index);

/* LOG */
void plugin_log(int level, char const *format, ...);

/* CONTEXT CREATE */
plugin_ctx_t *plugin_ctx_create(void);

/* GET CONTEXT */
plugin_ctx_t plugin_get_ctx(void);

/* LOG */
/* void plugin_log(int level, const char *format, ...); */

/* DISPATCH VALUES */
int plugin_dispatch_values(value_list_t *vl);

/* LIST CLONE */
value_list_t *plugin_value_list_clone(value_list_t const *vl_orig);

/* LIST FREE */
void plugin_value_list_free(value_list_t *vl);

/* DISPATCH MULTI VALUE */
__attribute__((sentinel)) int plugin_dispatch_multivalue(value_list_t const *template,
                           bool store_percentage, int store_type, ...);

/* READ */
int plugin_register_read(const char *name, int (*callback)(void));

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

/* cf_util_get_boolean */
int cf_util_get_boolean(const oconfig_item_t *ci, bool *ret_bool);

/* cf_register_complex */
int cf_register_complex(const char *type, int (*callback)(oconfig_item_t *));

#endif
/* max_size */
size_t max_size(size_t a, size_t b);
#endif /* COLLECTD_GLUE_H */
