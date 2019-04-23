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

/* %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
                            DEFINE
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% */

#ifndef GAUGE_FORMAT
#define GAUGE_FORMAT "%.15g"
#endif

#define STATE_MISSING 15

#ifndef JSON_GAUGE_FORMAT
#define JSON_GAUGE_FORMAT GAUGE_FORMAT
#endif

/* %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
                            INCLUDES
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% */

#include "write_json.h"

/* %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
                            TYPEDEF & STRUCTURE
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% */

typedef struct cache_entry_s {
  char name[6 * DATA_MAX_NAME_LEN];
  size_t values_num;
  gauge_t *values_gauge;
  value_t *values_raw;
  cdtime_t last_time;
  cdtime_t last_update;
  cdtime_t interval;
  int state;
  int hits;
  gauge_t *history;
  size_t history_index;
  size_t history_length;
  meta_data_t *meta;
} cache_entry_t;

typedef struct c_avl_node_s c_avl_node_t;
struct c_avl_node_s {
  void *key;
  void *value;

  int height;
  struct c_avl_node_s *left;
  struct c_avl_node_s *right;
  struct c_avl_node_s *parent;
};

typedef struct c_avl_tree_s c_avl_tree_t;
struct c_avl_tree_s {
  c_avl_node_t *root;
  int (*compare)(const void *, const void *);
  int size;
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

/* %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
                              GLOBAL VARIABLE
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% */

static pthread_mutex_t cache_lock = PTHREAD_MUTEX_INITIALIZER;
static c_avl_tree_t *cache_tree;

/* %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
                          WRITE JSON CALLBACK
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% */

json_object *write_json(metrics_t *metrics_list)
{
  /* Variables definition */
  json_object *res;
  json_object *res_tmp;
  json_object *res_info;
  json_object *res_value;
  json_object *res_value_tmp;
  char *index_metrics;
  char *index_value;

  /* Variables allocation */
  res = json_object_new_object();
  index_metrics = (char *)malloc(10*sizeof(char) + 1);
  index_value = (char *)malloc(10*sizeof(char) + 1);

  /* Ensure the metrics list is not NULL */
  if(!metrics_list)
    return json_object_new_string("Metrics list NULL");

  /* Ensure the metrics list is not empty */
  if(!metrics_list->size)
    return json_object_new_string("Metrics list empty");

  /* For each one of the metrics in the metrics list */
  for(int i = 0 ; i != metrics_list->size ; i++)
  {
    /* Because the jsons are freed when using json_object_object_add, we have to allocate them at each new loop */
    res_info = json_object_new_object();
    res_value = json_object_new_object();
    res_tmp = json_object_new_object();

    /* Convert the index metrics in string for the json key */
    sprintf(index_metrics, "%d", i+1);

    /* Make sure the host is known */
    if(!(strncmp(metrics_list->metrics[i].host, "", strlen(metrics_list->metrics[i].host))))
      strcpy(metrics_list->metrics[i].host, "Low Collector");

    /* Pack the metrics information in a tampon response*/
    wrap_json_pack(&res_info,
                   "{s:[ss] s:[ss] s:I}",
                   "plugin",
                   metrics_list->metrics[i].plugin, metrics_list->metrics[i].plugin_instance,
                   "type",
                   metrics_list->metrics[i].type, metrics_list->metrics[i].type_instance,
                   "values length",
                   metrics_list->metrics[i].values_len);

    /* Add the json metrics infos the json tampon response */
    json_object_object_add(res_tmp,
                           "infos",
                           res_info);

    /* Gather all the metrics values in a json */
    for(int j = 0 ; j != metrics_list->metrics[i].values_len ; j++)
    {
      /* Because the jsons are freed when using json_object_object_add, we have to allocate them at each new loop */
      res_value_tmp = json_object_new_object();

      /* Convert the index value in string for the json key */
      sprintf(index_value, "%d", j+1);
      wrap_json_pack(&res_value_tmp,
                     "{s:f s:I s:I s:I}",
                     "gauge", metrics_list->metrics[i].values[j].gauge,
                     "absolute", metrics_list->metrics[i].values[j].absolute,
                     "derive", metrics_list->metrics[i].values[j].derive,
                     "counter", metrics_list->metrics[i].values[j].counter);
      /* Add the tampin value json in the final response with a good key */
      json_object_object_add(res_value,
                             index_value,
                             res_value_tmp);
    }

    /* Add the json metrics values the json tampon response */
    json_object_object_add(res_tmp,
                           "values",
                           res_value);

    /* Add the tampon response in the final one with the good key */
    json_object_object_add(res,
                           strcat(strcat(metrics_list->metrics[i].host, " "), index_metrics),
                           res_tmp);
  }

  return res;
}

c_avl_node_t *search(c_avl_tree_t *t, const void *key) {
  c_avl_node_t *n;
  int cmp;

  n = t->root;
  while (n != NULL) {
    cmp = t->compare(key, n->key);
    if (cmp == 0)
      return n;
    else if (cmp < 0)
      n = n->left;
    else
      n = n->right;
  }

  return NULL;
}

int c_avl_get(c_avl_tree_t *t, const void *key, void **value) {
  c_avl_node_t *n;

  assert(t != NULL);

  n = search(t, key);
  if (n == NULL)
    return -1;

  if (value != NULL)
    *value = n->value;

  return 0;
}

int uc_get_rate_by_name(const char *name, gauge_t **ret_values,
                        size_t *ret_values_num) {
  gauge_t *ret = NULL;
  size_t ret_num = 0;
  cache_entry_t *ce = NULL;
  int status = 0;

  pthread_mutex_lock(&cache_lock);

  if (c_avl_get(cache_tree, name, (void *)&ce) == 0) {
    assert(ce != NULL);

    /* remove missing values from getval */
    if (ce->state == STATE_MISSING) {
      DEBUG("utils_cache: uc_get_rate_by_name: requested metric \"%s\" is in "
            "state \"missing\".",
            name);
      status = -1;
    } else {
      ret_num = ce->values_num;
      ret = malloc(ret_num * sizeof(*ret));
      if (ret == NULL) {
        ERROR("utils_cache: uc_get_rate_by_name: malloc failed.");
        status = -1;
      } else {
        memcpy(ret, ce->values_gauge, ret_num * sizeof(gauge_t));
      }
    }
  } else {
    DEBUG("utils_cache: uc_get_rate_by_name: No such value: %s", name);
    status = -1;
  }

  pthread_mutex_unlock(&cache_lock);

  if (status == 0) {
    *ret_values = ret;
    *ret_values_num = ret_num;
  }

  return status;
}

gauge_t *uc_get_rate(const data_set_t *ds, const value_list_t *vl) {
  char name[6 * DATA_MAX_NAME_LEN];
  gauge_t *ret = NULL;
  size_t ret_num = 0;
  int status;

  if (FORMAT_VL(name, sizeof(name), vl) != 0) {
    ERROR("utils_cache: uc_get_rate: FORMAT_VL failed.");
    return NULL;
  }

  status = uc_get_rate_by_name(name, &ret, &ret_num);
  if (status != 0)
    return NULL;

  /* This is important - the caller has no other way of knowing how many
   * values are returned. */
  if (ret_num != ds->ds_num) {
    ERROR("utils_cache: uc_get_rate: ds[%s] has %" PRIsz " values, "
          "but uc_get_rate_by_name returned %" PRIsz ".",
          ds->type, ds->ds_num, ret_num);
    sfree(ret);
    return NULL;
  }

  return ret;
}

int values_to_json(char *buffer, size_t buffer_size,
                          const data_set_t *ds, const value_list_t *vl,
                          int store_rates) {
  size_t offset = 0;
  gauge_t *rates = NULL;

  memset(buffer, 0, buffer_size);

#define BUFFER_ADD(...)                                                        \
  do {                                                                         \
    int status;                                                                \
    status = snprintf(buffer + offset, buffer_size - offset, __VA_ARGS__);     \
    if (status < 1) {                                                          \
      sfree(rates);                                                            \
      return -1;                                                               \
    } else if (((size_t)status) >= (buffer_size - offset)) {                   \
      sfree(rates);                                                            \
      return -ENOMEM;                                                          \
    } else                                                                     \
      offset += ((size_t)status);                                              \
  } while (0)

  BUFFER_ADD("[");
  for (size_t i = 0; i < ds->ds_num; i++) {
    if (i > 0)
      BUFFER_ADD(",");

    if (ds->ds[i].type == DS_TYPE_GAUGE) {
      if (isfinite(vl->values[i].gauge))
        BUFFER_ADD(JSON_GAUGE_FORMAT, vl->values[i].gauge);
      else
        BUFFER_ADD("null");
    } else if (store_rates) {
      if (rates == NULL)
        rates = uc_get_rate(ds, vl);
      if (rates == NULL) {
        WARNING("utils_format_json: uc_get_rate failed.");
        sfree(rates);
        return -1;
      }

      if (isfinite(rates[i]))
        BUFFER_ADD(JSON_GAUGE_FORMAT, rates[i]);
      else
        BUFFER_ADD("null");
    } else if (ds->ds[i].type == DS_TYPE_COUNTER)
      BUFFER_ADD("%" PRIu64, (uint64_t)vl->values[i].counter);
    else if (ds->ds[i].type == DS_TYPE_DERIVE)
      BUFFER_ADD("%" PRIi64, vl->values[i].derive);
    else if (ds->ds[i].type == DS_TYPE_ABSOLUTE)
      BUFFER_ADD("%" PRIu64, vl->values[i].absolute);
    else {
      ERROR("format_json: Unknown data source type: %i", ds->ds[i].type);
      sfree(rates);
      return -1;
    }
  } /* for ds->ds_num */
  BUFFER_ADD("]");

#undef BUFFER_ADD

  sfree(rates);
  return 0;
}

int json_escape_string(char *buffer, size_t buffer_size, /* {{{ */
                              const char *string) {
  size_t dst_pos;

  if ((buffer == NULL) || (string == NULL))
    return -EINVAL;

  if (buffer_size < 3)
    return -ENOMEM;

  dst_pos = 0;

#define BUFFER_ADD(c)                                                          \
  do {                                                                         \
    if (dst_pos >= (buffer_size - 1)) {                                        \
      buffer[buffer_size - 1] = '\0';                                          \
      return -ENOMEM;                                                          \
    }                                                                          \
    buffer[dst_pos] = (c);                                                     \
    dst_pos++;                                                                 \
  } while (0)

  /* Escape special characters */
  BUFFER_ADD('"');
  for (size_t src_pos = 0; string[src_pos] != 0; src_pos++) {
    if ((string[src_pos] == '"') || (string[src_pos] == '\\')) {
      BUFFER_ADD('\\');
      BUFFER_ADD(string[src_pos]);
    } else if (string[src_pos] <= 0x001F)
      BUFFER_ADD('?');
    else
      BUFFER_ADD(string[src_pos]);
  } /* for */
  BUFFER_ADD('"');
  buffer[dst_pos] = 0;

#undef BUFFER_ADD

  return 0;
}

int dsnames_to_json(char *buffer, size_t buffer_size, /* {{{ */
                           const data_set_t *ds) {
  size_t offset = 0;

  memset(buffer, 0, buffer_size);

#define BUFFER_ADD(...)                                                        \
  do {                                                                         \
    int status;                                                                \
    status = snprintf(buffer + offset, buffer_size - offset, __VA_ARGS__);     \
    if (status < 1)                                                            \
      return -1;                                                               \
    else if (((size_t)status) >= (buffer_size - offset))                       \
      return -ENOMEM;                                                          \
    else                                                                       \
      offset += ((size_t)status);                                              \
  } while (0)

  BUFFER_ADD("[");
  for (size_t i = 0; i < ds->ds_num; i++) {
    if (i > 0)
      BUFFER_ADD(",");

    BUFFER_ADD("\"%s\"", ds->ds[i].name);
  } /* for ds->ds_num */
  BUFFER_ADD("]");

#undef BUFFER_ADD

  return 0;
}

int dstypes_to_json(char *buffer, size_t buffer_size,
                           const data_set_t *ds) {
  size_t offset = 0;

  memset(buffer, 0, buffer_size);

#define BUFFER_ADD(...)                                                        \
  do {                                                                         \
    int status;                                                                \
    status = snprintf(buffer + offset, buffer_size - offset, __VA_ARGS__);     \
    if (status < 1)                                                            \
      return -1;                                                               \
    else if (((size_t)status) >= (buffer_size - offset))                       \
      return -ENOMEM;                                                          \
    else                                                                       \
      offset += ((size_t)status);                                              \
  } while (0)

  BUFFER_ADD("[");
  for (size_t i = 0; i < ds->ds_num; i++) {
    if (i > 0)
      BUFFER_ADD(",");

    BUFFER_ADD("\"%s\"", DS_TYPE_TO_STRING(ds->ds[i].type));
  } /* for ds->ds_num */
  BUFFER_ADD("]");

#undef BUFFER_ADD

  return 0;
}

int meta_data_keys_to_json(char *buffer, size_t buffer_size, /* {{{ */
                                  meta_data_t *meta, char **keys,
                                  size_t keys_num) {
  size_t offset = 0;
  int status;

  buffer[0] = 0;

#define BUFFER_ADD(...)                                                        \
  do {                                                                         \
    status = snprintf(buffer + offset, buffer_size - offset, __VA_ARGS__);     \
    if (status < 1)                                                            \
      return -1;                                                               \
    else if (((size_t)status) >= (buffer_size - offset))                       \
      return -ENOMEM;                                                          \
    else                                                                       \
      offset += ((size_t)status);                                              \
  } while (0)

  for (size_t i = 0; i < keys_num; ++i) {
    int type;
    char *key = keys[i];

    type = meta_data_type(meta, key);
    if (type == MD_TYPE_STRING) {
      char *value = NULL;
      if (meta_data_get_string(meta, key, &value) == 0) {
        char temp[512] = "";

        status = json_escape_string(temp, sizeof(temp), value);
        sfree(value);
        if (status != 0)
          return status;

        BUFFER_ADD(",\"%s\":%s", key, temp);
      }
    } else if (type == MD_TYPE_SIGNED_INT) {
      int64_t value = 0;
      if (meta_data_get_signed_int(meta, key, &value) == 0)
        BUFFER_ADD(",\"%s\":%" PRIi64, key, value);
    } else if (type == MD_TYPE_UNSIGNED_INT) {
      uint64_t value = 0;
      if (meta_data_get_unsigned_int(meta, key, &value) == 0)
        BUFFER_ADD(",\"%s\":%" PRIu64, key, value);
    } else if (type == MD_TYPE_DOUBLE) {
      double value = 0.0;
      if (meta_data_get_double(meta, key, &value) == 0)
        BUFFER_ADD(",\"%s\":%f", key, value);
    } else if (type == MD_TYPE_BOOLEAN) {
      bool value = false;
      if (meta_data_get_boolean(meta, key, &value) == 0)
        BUFFER_ADD(",\"%s\":%s", key, value ? "true" : "false");
    }
  } /* for (keys) */

  if (offset == 0)
    return ENOENT;

  buffer[0] = '{'; /* replace leading ',' */
  BUFFER_ADD("}");

#undef BUFFER_ADD

  return 0;
}

static int meta_data_to_json(char *buffer, size_t buffer_size, /* {{{ */
                             meta_data_t *meta) {
  char **keys = NULL;
  size_t keys_num;
  int status;

  if ((buffer == NULL) || (buffer_size == 0) || (meta == NULL))
    return EINVAL;

  status = meta_data_toc(meta, &keys);
  if (status <= 0)
    return status;
  keys_num = (size_t)status;

  status = meta_data_keys_to_json(buffer, buffer_size, meta, keys, keys_num);

  for (size_t i = 0; i < keys_num; ++i)
    sfree(keys[i]);
  sfree(keys);

  return status;
}

/* Value_list_to_json */
int value_list_to_json(char *buffer, size_t buffer_size, /* {{{ */
                              const data_set_t *ds, const value_list_t *vl,
                              int store_rates) {
  char temp[512];
  size_t offset = 0;
  int status;

  memset(buffer, 0, buffer_size);

#define BUFFER_ADD(...)                                                        \
  do {                                                                         \
    status = snprintf(buffer + offset, buffer_size - offset, __VA_ARGS__);     \
    if (status < 1)                                                            \
      return -1;                                                               \
    else if (((size_t)status) >= (buffer_size - offset))                       \
      return -ENOMEM;                                                          \
    else                                                                       \
      offset += ((size_t)status);                                              \
  } while (0)



  /* All value lists have a leading comma. The first one will be replaced with
   * a square bracket in `format_json_finalize'. */
  BUFFER_ADD(",{");

  status = values_to_json(temp, sizeof(temp), ds, vl, store_rates);
  if (status != 0)
    return status;
  BUFFER_ADD("\"values\":%s", temp);

  status = dstypes_to_json(temp, sizeof(temp), ds);
  if (status != 0)
    return status;
  BUFFER_ADD(",\"dstypes\":%s", temp);

  status = dsnames_to_json(temp, sizeof(temp), ds);
  if (status != 0)
    return status;
  BUFFER_ADD(",\"dsnames\":%s", temp);

  BUFFER_ADD(",\"time\":%.3f", CDTIME_T_TO_DOUBLE(vl->time));
  BUFFER_ADD(",\"interval\":%.3f", CDTIME_T_TO_DOUBLE(vl->interval));

#define BUFFER_ADD_KEYVAL(key, value)                                          \
  do {                                                                         \
    status = json_escape_string(temp, sizeof(temp), (value));                  \
    if (status != 0)                                                           \
      return status;                                                           \
    BUFFER_ADD(",\"%s\":%s", (key), temp);                                     \
  } while (0)

  BUFFER_ADD_KEYVAL("host", vl->host);
  BUFFER_ADD_KEYVAL("plugin", vl->plugin);
  BUFFER_ADD_KEYVAL("plugin_instance", vl->plugin_instance);
  BUFFER_ADD_KEYVAL("type", vl->type);
  BUFFER_ADD_KEYVAL("type_instance", vl->type_instance);

  if (vl->meta != NULL) {
    char meta_buffer[buffer_size];
    memset(meta_buffer, 0, sizeof(meta_buffer));
    status = meta_data_to_json(meta_buffer, sizeof(meta_buffer), vl->meta);
    if (status != 0)
      return status;

    BUFFER_ADD(",\"meta\":%s", meta_buffer);
  } /* if (vl->meta != NULL) */

  BUFFER_ADD("}");

#undef BUFFER_ADD_KEYVAL
#undef BUFFER_ADD

  return 0;
}

meta_entry_t *md_entry_lookup(meta_data_t *md, /* {{{ */
                                     const char *key) {
  meta_entry_t *e;

  if ((md == NULL) || (key == NULL))
    return NULL;

  for (e = md->head; e != NULL; e = e->next)
    if (strcasecmp(key, e->key) == 0)
      break;

  return e;
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

int meta_data_get_boolean(meta_data_t *md, /* {{{ */
                          const char *key, bool *value) {
  meta_entry_t *e;

  if ((md == NULL) || (key == NULL) || (value == NULL))
    return -EINVAL;

  pthread_mutex_lock(&md->lock);

  e = md_entry_lookup(md, key);
  if (e == NULL) {
    pthread_mutex_unlock(&md->lock);
    return -ENOENT;
  }

  if (e->type != MD_TYPE_BOOLEAN) {
    ERROR("meta_data_get_boolean: Type mismatch for key `%s'", e->key);
    pthread_mutex_unlock(&md->lock);
    return -ENOENT;
  }

  *value = e->value.mv_boolean;

  pthread_mutex_unlock(&md->lock);
  return 0;
}

int meta_data_get_double(meta_data_t *md, /* {{{ */
                         const char *key, double *value) {
  meta_entry_t *e;

  if ((md == NULL) || (key == NULL) || (value == NULL))
    return -EINVAL;

  pthread_mutex_lock(&md->lock);

  e = md_entry_lookup(md, key);
  if (e == NULL) {
    pthread_mutex_unlock(&md->lock);
    return -ENOENT;
  }

  if (e->type != MD_TYPE_DOUBLE) {
    ERROR("meta_data_get_double: Type mismatch for key `%s'", e->key);
    pthread_mutex_unlock(&md->lock);
    return -ENOENT;
  }

  *value = e->value.mv_double;

  pthread_mutex_unlock(&md->lock);
  return 0;
}

int meta_data_type(meta_data_t *md, const char *key) /* {{{ */
{
  if ((md == NULL) || (key == NULL))
    return -EINVAL;

  pthread_mutex_lock(&md->lock);

  for (meta_entry_t *e = md->head; e != NULL; e = e->next) {
    if (strcasecmp(key, e->key) == 0) {
      pthread_mutex_unlock(&md->lock);
      return e->type;
    }
  }

  pthread_mutex_unlock(&md->lock);
  return 0;
}

int meta_data_get_unsigned_int(meta_data_t *md, /* {{{ */
                               const char *key, uint64_t *value) {
  meta_entry_t *e;

  if ((md == NULL) || (key == NULL) || (value == NULL))
    return -EINVAL;

  pthread_mutex_lock(&md->lock);

  e = md_entry_lookup(md, key);
  if (e == NULL) {
    pthread_mutex_unlock(&md->lock);
    return -ENOENT;
  }

  if (e->type != MD_TYPE_UNSIGNED_INT) {
    ERROR("meta_data_get_unsigned_int: Type mismatch for key `%s'", e->key);
    pthread_mutex_unlock(&md->lock);
    return -ENOENT;
  }

  *value = e->value.mv_unsigned_int;

  pthread_mutex_unlock(&md->lock);
  return 0;
}

int meta_data_toc(meta_data_t *md, char ***toc) /* {{{ */
{
  int i = 0, count = 0;

  if ((md == NULL) || (toc == NULL))
    return -EINVAL;

  pthread_mutex_lock(&md->lock);

  for (meta_entry_t *e = md->head; e != NULL; e = e->next)
    ++count;

  if (count == 0) {
    pthread_mutex_unlock(&md->lock);
    return count;
  }

  *toc = calloc(count, sizeof(**toc));
  for (meta_entry_t *e = md->head; e != NULL; e = e->next)
    (*toc)[i++] = strdup(e->key);

  pthread_mutex_unlock(&md->lock);
  return count;
}

int meta_data_get_signed_int(meta_data_t *md, /* {{{ */
                             const char *key, int64_t *value) {
  meta_entry_t *e;

  if ((md == NULL) || (key == NULL) || (value == NULL))
    return -EINVAL;

  pthread_mutex_lock(&md->lock);

  e = md_entry_lookup(md, key);
  if (e == NULL) {
    pthread_mutex_unlock(&md->lock);
    return -ENOENT;
  }

  if (e->type != MD_TYPE_SIGNED_INT) {
    ERROR("meta_data_get_signed_int: Type mismatch for key `%s'", e->key);
    pthread_mutex_unlock(&md->lock);
    return -ENOENT;
  }

  *value = e->value.mv_signed_int;

  pthread_mutex_unlock(&md->lock);
  return 0;
}

int format_name(char *ret, int ret_len, const char *hostname,
                const char *plugin, const char *plugin_instance,
                const char *type, const char *type_instance) {
  char *buffer;
  size_t buffer_size;

  buffer = ret;
  buffer_size = (size_t)ret_len;

#define APPEND(str)                                                            \
  do {                                                                         \
    size_t l = strlen(str);                                                    \
    if (l >= buffer_size)                                                      \
      return ENOBUFS;                                                          \
    memcpy(buffer, (str), l);                                                  \
    buffer += l;                                                               \
    buffer_size -= l;                                                          \
  } while (0)

  assert(plugin != NULL);
  assert(type != NULL);

  APPEND(hostname);
  APPEND("/");
  APPEND(plugin);
  if ((plugin_instance != NULL) && (plugin_instance[0] != 0)) {
    APPEND("-");
    APPEND(plugin_instance);
  }
  APPEND("/");
  APPEND(type);
  if ((type_instance != NULL) && (type_instance[0] != 0)) {
    APPEND("-");
    APPEND(type_instance);
  }
  assert(buffer_size > 0);
  buffer[0] = 0;

#undef APPEND
  return 0;
}

char *md_strdup(const char *orig) /* {{{ */
{
  size_t sz;
  char *dest;

  if (orig == NULL)
    return NULL;

  sz = strlen(orig) + 1;
  dest = malloc(sz);
  if (dest == NULL)
    return NULL;

  memcpy(dest, orig, sz);

  return dest;
}

int meta_data_get_string(meta_data_t *md, /* {{{ */
                         const char *key, char **value) {
  meta_entry_t *e;
  char *temp;

  if ((md == NULL) || (key == NULL) || (value == NULL))
    return -EINVAL;

  pthread_mutex_lock(&md->lock);

  e = md_entry_lookup(md, key);
  if (e == NULL) {
    pthread_mutex_unlock(&md->lock);
    return -ENOENT;
  }

  if (e->type != MD_TYPE_STRING) {
    ERROR("meta_data_get_string: Type mismatch for key `%s'", e->key);
    pthread_mutex_unlock(&md->lock);
    return -ENOENT;
  }

  temp = md_strdup(e->value.mv_string);
  if (temp == NULL) {
    pthread_mutex_unlock(&md->lock);
    ERROR("meta_data_get_string: md_strdup failed.");
    return -ENOMEM;
  }

  pthread_mutex_unlock(&md->lock);

  *value = temp;

  return 0;
}

json_object *write_json_collectd(metrics_t *metrics_list)
{

  /* Variables definition */
  json_object *res;
  json_object *res_metrics;
  char *index;
  int status;
  char *buffer;
  data_set_t *data_set;
  size_t buffer_len = 1024;
  int store_rate = 0;

  /* variables allocation */
  res = json_object_new_object();
  res_metrics = json_object_new_object();
  index = (char *)malloc(100*sizeof(int)*sizeof(char));
  buffer = (char *)malloc(buffer_len*sizeof(char));
  data_set = (data_set_t *)malloc(sizeof(data_set_t));
  data_set->ds = (data_source_t *)malloc(sizeof(data_source_t));

  /* Ensure the metrics list is not NULL */
  if(!metrics_list)
    return json_object_new_string("Metrics list NULL.");

  /* Ensure the metrics list is not empty */
  if(!metrics_list->size)
    return json_object_new_string("Metrics list empty.");

  /* Data_set configuration */
  data_set->ds_num = 1;
  data_set->ds->max = NAN;
  data_set->ds->min = NAN;
  strcpy(data_set->ds->name, "value");
  data_set->ds->type = DS_TYPE_GAUGE;

  /* For each one of the metrics stored in the metrics list */
  for(int i = 0 ; i != metrics_list->size ; i++)
  {
    /* Copy the type of the metrics into the data_set */
    strcpy(data_set->type, metrics_list->metrics[i].type);
    status = value_list_to_json(buffer, buffer_len, data_set, &metrics_list->metrics[i], store_rate);
    if(status)
      return json_object_new_string("Fail to parse the metrics.");

    else
    {
      sprintf(index, "%d", i);
      res_metrics = json_object_new_string(buffer);
      json_object_object_add(res, index, res_metrics);
    }
  }
  return res;
}
