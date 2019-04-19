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

#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>
#include <pthread.h>
#include <dlfcn.h>
#include <string.h>
#include <math.h>
#include <errno.h>
#include "collectd_glue.h"

pthread_mutex_t strerror_r_lock = PTHREAD_MUTEX_INITIALIZER;
plugin_ctx_t ctx_init = {0};
pthread_key_t plugin_ctx_key;

/* DECLARE STATIC FUNCTION */
char *sstrncpy(char *dest, const char *src, size_t n);
static void plugin_value_list_free(value_list_t *vl);

/* %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
                            Global plugin variables
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% */

plugin_t *Plugin_collectd;
metrics_t *Metrics_collectd;

/* Plugin init */
int plugin_init(const char *plugin_label)
{
    Plugin_collectd = (plugin_t*)malloc(sizeof(plugin_t));
    if(!Plugin_collectd)
        return -1;

    printf("%s : Plugin address %p : \n", __func__, Plugin_collectd);

    Plugin_collectd->plugin_callback = (plugin_callback_t*)malloc(sizeof(plugin_callback_t));
    if(!Plugin_collectd->plugin_callback)
        return -1;

    printf("%s : Plugin callback address %p : \n", __func__, Plugin_collectd->plugin_callback);

    Plugin_collectd->plugin_callback[0].init = (plugin_init_cb)malloc(sizeof(plugin_init_cb));
    if(!Plugin_collectd->plugin_callback[0].init)
        return -1;

    printf("%s : Plugin callback init address %p : \n", __func__, Plugin_collectd->plugin_callback[0].init);

    Plugin_collectd->plugin_callback[0].config = (plugin_config_cb)malloc(sizeof(plugin_config_cb));
    if(!Plugin_collectd->plugin_callback[0].config)
        return -1;

    printf("%s : Plugin callback config address %p : \n", __func__, Plugin_collectd->plugin_callback[0].config);

    Plugin_collectd->plugin_callback[0].complex_config = (plugin_complex_config_cb)malloc(sizeof(plugin_complex_config_cb));
    if(!Plugin_collectd->plugin_callback[0].complex_config)
        return -1;

    printf("%s : Plugin callback complex config address %p : \n", __func__, Plugin_collectd->plugin_callback[0].complex_config);

    Plugin_collectd->plugin_callback[0].read = (plugin_read_cb)malloc(sizeof(plugin_read_cb));
    if(!Plugin_collectd->plugin_callback[0].read)
        return -1;

    printf("%s : Plugin callback read address %p : \n", __func__, Plugin_collectd->plugin_callback[0].read);

    Plugin_collectd->plugin_callback[0].name = (char *)malloc(strlen(plugin_label)*sizeof(char));
    if(!Plugin_collectd->plugin_callback[0].name)
        return -1;

    /* Put the plugin label in the new plugin instance created */
    strcpy(Plugin_collectd->plugin_callback[0].name, plugin_label);
    Plugin_collectd->size = 1;
    return 0;
}

int plugin_add(const char *plugin_label)
{
    /* Trivial case, the plugin is empty, no need to check if a plugin with the name plugin_label exists */
    if(!Plugin_collectd)
        return plugin_init(plugin_label);

    /* Here we have to add at the end of the plugin table the argument list */
    else
    {
        /* If a plugin with label name already exists,we dont want to add this plugin */
        for(int i = 0 ; i != Plugin_collectd->size ; i++)
        {
            if(!strncmp(Plugin_collectd->plugin_callback[i].name, plugin_label, max_size(strlen(plugin_label), strlen(Plugin_collectd->plugin_callback[i].name))))
                return -1;
        }

        /* If we reach that point, we want to add an other plugin in the plugin table */
        if(!(Plugin_collectd->plugin_callback = realloc(Plugin_collectd->plugin_callback, (Plugin_collectd->size + 1)*sizeof(plugin_callback_t))))
           return -1;

        Plugin_collectd->plugin_callback[Plugin_collectd->size].init = (plugin_init_cb)malloc(sizeof(plugin_init_cb));
        if(!Plugin_collectd->plugin_callback[Plugin_collectd->size].init)
            return -1;

        Plugin_collectd->plugin_callback[Plugin_collectd->size].config = (plugin_config_cb)malloc(sizeof(plugin_config_cb));
        if(!Plugin_collectd->plugin_callback[Plugin_collectd->size].config)
            return -1;

        Plugin_collectd->plugin_callback[Plugin_collectd->size].complex_config = (plugin_complex_config_cb)malloc(sizeof(plugin_complex_config_cb));
        if(!Plugin_collectd->plugin_callback[Plugin_collectd->size].complex_config)
            return -1;

        Plugin_collectd->plugin_callback[Plugin_collectd->size].read = (plugin_read_cb)malloc(sizeof(plugin_read_cb));
        if(!Plugin_collectd->plugin_callback[Plugin_collectd->size].read)
            return -1;

        Plugin_collectd->plugin_callback[Plugin_collectd->size].name = (char *)malloc(strlen(plugin_label)*sizeof(char));
        if(!Plugin_collectd->plugin_callback[Plugin_collectd->size].name)
            return -1;

        strcpy(Plugin_collectd->plugin_callback[Plugin_collectd->size].name, plugin_label);
        Plugin_collectd->size ++;
    }

    return 0;
}

/* Plugin deinit */
int plugin_deinit(size_t plugin_index)
{
    /* If the index is not valid */
    if(!&Plugin_collectd->plugin_callback[plugin_index])
        return -1;

    /* Create a plugin tampon to store the new plugin list */
    plugin_t *plugin;
    plugin = (plugin_t*)malloc(sizeof(plugin_t));
    int index = 0;

    if(!plugin)
        return -1;

    /* We ensure there is at least one plugin stored */
    if(!Plugin_collectd->size)
        return -1;

    /* Retrieve the new size of the plugin list */
    plugin->size = Plugin_collectd->size -1;

    /* Plugin tampon table allocation */
    plugin->plugin_callback = (plugin_callback_t*)malloc(plugin->size*sizeof(plugin_callback_t));

    if(!plugin->plugin_callback)
        return -1;

    /* If there was only one plugin remaining */
    if(!plugin->size)
    {
        Plugin_collectd = plugin;
        return 0;
    }

    /* Copy each one of the the previous stored plugin in the new tampon list */
    for(int i = 0 ; i != Plugin_collectd->size ; i++)
    {
        if(i != plugin_index)
        {
            /* init callback copy */
            plugin->plugin_callback[index].init = (plugin_init_cb)malloc(sizeof(plugin_init_cb));
            if(!plugin->plugin_callback[index].init)
                return -1;

            memcpy(&plugin->plugin_callback[index].init, &Plugin_collectd->plugin_callback[i].init, sizeof(plugin_init_cb));

            /* config callback copy */
            plugin->plugin_callback[index].config = (plugin_config_cb)malloc(sizeof(plugin_config_cb));
            if(!plugin->plugin_callback[index].config)
                return -1;

            memcpy(&plugin->plugin_callback[index].config, &Plugin_collectd->plugin_callback[i].config, sizeof(plugin_config_cb));

            /* complex config callback copy */
            plugin->plugin_callback[index].complex_config = (plugin_complex_config_cb)malloc(sizeof(plugin_complex_config_cb));
            if(!plugin->plugin_callback[index].complex_config)
                return -1;

            memcpy(&plugin->plugin_callback[index].complex_config, &Plugin_collectd->plugin_callback[i].complex_config, sizeof(plugin_complex_config_cb));

            /* read callback copy */
            plugin->plugin_callback[index].read = (plugin_read_cb)malloc(sizeof(plugin_read_cb));
            if(!plugin->plugin_callback[index].read)
                return -1;

            memcpy(&plugin->plugin_callback[index].read, &Plugin_collectd->plugin_callback[i].read, sizeof(plugin_read_cb));

            /* plugin name copy */
            plugin->plugin_callback[index].name = (char *)malloc(strlen(Plugin_collectd->plugin_callback[i].name));
            if(!plugin->plugin_callback[index].name)
                return -1;

            strcpy(plugin->plugin_callback[index].name, Plugin_collectd->plugin_callback[i].name);
            index ++;
        }
    }

    /* Let's make the Global plugin variable points towards the brand new plugin list */
    memcpy(Plugin_collectd, plugin, sizeof(plugin_t));
    return 0;
}

/* INDEX PLUGIN LABEL */
int index_plugin_label(plugin_t *plugin_list, const char *plugin_label)
{
    if(!plugin_list)
    {
        return -1;
    }

    for(int i = 0 ; i != plugin_list->size ; i++)
    {
        if(!strncmp(plugin_list->plugin_callback[i].name, plugin_label, max_size(strlen(plugin_label), strlen(plugin_list->plugin_callback->name))))
            return i;
    }

    return -1;
}

/* Metrics INIT */
int metrics_init(value_list_t const *list)
{
    Metrics_collectd = (metrics_t*)malloc(sizeof(metrics_t));
    Metrics_collectd->metrics = (value_list_t*)malloc(sizeof(value_list_t));
    if(!Metrics_collectd || !Metrics_collectd->metrics)
    {
        return -1;
    }
    memcpy(&Metrics_collectd->metrics[0], list, sizeof(value_list_t));
    Metrics_collectd->metrics->values = (value_t*)malloc(list->values_len*sizeof(value_t));
    if(!Metrics_collectd->metrics->values) {
        return -1;
    }
    memcpy(Metrics_collectd->metrics[0].values, list->values, list->values_len*sizeof(value_t));
    Metrics_collectd->size = 1;
    return 0;
}

int metrics_add(value_list_t const *list)
{
     /* Trivial case, the metrics list is empty */
    if(!Metrics_collectd)
    {
        if(metrics_init(list)) {
            return -1;
        }
    }

    /* Here we have to add at the end of the metrics table the argument list */
    else
    {

       if(! (Metrics_collectd->metrics = realloc(Metrics_collectd->metrics, (Metrics_collectd->size + 1)*sizeof(value_list_t)))) {
           return -1;
       }

        /* Copy the new value in the metrics table */
        memcpy(&Metrics_collectd->metrics[Metrics_collectd->size], list, sizeof(value_list_t));
        Metrics_collectd->metrics[Metrics_collectd->size].values = (value_t*)malloc(list->values_len*sizeof(value_t));
        memcpy(Metrics_collectd->metrics[Metrics_collectd->size].values, list->values, list->values_len*sizeof(value_t));

        Metrics_collectd->size ++;
    }

    return 0;
}

/* Metrics DEINIT */
void metrics_deinit(void)
{
    Metrics_collectd->size = 0;
    Metrics_collectd->metrics = NULL;
}

/* %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
                            Plugin Functions
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% */

/* INITIALIZATION */
int plugin_register_init(const char *name, plugin_init_cb callback)
{
    int plugin_index;

    /* If the plugin list is NULL, we want to add a plugin object in the list with the name attribute*/
    if(!Plugin_collectd)
        plugin_add(name);

    /* If a plugin with name do not already exist, we want to add the plugin with the name attribute */
    if(index_plugin_label(Plugin_collectd, name) == -1)
        plugin_add(name);

    plugin_index = index_plugin_label(Plugin_collectd, name);
    printf("%s : Plugin index : %d\n", __func__, plugin_index);

    /* Store in the plugin previously created the initialization callback */
    /* Plugin_collectd->plugin_callback[plugin_index].init = callback; */
    memcpy(Plugin_collectd->plugin_callback[plugin_index].init, callback, sizeof(plugin_init_cb));
    printf("%s : init callback address : %p\n", __func__, Plugin_collectd->plugin_callback[plugin_index].init);
    return 0;
}

/* CONFIGURATION */
int plugin_register_config(const char *name,
                          int (*callback)(const char *key,
                                          const char *val),
                          const char **keys, int keys_num)
{
    int plugin_index;

    /* If the plugin list is NULL, we want to add a plugin object in the list with the name attribute*/
    if(!Plugin_collectd)
        plugin_add(name);

    /* If a plugin with name do not already exist, we want to add the plugin with the name attribute */
    if(index_plugin_label(Plugin_collectd, name) == -1)
        plugin_add(name);

    plugin_index = index_plugin_label(Plugin_collectd, name);
    printf("%s : Plugin index : %d\n", __func__, plugin_index);

    /* Store in the plugin previously created the initialization callback */
    memcpy(Plugin_collectd->plugin_callback[plugin_index].config, callback, sizeof(plugin_config_cb));
    return 0;
}

/* COMPLEX CONFIG */
int plugin_register_complex_config(const char *type,
                                          int (*callback)(oconfig_item_t *))
{
     /* If the plugin table is missconfigured */
    /* if(!Plugin_collectd->plugin_callback[plugin_index].complex_config)
    {
        return -1;
    }

    Plugin_collectd->plugin_callback[plugin_index].complex_config = callback;
    return cf_register_complex(type, callback); */
    return 0;
}

void plugin_log(int level, char const *format, ...)
{
  char buffer[1024];
  va_list ap;

  va_start(ap, format);
  vsnprintf(buffer, sizeof(buffer), format, ap);
  va_end(ap);

  printf("plugin_log (%i, \"%s\");\n", level, buffer);
}

/* CONTEXT CREATE */
plugin_ctx_t *plugin_ctx_create(void) {
    plugin_ctx_t *ctx;

    ctx = malloc(sizeof(*ctx));
    if (ctx == NULL) {
        return NULL;
    }

    *ctx = ctx_init;
    assert(plugin_ctx_key_initialized);
    pthread_setspecific(plugin_ctx_key, ctx);
    return ctx;
}

/* GET CONTEXT */
plugin_ctx_t plugin_get_ctx(void)
{
    plugin_ctx_t *ctx;

    assert(plugin_ctx_key_initialized);
    ctx = pthread_getspecific(plugin_ctx_key);

    if (ctx == NULL) {
        ctx = plugin_ctx_create();
        /* this must no happen -- exit() instead? */
        if (ctx == NULL)
        return ctx_init;
    }

    return *ctx;
}

/* DISPATCH VALUES */
int plugin_dispatch_values(value_list_t const *vl)
{
    if(metrics_add(vl))
    {
        return -1;
    }

    return 0;
}

/* LIST CLONE */
value_list_t *plugin_value_list_clone(value_list_t const *vl_orig)
{
    value_list_t *vl;

    if (vl_orig == NULL)
    {
        return NULL;
    }

    vl = malloc(sizeof(*vl));
    if (vl == NULL)
    {
        return NULL;
    }

    memcpy(vl, vl_orig, sizeof(*vl));

    if (vl->host[0] == 0)
    {
        sstrncpy(vl->host, "Low collector", sizeof(vl->host));
    }

    vl->values = calloc(vl_orig->values_len, sizeof(*vl->values));
    if (vl->values == NULL) {
        plugin_value_list_free(vl);
        return NULL;
    }

    memcpy(vl->values, vl_orig->values, vl_orig->values_len * sizeof(*vl->values));

    if (vl->time == 0)
    {
        vl->time = cdtime();
    }

  return vl;
}

/* LIST FREE */
static void plugin_value_list_free(value_list_t *vl)
{
  if (vl == NULL)
  {
      return;
  }

  sfree(vl->values);
  sfree(vl);
}

/* DISPATCH MULTI VALUE */
__attribute__((sentinel)) int plugin_dispatch_multivalue(value_list_t const *template,
                           bool store_percentage, int store_type, ...)
{
    value_list_t *vl;
    int failed = 0;
    gauge_t sum = 0.0;
    va_list ap;

    assert(template->values_len == 1);

    /* Calculate sum for Gauge to calculate percent if needed */
    if (DS_TYPE_GAUGE == store_type) {
        va_start(ap, store_type);
        while (42) {
        char const *name;
        gauge_t value;

        name = va_arg(ap, char const *);
        if (name == NULL)
            break;

        value = va_arg(ap, gauge_t);
        if (!isnan(value))
            sum += value;
        }
        va_end(ap);
    }

    vl = plugin_value_list_clone(template);
    /* plugin_value_list_clone makes sure vl->time is set to non-zero. */
    if (store_percentage)
        sstrncpy(vl->type, "percent", sizeof(vl->type));

    va_start(ap, store_type);
    while (42) {
        char const *name;

        /* Set the type instance. */
        name = va_arg(ap, char const *);
        if(name == NULL)
        {
            break;
        }
        sstrncpy(vl->type_instance, name, sizeof(vl->type_instance));

        /* Set the value. */
        switch (store_type)
        {
            case DS_TYPE_GAUGE:
            {
                vl->values[0].gauge = va_arg(ap, gauge_t);
                if (store_percentage)
                    vl->values[0].gauge *= sum ? (100.0 / sum) : NAN;
                break;
            }

            case DS_TYPE_ABSOLUTE:
            {
                vl->values[0].absolute = va_arg(ap, absolute_t);
                break;
            }

            case DS_TYPE_COUNTER:
            {
                vl->values[0].counter = va_arg(ap, counter_t);
                break;
            }

            case DS_TYPE_DERIVE:
            {
                vl->values[0].derive = va_arg(ap, derive_t);
                break;
            }

            default:
            {
                failed++;
            }
        }

        if(!plugin_dispatch_values(vl))
        {
            failed++;
        }
    }
    va_end(ap);

    plugin_value_list_free(vl);
    return failed;
}

/* READ */
int plugin_register_read(const char *name, int (*callback)(void))
{

    int plugin_index;

    /* If the plugin list is NULL, we want to add a plugin object in the list with the name attribute*/
    if(!Plugin_collectd)
        plugin_add(name);

    /* If a plugin with name do not already exist, we want to add the plugin with the name attribute */
    if(index_plugin_label(Plugin_collectd, name) == -1)
        plugin_add(name);

    plugin_index = index_plugin_label(Plugin_collectd, name);
    printf("%s : Plugin index : %d\n", __func__, plugin_index);

    /* Store in the plugin previously created the initialization callback */
    Plugin_collectd->plugin_callback[plugin_index].read = (plugin_read_cb)callback;
    return 0;
}

/* %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
                            Additional Functions
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% */

/* sstrncpy */
char *sstrncpy(char *dest, const char *src, size_t n)
{
    strncpy(dest, src, n);
    dest[n - 1] = '\0';
    return dest;
}

/* value_to_rate */
int value_to_rate(gauge_t *ret_rate,
                  value_t value, int ds_type, cdtime_t t,
                  value_to_rate_state_t *state)
{
    gauge_t interval;

    if (t <= state->last_time) {
        memset(state, 0, sizeof(*state));
        return EINVAL;
    }

    interval = CDTIME_T_TO_DOUBLE(t - state->last_time);

    if (state->last_time == 0) {
        state->last_value = value;
        state->last_time = t;
        return EAGAIN;
    }

    switch (ds_type)
    {

        case DS_TYPE_DERIVE:
        {
            derive_t diff = value.derive - state->last_value.derive;
            *ret_rate = ((gauge_t)diff) / ((gauge_t)interval);
            break;
        }

        case DS_TYPE_GAUGE:
        {
            *ret_rate = value.gauge;
            break;
        }

        case DS_TYPE_COUNTER:
        {
            counter_t diff = counter_diff(state->last_value.counter, value.counter);
            *ret_rate = ((gauge_t)diff) / ((gauge_t)interval);
            break;
        }

        case DS_TYPE_ABSOLUTE:
        {
            absolute_t diff = value.absolute;
            *ret_rate = ((gauge_t)diff) / ((gauge_t)interval);
            break;
        }

        default:
        {
            return EINVAL;
        }
    }

    state->last_value = value;
    state->last_time = t;
    return 0;
}

/* cdtime */
cdtime_t cdtime(void)
{
    int status;
    struct timespec ts = {0, 0};

    status = clock_gettime(CLOCK_REALTIME, &ts);
    if (status != 0) {
        return 0;
    }
    return TIMESPEC_TO_CDTIME_T(&ts);
}

/* sstrerror */
char *sstrerror(int errnum, char *buf, size_t buflen)
{
    char *temp;

    pthread_mutex_lock(&strerror_r_lock);

    temp = strerror(errnum);
    sstrncpy(buf, temp, buflen);

    pthread_mutex_unlock(&strerror_r_lock);
    return buf;
}

/* strsplit */
int strsplit(char *string, char **fields, size_t size)
{
    size_t i;
    char *ptr;
    char *saveptr;

    i = 0;
    ptr = string;
    saveptr = NULL;
    while ((fields[i] = strtok_r(ptr, " \t\r\n", &saveptr)) != NULL) {
        ptr = NULL;
        i++;

        if (i >= size)
        break;
    }
    return (int)i;
}

/* counter_diff */
counter_t counter_diff(counter_t old_value, counter_t new_value)
{
    counter_t diff;
    if (old_value > new_value) {
        if (old_value <= 4294967295U)
        diff = (4294967295U - old_value) + new_value + 1;
        else
        diff = (18446744073709551615ULL - old_value) + new_value + 1;
    } else {
        diff = new_value - old_value;
    }
    return diff;
}

/* max_size */
size_t max_size(size_t a, size_t b)
{
    if(a >= b)
    {
        return a;
    }
    return b;
}

/* cf_util_get_boolean */
int cf_util_get_boolean(const oconfig_item_t *ci, bool *ret_bool)
{
    if ((ci == NULL) || (ret_bool == NULL))
        return EINVAL;

    if ((ci->values_num != 1) || ((ci->values[0].type != OCONFIG_TYPE_BOOLEAN) &&
                                (ci->values[0].type != OCONFIG_TYPE_STRING))) {
        return -1;
    }

    switch (ci->values[0].type)
    {
        case OCONFIG_TYPE_BOOLEAN:
        {
            *ret_bool = ci->values[0].value.boolean ? true : false;
            break;
        }

        case OCONFIG_TYPE_STRING:
        {
            if(IS_TRUE(ci->values[0].value.string))
                *ret_bool = true;

            else if(IS_FALSE(ci->values[0].value.string))
                *ret_bool = false;

            else
                return -1;

            break;
        }
    }

  return 0;
}

/* cf_register_complex */
int cf_register_complex(const char *type, int (*callback)(oconfig_item_t *)) {
  cf_complex_callback_t *new;

  new = malloc(sizeof(*new));
  if (new == NULL)
    return -1;

  new->type = strdup(type);
  if (new->type == NULL) {
    sfree(new);
    return -1;
  }

  new->callback = callback;
  new->next = NULL;

  new->ctx = plugin_get_ctx();

  if (complex_callback_head == NULL) {
    complex_callback_head = new;
  } else {
    cf_complex_callback_t *last = complex_callback_head;
    while (last->next != NULL)
      last = last->next;
    last->next = new;
  }

  return 0;
}
