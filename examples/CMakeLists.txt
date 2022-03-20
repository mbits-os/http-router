if (HTTP_ROUTER_EXAMPLES)

if (HTTP_ROUTER_STANDALONE)
set(FOLDER examples)
else()
set(FOLDER http-router/examples)
endif()

add_executable(simple-server simple-server.cc)
target_link_libraries(simple-server PRIVATE http-router http-router-filters)

set_target_properties(simple-server PROPERTIES
  FOLDER ${FOLDER}
)

endif()
