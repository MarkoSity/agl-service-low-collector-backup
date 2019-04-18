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
    SET(SRC_CPU
    "${CMAKE_CURRENT_SOURCE_DIR}/sources/src/cpu.c"
    #"cpu.c"
    )

    ADD_LIBRARY(${TARGET_NAME} SHARED ${SRC_CPU})

    TARGET_INCLUDE_DIRECTORIES(${TARGET_NAME} PUBLIC
      ${CMAKE_CURRENT_SOURCE_DIR}
      ${CMAKE_CURRENT_SOURCE_DIR}/sources/src
      ${CMAKE_CURRENT_SOURCE_DIR}/sources/src/daemon
    )

    # FIXME - Try to remove FP_LAYOUT_NEED_NOTHING and use config.h instead
    TARGET_COMPILE_DEFINITIONS(${TARGET_NAME}
        PUBLIC -DFP_LAYOUT_NEED_NOTHING
        PUBLIC -DHAVE_CONFIG_H
    )

    target_compile_options(${TARGET_NAME}
        PUBLIC -Wno-conversion -Wno-int-conversion
        PUBLIC -std=c99
    )

  SET_TARGET_PROPERTIES(${TARGET_NAME} PROPERTIES
        LABELS "PLUGINS"
        PREFIX ""
        SUFFIX ".so"
        OUTPUT_NAME ${TARGET_NAME}
  )

  PROJECT_TARGET_ADD(mem)
    SET(SRC_MEM
		"memory.c"
    )

  PROJECT_TARGET_ADD(collectd_glue)

    set(SRC_COLLECTD_GLUE
		"collectd_glue.c"
    )

    ADD_LIBRARY(${TARGET_NAME} SHARED ${SRC_COLLECTD_GLUE})

    TARGET_INCLUDE_DIRECTORIES(${TARGET_NAME} PUBLIC
      ${CMAKE_CURRENT_SOURCE_DIR}
      ${CMAKE_CURRENT_SOURCE_DIR}/sources/src
      ${CMAKE_CURRENT_SOURCE_DIR}/sources/src/daemon
    )

    # FIXME - Try to remove FP_LAYOUT_NEED_NOTHING and use config.h instead
    TARGET_COMPILE_DEFINITIONS(${TARGET_NAME}
        PUBLIC -DFP_LAYOUT_NEED_NOTHING
        PUBLIC -DHAVE_CONFIG_H
    )

  SET_TARGET_PROPERTIES(${TARGET_NAME} PROPERTIES
        LABELS "COLLECTD_GLUE"
        PREFIX ""
        SUFFIX ".so"
        OUTPUT_NAME ${TARGET_NAME}
  )
