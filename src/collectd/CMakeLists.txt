###########################################################################
# Copyright 2015, 2016, 2017 IoT.bzh
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
###########################################################################

PROJECT_TARGET_ADD(cpu)
    set(SRC_CPU
		"cpu.c"
    )

    ADD_LIBRARY(${TARGET_NAME} SHARED ${SRC_CPU})

  SET_TARGET_PROPERTIES(${TARGET_NAME} PROPERTIES
        LABELS "PLUGIN"
        PREFIX ""
        SUFFIX ".so"
        OUTPUT_NAME ${TARGET_NAME}
  )

  PROJECT_TARGET_ADD(mem)
    set(SRC_MEM
		"memory.c"
    )

    ADD_LIBRARY(${TARGET_NAME} SHARED ${SRC_MEM})

  SET_TARGET_PROPERTIES(${TARGET_NAME} PROPERTIES
        LABELS "PLUGIN"
        PREFIX ""
        SUFFIX ".so"
        OUTPUT_NAME ${TARGET_NAME}
  )

  PROJECT_TARGET_ADD(plugin)

    set(SRC_PLUGIN
		"plugin.c"
    )

    ADD_LIBRARY(${TARGET_NAME} SHARED ${SRC_PLUGIN})

  SET_TARGET_PROPERTIES(${TARGET_NAME} PROPERTIES
        LABELS "PLUGIN"
        PREFIX ""
        SUFFIX ".so"
        OUTPUT_NAME ${TARGET_NAME}
  )
