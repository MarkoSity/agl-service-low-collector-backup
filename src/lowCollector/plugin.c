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

#include "plugin.h"
#include <stdlib.h>
#include <stdio.h>

/* Define the variable which wil be a table of all the plugin used by the binding
It is defined in a source file in order to create a symbol
which will be the link between the AGL binding and the collectd plugin functions */

plugin_t *Plugin;

int plugin_init(void)
{
    Plugin = malloc(sizeof(plugin_t));
    return 0;
}

int plugin_register_init(const char *name, int (*callback)(void))
{

    /* Store in the plugin the desired callback */
    Plugin->init = callback;
    printf("initialization callback stored in the plugin\n");

    return 0;
}

int plugin_register_config(const char *name,
                           int (*callback)(const char *key,
                           const char *val),
                           const char **keys, int keys_num)
{
    /* Store in the plugin the desired callback */
    Plugin->config = callback;
    printf("Configuration callback stored in the plugin\n");
    return 0;
}

int plugin_register_read(const char *name, int (*callback)(user_data_t *))
{
    /* Store in the plugin the desired callback */
    Plugin->read = callback;
    printf("Read callback stored in the plugin\n");
    return 0;
}

int plugin_dispatch_values(value_list_t const *vl)
{
    printf("plugin_dispatch_values %p %s\n", (void *)vl, vl?vl->plugin:"NULL");
    return 0;
}
