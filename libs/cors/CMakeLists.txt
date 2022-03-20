# Copyright (c) 2022 midnightBITS
# This file is licensed under MIT license (see LICENSE for details)

set(PLUGIN_NAME cors)

set(SRCS
  include/http-router/filters/${PLUGIN_NAME}.hh
  src/${PLUGIN_NAME}.cc
)

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SRCS})

add_library(${PLUGIN_NAME}-filter STATIC ${SRCS})
target_compile_options(${PLUGIN_NAME}-filter PRIVATE ${HTTP_ROUTER_ADDITIONAL_WALL_FLAGS})
target_compile_features(${PLUGIN_NAME}-filter PRIVATE cxx_std_17)
target_include_directories(${PLUGIN_NAME}-filter
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)
target_link_libraries(${PLUGIN_NAME}-filter PUBLIC http-router)
set_target_properties(${PLUGIN_NAME}-filter PROPERTIES
  VERSION ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
  FOLDER ${MODULES_FOLDER}
)

##################################################################
##  INSTALL
##################################################################

if (HTTP_ROUTER_INSTALL)
  install(TARGETS ${PLUGIN_NAME}-filter)
  install(DIRECTORY include/http-router DESTINATION include)
endif()
