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

#define BASIS_H
#ifdef BASIS_H

#include <stdint.h>
#include <stdbool.h>
#include <pthread.h>
#include "../../collectd/config.h"

#define NOTIF_MAX_MSG_LEN 256

typedef uint64_t counter_t;
typedef double gauge_t;
typedef uint64_t derive_t;
typedef uint64_t absolute_t;

union value_u {
  counter_t counter;
  gauge_t gauge;
  derive_t derive;
  absolute_t absolute;
};

typedef union value_u value_t;
typedef uint64_t cdtime_t;

union meta_value_u {
  char *mv_string;
  int64_t mv_signed_int;
  uint64_t mv_unsigned_int;
  double mv_double;
  bool mv_boolean;
};

typedef struct meta_entry_s meta_entry_t;
struct meta_data_s {
  meta_entry_t *head;
  pthread_mutex_t lock;
};

typedef union meta_value_u meta_value_t;
struct meta_entry_s {
  char *key;
  meta_value_t value;
  int type;
  meta_entry_t *next;
};

typedef struct meta_data_s meta_data_t;

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
typedef struct value_list_s value_list_t;

struct value_to_rate_state_s {
  value_t last_value;
  cdtime_t last_time;
};
typedef struct value_to_rate_state_s value_to_rate_state_t;

struct user_data_s {
  void *data;
  void (*free_func)(void *);
};
typedef struct user_data_s user_data_t;

enum notification_meta_type_e {
  NM_TYPE_STRING,
  NM_TYPE_SIGNED_INT,
  NM_TYPE_UNSIGNED_INT,
  NM_TYPE_DOUBLE,
  NM_TYPE_BOOLEAN
};

typedef struct notification_meta_s {
  char name[DATA_MAX_NAME_LEN];
  enum notification_meta_type_e type;
  union {
    const char *nm_string;
    int64_t nm_signed_int;
    uint64_t nm_unsigned_int;
    double nm_double;
    bool nm_boolean;
  } nm_value;
  struct notification_meta_s *next;
} notification_meta_t;

typedef struct notification_s {
  int severity;
  cdtime_t time;
  char message[NOTIF_MAX_MSG_LEN];
  char host[DATA_MAX_NAME_LEN];
  char plugin[DATA_MAX_NAME_LEN];
  char plugin_instance[DATA_MAX_NAME_LEN];
  char type[DATA_MAX_NAME_LEN];
  char type_instance[DATA_MAX_NAME_LEN];
  notification_meta_t *meta;
} notification_t;

struct data_source_s {
  char name[DATA_MAX_NAME_LEN];
  int type;
  double min;
  double max;
};
typedef struct data_source_s data_source_t;

struct data_set_s {
  char type[DATA_MAX_NAME_LEN];
  size_t ds_num;
  data_source_t *ds;
};
typedef struct data_set_s data_set_t;


#endif
