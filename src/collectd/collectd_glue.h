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
                            Callbacks
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% */

typedef int (*plugin_config_cb)(char const*, char const*);
typedef int (*plugin_complex_config_cb)(oconfig_item_t *);
typedef void (*module_register_t)(void);

/* %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
                            Plugin structure
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% */

typedef struct plugin_s plugin_t;
struct plugin_s
{
    char *name;
    plugin_init_cb init;
    plugin_config_cb config;
    plugin_complex_config_cb complex_config;
    plugin_read_cb read;
};

typedef struct plugin_list_s plugin_list_t;
struct plugin_list_s
{
    plugin_t *plugin;
    size_t size;
};

typedef struct metrics_list_s metrics_list_t;
struct metrics_list_s
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
typedef int (*index_plugin_label_t)(plugin_list_t *, const char *);

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
int index_plugin_label(plugin_list_t *plugin_list, const char *plugin_label);

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

/* max_size */
size_t max_size(size_t a, size_t b);
#endif /* COLLECTD_GLUE_H */
