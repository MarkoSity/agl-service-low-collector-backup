/*
 * Copyright (C) 2016-2019 "IoT.bzh"
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
#include <time.h>
#include "collectd_glue.h"

char hostname[13] = "marco-laptop";

/* %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
                            Plugin & Metrics
                                variables
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% */

plugin_list_t *Plugin_list;
metrics_list_t *Metrics_list;

/* %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
                                Useful
                                function
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% */

size_t max_size(size_t a, size_t b)
{
  if(a >= b)
    return a;

  return b;
}
/* %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
                            Plugin list
                              function
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% */

/* Plugin init */
int plugin_init(const char *plugin_label)
{
  /* Allocate a new plugin list instance */
  Plugin_list = (plugin_list_t *)malloc(sizeof(plugin_list_t));
  if(!Plugin_list)
    return -1;

  /* Allocate the first plugin */
  Plugin_list->plugin = (plugin_t*)malloc(sizeof(plugin_t));
  if(!Plugin_list->plugin)
  {
    /* Free the memory previously allocated */
    sfree(Plugin_list);
    return -1;
  }


  /* Allocate the name field of the first plugin */
  Plugin_list->plugin[0].name = (char *)malloc(strlen(plugin_label)*sizeof(char));
  if(!Plugin_list->plugin[0].name)
  {
    /* Free the memory previously allocated */
    sfree(Plugin_list->plugin);
    sfree(Plugin_list);
    return -1;
  }

  /* Copy the plugin_label into the name field */
  strcpy(Plugin_list->plugin[0].name, plugin_label);

  /* Notify the structue we have add a new plugin object */
  Plugin_list->size = 1;
  return 0;
}

int plugin_add(const char *plugin_label)
{
  /* Variable definition */
  int index_plugin;

  /* Trivial case, the plugin list is NULL, no need to check if a plugin with the name plugin_label exists */
  if(!Plugin_list)
    return plugin_init(plugin_label);

  /* Here we have to add at the end of the plugin list a new plugin */
  else
  {
    /* If a plugin with label name already exists,we dont want to add this plugin */
    index_plugin = index_plugin_label(Plugin_list, plugin_label);
    if(index_plugin != -1)
      return -1;

    /* Proceed to a reallocation of the previous plugin list, and add a new spot */
    if(!(Plugin_list->plugin = realloc(Plugin_list->plugin, (Plugin_list->size + 1)*sizeof(plugin_t))))
    {
      /* Free the new plugin slot we try to allocate */
      plugin_deinit(Plugin_list->size);
      return -1;
    }

    /* Allocate a new name field to the new plugin created */
    Plugin_list->plugin[Plugin_list->size].name = (char *)malloc(strlen(plugin_label)*sizeof(char));
    if(!Plugin_list->plugin[Plugin_list->size].name)
    {
      /* Free the new plugin slot we try to allocate */
      plugin_deinit(Plugin_list->size);
      return -1;
    }

    /* Copy the plugin_label into the name field */
    strcpy(Plugin_list->plugin[Plugin_list->size].name, plugin_label);
    Plugin_list->size ++;
  }

    return 0;
}

/* Plugin deinit */
int plugin_deinit(size_t plugin_index)
{
  /* If the index is not valid */
  if(!&Plugin_list->plugin[plugin_index])
    return -1;

  /* Create a plugin tampon to store the new plugin list */
  plugin_list_t *plugin;
  plugin = (plugin_list_t *)malloc(sizeof(plugin_list_t));
  if(!plugin)
    return -1;

  /* We ensure there is at least one plugin stored */
  if(!Plugin_list->size)
    return -1;

  /* Retrieve the new size of the plugin list */
  plugin->size = Plugin_list->size -1;

  /* Plugin tampon table allocation */
  plugin->plugin = (plugin_t *)malloc(plugin->size*sizeof(plugin_t));
  if(!plugin->plugin)
    return -1;

  /* Current index of the plugin tampon */
  int index = 0;

  /* Copy each one of the the previous stored plugin in the new tampon list */
  for(int i = 0 ; i != Plugin_list->size ; i++)
  {
    /* If the current plugin is not the one we want to delete */
    if(i != plugin_index)
    {
      /* init callback copy */
      plugin->plugin[index].init = (plugin_init_cb)malloc(sizeof(plugin_init_cb));
      if(!plugin->plugin[index].init)
        return -1;
      memcpy(&plugin->plugin[index].init, &Plugin_list->plugin[i].init, sizeof(plugin_init_cb));

      /* config callback copy */
      plugin->plugin[index].config = (plugin_config_cb)malloc(sizeof(plugin_config_cb));
      if(!plugin->plugin[index].config)
        return -1;
      memcpy(&plugin->plugin[index].config, &Plugin_list->plugin[i].config, sizeof(plugin_config_cb));

      /* complex config callback copy */
      plugin->plugin[index].complex_config = (plugin_complex_config_cb)malloc(sizeof(plugin_complex_config_cb));
      if(!plugin->plugin[index].complex_config)
        return -1;
      memcpy(&plugin->plugin[index].complex_config, &Plugin_list->plugin[i].complex_config, sizeof(plugin_complex_config_cb));

      /* read callback copy */
      plugin->plugin[index].read = (plugin_read_cb)malloc(sizeof(plugin_read_cb));
      if(!plugin->plugin[index].read)
        return -1;
      memcpy(&plugin->plugin[index].read, &Plugin_list->plugin[i].read, sizeof(plugin_read_cb));

      /* plugin name copy */
      plugin->plugin[index].name = (char *)malloc(strlen(Plugin_list->plugin[i].name));
      if(!plugin->plugin[index].name)
        return -1;
      strcpy(plugin->plugin[index].name, Plugin_list->plugin[i].name);

      /* Increment the plugin tampon index */
      index ++;
    }
  }

  /* Let's make the Global plugin variable points towards the brand new plugin list */
  memcpy(Plugin_list, plugin, sizeof(plugin_t));
  return 0;
}



/* INDEX PLUGIN LABEL */
int index_plugin_label(plugin_list_t *plugin_list, const char *plugin_label)
{
  /* If the plugin list is NULL */
  if(!plugin_list)
    return -1;

  /* For each one of the plugin stored in the list */
  for(int i = 0 ; i != plugin_list->size ; i++)
  {
    /* If the name matched, return the current index */
    if(!strncmp(plugin_list->plugin[i].name, plugin_label, max_size(strlen(plugin_label), strlen(plugin_list->plugin->name))))
      return i;
  }

  return -1;
}

/* %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
                            Metrics list
                              function
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% */

/* Metrics INIT */
int metrics_init(value_list_t const *list)
{
  /* Allocate a new Metrics list */
  Metrics_list = (metrics_list_t*)malloc(sizeof(metrics_list_t));
  if(!Metrics_list)
    return -1;

  /* Allocate a new metrics object */
  Metrics_list->metrics = (value_list_t*)malloc(sizeof(value_list_t));
  if(!Metrics_list->metrics)
  {
    /* Free the memory allocated and return an error */
    sfree(Metrics_list);
    return -1;
  }

  /* Copy the content of the argument list into the spot allocated */
  memcpy(&Metrics_list->metrics[0], list, sizeof(value_list_t));

  /* Allocate a new metrics value */
  Metrics_list->metrics->values = (value_t*)malloc(list->values_len*sizeof(value_t));
  if(!Metrics_list->metrics->values)
  {
    /* Free the memory allocated and return an error */
    sfree(Metrics_list->metrics);
    sfree(Metrics_list);
    return -1;
  }

  /* Copy the argument value list into the spot previously allocated */
  memcpy(Metrics_list->metrics[0].values, list->values, list->values_len*sizeof(value_t));

  /* Notify to the structure we have add a new metrics */
  Metrics_list->size = 1;
  return 0;
}

int metrics_add(value_list_t const *list)
{
  /* Temporary metrics and value declaration, used to sort the metrics list */
  value_list_t *metrics_tmp;

  /* Trivial case, the metrics list is NULL */
  if(!Metrics_list)
  {
    if(metrics_init(list))
      return -1;
  }

  /* Here we have to add at the end of the metrics table the argument list */
  else
  {
    /* Realloc the metrics list and add a new spot at the end */
    Metrics_list->metrics = realloc(Metrics_list->metrics, (Metrics_list->size + 1)*sizeof(value_list_t));
    if(!&Metrics_list->metrics[Metrics_list->size])
    {
      /* If something went wrong, we free the whole metrics list and notify the error */
      metrics_deinit();
      return -1;
    }

    /* Copy the new metrics in the previous new one allocated metrics */
    memcpy(&Metrics_list->metrics[Metrics_list->size], list, sizeof(value_list_t));

    /* Allocate a new field to store the metrics value */
    Metrics_list->metrics[Metrics_list->size].values = (value_t*)malloc(list->values_len*sizeof(value_t));
    if(!Metrics_list->metrics[Metrics_list->size].values)
    {
      /* If something went wrong, we free the whole metrics list and notify the error */
      metrics_deinit();
      return -1;
    }

    /* Copy the new metrics value in the previous new one allocated metrics value */
    memcpy(Metrics_list->metrics[Metrics_list->size].values, list->values, list->values_len*sizeof(value_t));

    /* Notify the structure we have add a new metrics */
    Metrics_list->size ++;

    /* Now that the metrics has been added to the table, we have to sort the metrics list
    according to their plugin instance, the json writing function need the metrics list to 
    be sorted according to their plugin instance  */
    for (int i = 0; i != Metrics_list->size; i++)
    {
      for (int j = 0; j != Metrics_list->size; j++)
      {
        if (strncmp(Metrics_list->metrics[i].plugin_instance,
                    Metrics_list->metrics[j].plugin_instance,
                    max_size(strlen(Metrics_list->metrics[i].plugin_instance), 
                              strlen(Metrics_list->metrics[j].plugin_instance))) < 0)
        {
          /* Allocate the temporary metrics values */
          metrics_tmp = (value_list_t *)malloc(sizeof(value_list_t));
          if(!metrics_tmp)
          {
            metrics_deinit();
            return -1;
          }

          /* Store the current metrics values */
          memcpy(metrics_tmp, &Metrics_list->metrics[i], sizeof(value_list_t));

          /* Replace the metrics content at index i */
          memcpy(&Metrics_list->metrics[i], &Metrics_list->metrics[j], sizeof(value_list_t));

          /* Replace the metrics content at index j */
          memcpy(&Metrics_list->metrics[j], metrics_tmp, sizeof(value_list_t));

          /* Free the temporary metrics value */
          sfree(metrics_tmp);
        }
      }
    }
  }

  return 0;
}

/* Metrics DEINIT */
void metrics_deinit(void)
{
  /* For each one of the metric stored in the list */
  for(int i = 0 ; i != Metrics_list->size ; i++)
  {
    /* Free the metrics allocated */
    sfree(Metrics_list->metrics[i].values);
  }

  /* Free the metrics list */
  sfree(Metrics_list->metrics);
  sfree(Metrics_list);
}

/* %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
                            Plugin Functions
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% */

/* INITIALIZATION */
int plugin_register_init(const char *name, plugin_init_cb callback)
{
  /* Variable definition */
  int plugin_index;

  /* If the plugin list is NULL, we want to add a plugin object in the list with the argument name*/
  if(!Plugin_list)
    plugin_add(name);

  /* If a plugin with name do not already exist, we want to add the plugin with the argument name */
  if(index_plugin_label(Plugin_list, name) == -1)
    plugin_add(name);

  /* Retrieve the plugin index regarding the argument name */
  plugin_index = index_plugin_label(Plugin_list, name);

  /* Store the address calback in our plugin list structure */
  memcpy(&Plugin_list->plugin[plugin_index].init, &callback, sizeof(plugin_init_cb));

  return 0;
}

/* CONFIGURATION */
int plugin_register_config(const char *name,
                          int (*callback)(const char *key,
                                          const char *val),
                          const char **keys, int keys_num)
{
  /* Variable definition */
  int plugin_index;

  /* If the plugin list is NULL, we want to add a plugin object in the list with the argument name */
  if(!Plugin_list)
    plugin_add(name);

  /* If a plugin with name do not already exist, we want to add the plugin with the argument name */
  if(index_plugin_label(Plugin_list, name) == -1)
    plugin_add(name);

  /* Retrieve the plugin index regarding the argument name */
  plugin_index = index_plugin_label(Plugin_list, name);

  /* Store the address calback in our plugin list structure */
  memcpy(&Plugin_list->plugin[plugin_index].config, &callback, sizeof(plugin_config_cb));
  return 0;
}

/* COMPLEX CONFIG */
int plugin_register_complex_config(const char *type,
                                          int (*callback)(oconfig_item_t *))
{
  /* Variable definition */
  int plugin_index;

  /* If the plugin list is NULL, we want to add a plugin object in the list with the argument name */
  if(!Plugin_list)
    plugin_add(type);

  /* If a plugin with name do not already exist, we want to add the plugin with the argument name */
  if(index_plugin_label(Plugin_list, type) == -1)
    plugin_add(type);

  /* Retrieve the plugin index regarding the argument name */
  plugin_index = index_plugin_label(Plugin_list, type);

  /* Store the address calback in our plugin list structure */
  memcpy(&Plugin_list->plugin[plugin_index].complex_config, &callback, sizeof(plugin_complex_config_cb));
  return 0;
}

/* LOG */
void plugin_log(int level, char const *format, ...)
{
  char buffer[1024];
  va_list ap;

  va_start(ap, format);
  vsnprintf(buffer, sizeof(buffer), format, ap);
  va_end(ap);

  printf("plugin_log (%i, \"%s\");\n", level, buffer);
}

/* Mock context */
plugin_ctx_t mock_context = {.interval = TIME_T_TO_CDTIME_T_STATIC(10),};

/* Plugin GET CONTEXT */
plugin_ctx_t plugin_get_ctx(void)
{
  return mock_context;
}

/* LIST FREE */
void plugin_value_list_free(value_list_t *vl)
{
  if (vl == NULL)
    return;

  sfree(vl->values);
  sfree(vl);
}

/* DISPATCH VALUES */
int plugin_dispatch_values(value_list_t const *vl)
{
  /* Variables definition */
  value_list_t *tmp_vl;

  /* Value list allocation and copy */
  tmp_vl = (value_list_t *)malloc(sizeof(value_list_t));
  memcpy(tmp_vl, vl, sizeof(value_list_t));

  /* Values allocation and copy */
  tmp_vl->values = (value_t *)malloc(sizeof(value_t));
  memcpy(tmp_vl->values, vl->values, sizeof(value_t));

  /* If the hostname is not set yet */
  if(!strncmp(tmp_vl->host, "", strlen(tmp_vl->host)))
    strcpy(tmp_vl->host, hostname);

  /* We want to add in our metrics list the argument vl */
  if(metrics_add(tmp_vl))
  {
    /* Erase the temporary value list previously allocated */
    plugin_value_list_free(tmp_vl);
    return -1;
  }

  return 0;
}

/* LIST CLONE */
value_list_t *plugin_value_list_clone(value_list_t const *vl_orig)
{
  value_list_t *vl;

  if (vl_orig == NULL)
    return NULL;

  vl = malloc(sizeof(*vl));
  if (vl == NULL)
    return NULL;

  memcpy(vl, vl_orig, sizeof(*vl));

  if (vl->host[0] == 0)
    sstrncpy(vl->host, hostname, sizeof(vl->host));

  vl->values = calloc(vl_orig->values_len, sizeof(*vl->values));
  if (vl->values == NULL)
  {
      plugin_value_list_free(vl);
      return NULL;
  }

  memcpy(vl->values, vl_orig->values, vl_orig->values_len * sizeof(*vl->values));

  if (vl->time == 0)
    vl->time = cdtime();

  return vl;
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
    while (42)
    {
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
  while (42)
  {
    char const *name;

    /* Set the type instance. */
    name = va_arg(ap, char const *);
    if(name == NULL)
      break;

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
        failed++;
    }

    if(!plugin_dispatch_values(vl))
      failed++;
  }

  va_end(ap);
  plugin_value_list_free(vl);
  return failed;
}

/* READ */
int plugin_register_read(const char *name, int (*callback)(void))
{
  /* Variable definition */
  int plugin_index;

  /* If the plugin list is NULL, we want to add a plugin object in the list with the argument name */
  if(!Plugin_list)
    plugin_add(name);

  /* If a plugin with name do not already exist, we want to add the plugin with the argument name */
  if(index_plugin_label(Plugin_list, name) == -1)
    plugin_add(name);

  /* Retrieve the plugin index regarding the argument name */
  plugin_index = index_plugin_label(Plugin_list, name);

  /* Store the address calback in our plugin list structure */
  memcpy(&Plugin_list->plugin[plugin_index].read, &callback, sizeof(plugin_read_cb));

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

/* read file contents */
ssize_t read_file_contents(const char *filename, char *buf, size_t bufsize) {
  FILE *fh;
  ssize_t ret;

  fh = fopen(filename, "r");
  if (fh == NULL)
    return -1;

  ret = (ssize_t)fread(buf, 1, bufsize, fh);
  if ((ret == 0) && (ferror(fh) != 0)) {
    P_ERROR("read_file_contents: Reading file \"%s\" failed.", filename);
    ret = -1;
  }

  fclose(fh);
  return ret;
}

/* daemon log */
void daemon_log(int level, const char *format, ...) {
  char msg[1024] = ""; // Size inherits from plugin_log()

  char const *name = plugin_get_ctx().name;
  if (name == NULL)
    name = "UNKNOWN";

  va_list ap;
  va_start(ap, format);
  vsnprintf(msg, sizeof(msg), format, ap);
  va_end(ap);

  plugin_log(level, "%s plugin: %s", name, msg);
}

int parse_value(const char *value_orig, value_t *ret_value, int ds_type) {
  char *value;
  char *endptr = NULL;
  size_t value_len;

  if (value_orig == NULL)
    return EINVAL;

  value = strdup(value_orig);
  if (value == NULL)
    return ENOMEM;
  value_len = strlen(value);

  while ((value_len > 0) && isspace((int)value[value_len - 1])) {
    value[value_len - 1] = '\0';
    value_len--;
  }

  switch (ds_type) {
  case DS_TYPE_COUNTER:
    ret_value->counter = (counter_t)strtoull(value, &endptr, 0);
    break;

  case DS_TYPE_GAUGE:
    ret_value->gauge = (gauge_t)strtod(value, &endptr);
    break;

  case DS_TYPE_DERIVE:
    ret_value->derive = (derive_t)strtoll(value, &endptr, 0);
    break;

  case DS_TYPE_ABSOLUTE:
    ret_value->absolute = (absolute_t)strtoull(value, &endptr, 0);
    break;

  default:
    sfree(value);
    P_ERROR("parse_value: Invalid data source type: %i.", ds_type);
    return -1;
  }

  if (value == endptr) {
    P_ERROR("parse_value: Failed to parse string as %s: \"%s\".",
            DS_TYPE_TO_STRING(ds_type), value);
    sfree(value);
    return -1;
  } else if ((NULL != endptr) && ('\0' != *endptr))
    P_INFO("parse_value: Ignoring trailing garbage \"%s\" after %s value. "
           "Input string was \"%s\".",
           endptr, DS_TYPE_TO_STRING(ds_type), value_orig);

  sfree(value);
  return 0;
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
  }
  else
      diff = new_value - old_value;

  return diff;
}

/* cf_util_get_boolean */
int cf_util_get_boolean(const oconfig_item_t *ci, bool *ret_bool)
{
  if ((ci == NULL) || (ret_bool == NULL))
    return EINVAL;

  if ((ci->values_num != 1) || ((ci->values[0].type != OCONFIG_TYPE_BOOLEAN) &&
                              (ci->values[0].type != OCONFIG_TYPE_STRING)))
    return -1;

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
