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

typedef int (*plugin_config_cb)(char const*, char const*);
typedef int (*plugin_complex_config_cb)(oconfig_item_t *);
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
typedef int (*index_plugin_label_t)(plugin_t *, const char *);

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
