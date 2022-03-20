# Copyright (c) 2022 midnightBITS
# This file is licensed under MIT license (see LICENSE for details)

if (HTTP_ROUTER_TESTING)
  set(COVERALLS_PREFIX HTTP_ROUTER_)
  list(APPEND HTTP_ROUTER_COVERALLS_DIRS
    inc/router
    src
  )

  include(${CMAKE_SOURCE_DIR}/tools/coveralls/Coveralls.cmake)
endif()

configure_file(version.hh.in gen/include/http-router/version.hh @ONLY)

set(SRCS
  path_compiler.cc
  range.cc
  response.cc
  router.cc
  server/beast.cc
  server/beast.hh
  server/http_session.cc
  server/http_session.hh
  server/server.cc
  shared_roots.cc
  version.cc
  version.hh.in
)

set(GEN_SRCS
  "${CMAKE_CURRENT_BINARY_DIR}/gen/include/http-router/version.hh"
)

set(INCS
  ${HTTP_ROUTER_INCLUDE_DIR}/http-router/beast/patched/basic_file_body.hh
  ${HTTP_ROUTER_INCLUDE_DIR}/http-router/beast/patched/file_body_win32.hh
  ${HTTP_ROUTER_INCLUDE_DIR}/http-router/beast/patched/file_body.hh
  ${HTTP_ROUTER_INCLUDE_DIR}/http-router/delegate.hh
  ${HTTP_ROUTER_INCLUDE_DIR}/http-router/filters/base.hh
  ${HTTP_ROUTER_INCLUDE_DIR}/http-router/path_compiler.hh
  ${HTTP_ROUTER_INCLUDE_DIR}/http-router/range.hh
  ${HTTP_ROUTER_INCLUDE_DIR}/http-router/request.hh
  ${HTTP_ROUTER_INCLUDE_DIR}/http-router/response.hh
  ${HTTP_ROUTER_INCLUDE_DIR}/http-router/route.hh
  ${HTTP_ROUTER_INCLUDE_DIR}/http-router/router.hh
  ${HTTP_ROUTER_INCLUDE_DIR}/http-router/server.hh
  ${HTTP_ROUTER_INCLUDE_DIR}/http-router/shared_roots.hh
)


source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} PREFIX code/src FILES ${SRCS})
source_group(TREE ${HTTP_ROUTER_INCLUDE_DIR} PREFIX code/include FILES ${INCS})
source_group(TREE ${CMAKE_CURRENT_BINARY_DIR} FILES ${GEN_SRCS})

add_library(http-router STATIC ${SRCS} ${GEN_SRCS} ${INCS})

target_compile_options(http-router PRIVATE ${HTTP_ROUTER_ADDITIONAL_WALL_FLAGS})
target_compile_features(http-router PRIVATE cxx_std_17)
target_include_directories(http-router
  PUBLIC
    ${HTTP_ROUTER_INCLUDE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}/gen/include
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)
target_link_libraries(http-router PUBLIC fmt::fmt Boost::headers)
set_target_properties(http-router PROPERTIES
  VERSION ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
)

if (NOT HTTP_ROUTER_STANDALONE)
set_target_properties(http-router PROPERTIES
  FOLDER http-router
)
endif()

##################################################################
##  INSTALL
##################################################################

if (HTTP_ROUTER_INSTALL)
  install(TARGETS http-router)
  install(DIRECTORY ${HTTP_ROUTER_INCLUDE_DIR}/http-router DESTINATION include)
  install(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/gen/include/http-router DESTINATION include)
endif()
