
add_executable(libevent_io main.cpp)

target_link_libraries(libevent_io PRIVATE libevent::core)