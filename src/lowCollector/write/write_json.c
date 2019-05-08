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

/* %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
                            INCLUDES
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% */

#include "write_json.h"
#include "../userdata.h"

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
                          WRITE JSON CALLBACK
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% */

json_object *write_json(metrics_list_t *metrics_list)
{
  /* Variables declaration */
  /* Json */
  json_object *res;
  json_object *value;
  json_object *value_list;
  json_object *type;
  json_object *type_instance;
  json_object *plugin_with_instance;
  json_object *plugin_without_instance;
  json_object *plugin;

  /* String label */
  char *host_label;
  char *plugin_label;
  char *plugin_instance_label;
  char *type_label;
  char *type_instance_label;

  int index = 0;

  /* Variable allocation */
  res = json_object_new_object();
  if(!res)
    return json_object_new_string(ERR_ALLOC_CHAR);

  /* Ensure the metrics list is not NULL */
  if(!metrics_list)
    return json_object_new_string(ERR_METRICS_NULL_CHAR);

  /* Ensure the metrics list is not empty */
  if(!metrics_list->size)
    return json_object_new_string(ERR_METRICS_EMPTY_CHAR);

  /* While index is not out of bond */
  while(index != metrics_list->size)
  {
    /* Retrieve and store the metrics host label */
    host_label = strdup(metrics_list->metrics[index].host);
    if(!host_label)
      return json_object_new_string(ERR_ALLOC_CHAR);

    /* Json allocation */
    plugin = json_object_new_object();
    if(!plugin)
      return json_object_new_string(ERR_ALLOC_CHAR);

    /* While the metrics host do not change */
    while(!strncmp(metrics_list->metrics[index].host, host_label,
         max_size(strlen(metrics_list->metrics[index].host), strlen(host_label))))
    {
      /* Retrieve and store the metrics plugin */
      plugin_label = strdup(metrics_list->metrics[index].plugin);
      if(!plugin_label)
        return json_object_new_string(ERR_ALLOC_CHAR);

      /* Json allocation */
      plugin_without_instance = json_object_new_object();
      if(!plugin_without_instance)
        return json_object_new_string(ERR_ALLOC_CHAR);

      plugin_with_instance = json_object_new_object();
      if(!plugin_with_instance)
        return json_object_new_string(ERR_ALLOC_CHAR);

      /* While the metrics plugin do not change */
      while(!strncmp(metrics_list->metrics[index].host, host_label,
           max_size(strlen(metrics_list->metrics[index].host), strlen(host_label))) &&
           !strncmp(metrics_list->metrics[index].plugin, plugin_label,
           max_size(strlen(metrics_list->metrics[index].plugin), strlen(plugin_label))))
      {
        /* Retrieve and store the metrics plugin instance label */
        plugin_instance_label = strdup(metrics_list->metrics[index].plugin_instance);
        if(!plugin_instance_label)
          return json_object_new_string(ERR_ALLOC_CHAR);

        /* Json allocation */
        type = json_object_new_object();
        if(!type)
          return json_object_new_string(ERR_ALLOC_CHAR);

        /* While the metrics plugin instance do not change */
        while(!strncmp(metrics_list->metrics[index].host, host_label,
              max_size(strlen(metrics_list->metrics[index].host), strlen(host_label))) &&
              !strncmp(metrics_list->metrics[index].plugin, plugin_label,
              max_size(strlen(metrics_list->metrics[index].plugin), strlen(plugin_label))) &&
              !strncmp(metrics_list->metrics[index].plugin_instance, plugin_instance_label,
              max_size(strlen(metrics_list->metrics[index].plugin_instance), strlen(plugin_instance_label))))
        {
          /* Retrieve and store the metrics type */
          type_label = (char *) strdup(metrics_list->metrics[index].type);
          if(!type_label)
            return json_object_new_string(ERR_ALLOC_CHAR);

          /* Json allocation */
          type_instance = json_object_new_object();
          if(!type_instance)
            return json_object_new_string(ERR_ALLOC_CHAR);

          /* While the metrics type do not change */
          while(!strncmp(metrics_list->metrics[index].host, host_label,
                max_size(strlen(metrics_list->metrics[index].host), strlen(host_label))) &&
                !strncmp(metrics_list->metrics[index].plugin, plugin_label,
                max_size(strlen(metrics_list->metrics[index].plugin), strlen(plugin_label))) &&
                !strncmp(metrics_list->metrics[index].plugin_instance, plugin_instance_label,
                max_size(strlen(metrics_list->metrics[index].plugin_instance), strlen(plugin_instance_label))) &&
                !strncmp(metrics_list->metrics[index].type, type_label,
                max_size(strlen(metrics_list->metrics[index].type), strlen(type_label))))
          {
            /* Retrieve and store the metrics type instance*/
            type_instance_label = (char *)strdup(metrics_list->metrics[index].type_instance);
            if(!type_instance_label)
              return json_object_new_string(ERR_ALLOC_CHAR);

            /* Json allocation */
            value_list = json_object_new_array();
            if(!value_list)
              return json_object_new_string(ERR_ALLOC_CHAR);

            /* While the type instance do not change */
            while(!strncmp(metrics_list->metrics[index].host, host_label,
                  max_size(strlen(metrics_list->metrics[index].host), strlen(host_label))) &&
                  !strncmp(metrics_list->metrics[index].plugin, plugin_label,
                  max_size(strlen(metrics_list->metrics[index].plugin), strlen(plugin_label))) &&
                  !strncmp(metrics_list->metrics[index].plugin_instance, plugin_instance_label,
                  max_size(strlen(metrics_list->metrics[index].plugin_instance), strlen(plugin_instance_label))) &&
                  !strncmp(metrics_list->metrics[index].type, type_label,
                  max_size(strlen(metrics_list->metrics[index].type), strlen(type_label))) &&
                  !strncmp(metrics_list->metrics[index].type_instance, type_instance_label,
                  max_size(strlen(metrics_list->metrics[index].type_instance), strlen(type_instance_label))))
            {
              /* For each one of the value stored in the metrics */
              for(int i = 0 ; i != metrics_list->metrics[index].values_len ; i++)
              {
                /* Store the value in the value list */
                value = json_object_new_double(metrics_list->metrics[index].values[i].gauge);
                json_object_array_add(value_list, value);
              }

              /* Increment the index */
              index++;
            }

            /* If the previous metrics values have a type instance not empty */
            if(strncmp(type_instance_label, "", strlen(type_instance_label)))
              json_object_object_add(type_instance, type_instance_label, value_list);

            else
              json_object_object_add(type_instance, type_instance_label, value_list);
              /* wrap_json_object_add(type_instance, value_list); */

            /* Type instance label desallocation */
            sfree(type_instance_label);
          }

          json_object_object_add(type, type_label, type_instance);

          /* Type label desallocation */
          sfree(type_label);
        }

        /* If the current metrics do not have a plugin instance */
        if(!strncmp(plugin_instance_label, "", strlen(plugin_instance_label)))
          wrap_json_object_add(plugin_without_instance, type);

        /* Else the metrics has a plugin instance */
        else
          json_object_object_add(plugin_with_instance, plugin_instance_label, type);

        /* Plugin instance label desallocation */
        sfree(plugin_instance_label);
      }

      /* Gather the two previous json in a temporary json plugin */
      wrap_json_object_add(plugin_with_instance, plugin_without_instance);

      /* Associate the previous json with their associated key */
      json_object_object_add(plugin, plugin_label, plugin_with_instance);

      /* Plugin label desallocation */
      sfree(plugin_label);
    }

    /* Associate the whole metrics value with their associated host */
    json_object_object_add(res, host_label, plugin);

    /* Host label desallocation */
    sfree(host_label);
  }
  
  return res;
}
