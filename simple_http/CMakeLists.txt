add_executable(simple_http main.cpp)
target_link_libraries(simple_http PRIVATE libevent::core libevent::extra)
#target_include_directories(libevent_exercise PRIVATE ${LIBEVENT_INCLUDE_DIR})