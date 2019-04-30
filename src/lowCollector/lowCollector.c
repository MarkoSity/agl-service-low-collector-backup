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

#define COLLECTD_PATH "../build/src/collectd/./collectd_glue.so"

/* %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
                            INCLUDE
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% */

#include <dlfcn.h>
#include "api/api.h"

/* %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
                            AFB INITIALIZATION
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% */

static void afb_init(afb_req_t req)
{
  /* Variables definition */
  json_object *arg;
  afb_api_t api;
  userdata_t *userdata;
  json_object *res;

  /* Variables allocation */
  arg = json_object_new_object();
  res = json_object_new_object();

  /* Variables initialization */
  arg = afb_req_json(req);
  api = afb_req_get_api(req);
  userdata = afb_api_get_userdata(api);
  res = api_plugin_init(userdata, arg);

  /* Display the initialization output */
  AFB_API_NOTICE(afbBindingV3root, "Verbosity macro at level notice invoked at init invocation");
  afb_req_success(req, res, "Plugin initialization.");
  return;
}

/* %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
                            AFB CONFIGURATION
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% */

static void afb_config(afb_req_t req)
{
  /* Variables definition */
  json_object *arg;
  afb_api_t api;
  userdata_t *userdata;
  json_object *res;

  /* Variables allocation */
  arg = json_object_new_object();
  res = json_object_new_object();

  /* Variables initialization */
  arg = afb_req_json(req);
  api = afb_req_get_api(req);
  userdata = afb_api_get_userdata(api);
  res = api_plugin_config(userdata, arg);

  /* Display the configuration output */
  AFB_API_NOTICE(afbBindingV3root, "Verbosity macro at level notice invoked at config invocation");
  afb_req_success(req, res, "Plugin configuration.");
  return;
}

/* %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
                            AFB READ
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% */
static void afb_read(afb_req_t req)
{
  /* Variables definition */
  json_object *arg;
  afb_api_t api;
  userdata_t *userdata;
  json_object *res;

  /* Variables allocation */
  arg = json_object_new_object();
  res = json_object_new_object();

  /* Variables initialization */
  arg = afb_req_json(req);
  api = afb_req_get_api(req);
  userdata = afb_api_get_userdata(api);
  res = api_plugin_read(userdata, arg);

  /* Display the configuration output */
  AFB_API_NOTICE(afbBindingV3root, "Verbosity macro at level notice invoked at read invocation");
  afb_req_success(req, res, "Plugin read.");
  return;
}

static void afb_reset(afb_req_t req)
{
  /* Variables definition */
  json_object *arg;
  afb_api_t api;
  userdata_t *userdata;
  json_object *res;

  /* Variables allocation */
  arg = json_object_new_object();
  res = json_object_new_object();

  /* Variables initialization */
  arg = afb_req_json(req);
  api = afb_req_get_api(req);
  userdata = afb_api_get_userdata(api);
  res = api_plugin_reset(userdata, arg);

  /* Display the configuration output */
  AFB_API_NOTICE(afbBindingV3root, "Verbosity macro at level notice invoked at reset invocation");
  afb_req_success(req, res, "Plugin reset.");
  return;
}

// Binder initialization function
static int initialization(afb_api_t api)
{
    AFB_API_NOTICE(api, "Initialization");

    /* Userdata declaration */
    userdata_t *userdata;

    /* Userdata allocation */
    userdata = (userdata_t*)malloc(sizeof(userdata_t));
    userdata->handle_collectd = NULL;
    userdata->handle_cpu = NULL;
    userdata->handle_memory = NULL;
    userdata->handle_processes = NULL;

    /* Let's open the Collectd glue library */
    userdata->handle_collectd = dlopen(COLLECTD_PATH, RTLD_NOW | RTLD_GLOBAL);
    if(!userdata->handle_collectd)
    {
      printf("error : %s\n", dlerror());
      return -1;
    }

    /* Establish the link between tha api and the userdata */
    afb_api_set_userdata(api, userdata);

    AFB_API_NOTICE(api, "Initialization finished");
	return 0;
}

// Binder preinitialization
int preinit(afb_api_t api)
{
    AFB_API_NOTICE(api, "Preinitialization");

   // Set the verb of this API
   if(afb_api_add_verb(api, "init", "Initialize the global plugin variable with the plugin we want", afb_init, NULL, NULL, AFB_SESSION_NONE, 0)
   || afb_api_add_verb(api, "config", "Store the callbacks of the desired plugin", afb_config, NULL, NULL, AFB_SESSION_NONE, 0)
   || afb_api_add_verb(api, "read", "Read the metrics plugin previously configured", afb_read, NULL, NULL, AFB_SESSION_NONE, 0)
   || afb_api_add_verb(api, "reset", "Delete plugins stored in the list", afb_reset, NULL, NULL, AFB_SESSION_NONE, 0))
   {
        AFB_API_ERROR(api, "Verb initialization for API collector failed");
   }

   // Define the initialization function
   afb_api_on_init(api, initialization);

    // Now the api is set and ready to work
    afb_api_seal(api);

   AFB_API_NOTICE(api, "Preinitialization finished");

	return 0;
}

const afb_binding_t afbBindingExport = {
	.api = "collector",
	.specification = NULL,
	.preinit = preinit,
	.provide_class = NULL,
	.require_class = NULL,
	.require_api = NULL,
	.noconcurrency = 0
};
