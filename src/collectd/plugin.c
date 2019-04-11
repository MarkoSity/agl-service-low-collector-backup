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

pthread_mutex_t strerror_r_lock = PTHREAD_MUTEX_INITIALIZER;

/* %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
                            Global plugin variables
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% */

plugin_t *Plugin_collectd;
metrics_t *Metrics_collectd;

/* Plugin init */
int plugin_init(void)
{
    Plugin_collectd = (plugin_t*)malloc(sizeof(plugin_t));
    if(!Plugin_collectd)
    {
        return -1;
    }
    printf("Plugin initialized\n");
    return 0;
}

/* Plugin deinit */
int plugin_deinit(void)
{
    free(Plugin_collectd);
    if(Plugin_collectd)
    {
        return -1;
    }
    return 0;
}

/* Metrics INIT */
int metrics_init(value_list_t *list)
{
    Metrics_collectd = (metrics_t*)malloc(sizeof(metrics_t));
    Metrics_collectd->metrics = (value_list_t*)malloc(sizeof(value_list_t));
    if(!Metrics_collectd || !Metrics_collectd->metrics)
    {
        printf("metrics_init : Metrics malloc failed \n");
        return -1;
    }
    memcpy(&Metrics_collectd->metrics[0], list, sizeof(value_list_t));
    Metrics_collectd->metrics->values = (value_t*)malloc(list->values_len*sizeof(value_t));
    if(!Metrics_collectd->metrics->values) {
        printf("metrics_init : Metrics malloc failed \n");
        return -2;
    }
    memcpy(Metrics_collectd->metrics[0].values, list->values, list->values_len*sizeof(value_t));
    Metrics_collectd->size = 1;
    printf("Metrics initialized\n");
    return 0;
}

int metrics_add(value_list_t *list)
{
     /* Trivial case, the metrics list is empty */
    if(!Metrics_collectd)
    {
        if(metrics_init(list)) {
            printf("%s : Metrics malloc failed\n", __func__);
            return -1;
        }

        printf("Value to add : %lf\n", list->values->gauge);
        printf("Metrics_add : %lf\n", Metrics_collectd->metrics[Metrics_collectd->size - 1].values->gauge);
        printf("Metrics size : %ld\n", Metrics_collectd->size);
    }

    /* Here we have to add at the end of the table the argument list */
    else
    {
        printf("Metrics previous content : \n\t");
        for(int i = 0 ; i != Metrics_collectd->size ; i ++)
        {
            printf("%d Metrics previous value : %lf\n\t", i, Metrics_collectd->metrics[i].values->gauge);
        }
        printf("\n");

        /* Increase the metrics table size and realloc an other space in the table to store an other metric*/
        printf("value to add : %lf\n", list->values->gauge);
        printf("current size : %ld\n\n", Metrics_collectd->size);

       if(! (Metrics_collectd->metrics = realloc(Metrics_collectd->metrics, (Metrics_collectd->size + 1)*sizeof(value_list_t)))) {
           printf("%s : Metrics realloc failed\n", __func__);
           return -1;
       }

        /* Copy the new value in the tampon */
        memcpy(&Metrics_collectd->metrics[Metrics_collectd->size], list, sizeof(value_list_t));
        Metrics_collectd->metrics[Metrics_collectd->size].values = (value_t*)malloc(list->values_len*sizeof(value_t));
        memcpy(Metrics_collectd->metrics[Metrics_collectd->size].values, list->values, list->values_len*sizeof(value_t));

        Metrics_collectd->size ++;
    }

    printf("Metrics content : \n\t");
    for(int i = 0 ; i != Metrics_collectd->size ; i ++)
    {
        printf("%d Metrics value : %lf\n\t", i, Metrics_collectd->metrics[i].values->gauge);
    }
    printf("\n");

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
int plugin_register_init(const char *name, int (*callback)(void))
{
    /* Store in the plugin the desired callback */
    Plugin_collectd->init = callback;
    printf("Initialization callback stored in the plugin\n");
    return 0;
}

/* CONFIGURATION */
int plugin_register_config(const char *name,
                           int (*callback)(const char *key,
                           const char *val),
                           const char **keys, int keys_num)
{

    /* Store in the plugin the desired callback */
    Plugin_collectd->config = callback;
    printf("Configuration callback stored in the plugin\n");
    return 0;
}

/* LOG */
void plugin_log(int level, const char *format, ...)
{
    printf("plugin_log\n");
}

/* DISPATCH VALUES */
int plugin_dispatch_values(value_list_t *vl)
{
    printf("plugin_dispatch_values\n");

    if(metrics_add(vl))
    {
        printf("Plugin dispatch values : Error metrics add\n");
        return -1;
    }

    return 0;
}

/* READ */
int plugin_register_read(const char *name, int (*callback)(user_data_t *))
{
    /* Store in the plugin the desired callback */
    Plugin_collectd->read = callback;
    printf("Read callback stored in the plugin\n");
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

    switch (ds_type) {
    case DS_TYPE_DERIVE: {
        derive_t diff = value.derive - state->last_value.derive;
        *ret_rate = ((gauge_t)diff) / ((gauge_t)interval);
        break;
    }
    case DS_TYPE_GAUGE: {
        *ret_rate = value.gauge;
        break;
    }
    case DS_TYPE_COUNTER: {
        counter_t diff = counter_diff(state->last_value.counter, value.counter);
        *ret_rate = ((gauge_t)diff) / ((gauge_t)interval);
        break;
    }
    case DS_TYPE_ABSOLUTE: {
        absolute_t diff = value.absolute;
        *ret_rate = ((gauge_t)diff) / ((gauge_t)interval);
        break;
    }
    default:
        return EINVAL;
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
        ERROR("cdtime: clock_gettime failed: %s\n", STRERRNO);
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

/* cdrand_d */
double cdrand_d(void) {
    return 0;
}

/* check_drop_value */
bool check_drop_value(void)
{
    return true;
}

/* meta_data_destroy */
void meta_data_destroy(meta_data_t *md)
{
    return;
}

/* meta_data_clone */
meta_data_t *meta_data_clone(meta_data_t *orig)
{
    return NULL;
}

/* get_drop_probability */
double get_drop_probability(void)
{
    return 0;
}

/* md_entry_free */
void md_entry_free(meta_entry_t *e)
{
    return;
}

/* meta_data_create */
meta_data_t *meta_data_create(void)
{
    return NULL;
}

/* md_entry_clone */
meta_entry_t *md_entry_clone(const meta_entry_t *orig)
{
    return NULL;
}
