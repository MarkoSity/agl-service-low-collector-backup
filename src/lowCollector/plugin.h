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

#include "basis/basis.h"

/* Plugin callback types definition */
typedef int (*plugin_init_cb)(void);
typedef int (*plugin_config_cb)(char const*, char const*);
typedef int (*plugin_read_cb)(user_data_t *);
typedef int (*plugin_write_cb)(const data_set_t *, const value_list_t *, user_data_t *);
typedef int (*plugin_flush_cb)(cdtime_t timeout, const char *identifier, user_data_t *);
typedef int (*plugin_missing_cb)(const value_list_t *, user_data_t *);
typedef void (*plugin_log_cb)(int severity, const char *message, user_data_t *);
typedef int (*plugin_shutdown_cb)(void);
typedef int (*plugin_notification_cb)(const notification_t *, user_data_t *);

/* Plugin structure that we use for the binding in AGL */
struct plugin_s
{
    plugin_init_cb init;
    plugin_config_cb config;
    plugin_read_cb read;
    plugin_write_cb write;
    plugin_flush_cb flush;
    plugin_missing_cb missing;
    plugin_log_cb log;
    plugin_shutdown_cb shutdown;
    plugin_notification_cb notification;
};
/* Define new types regarding the AGL plugin
and the module register function
which will be used by each collectd plugin in order to initialize the AGL plugin */
typedef struct plugin_s plugin_t;
extern plugin_t *Plugin;
typedef void (*module_register_t)(void);

int plugin_init(void);

#endif

