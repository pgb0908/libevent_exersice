add_executable(hello_world main.cpp)
target_link_libraries(hello_world PRIVATE libevent::core)
#target_include_directories(libevent_exercise PRIVATE ${LIBEVENT_INCLUDE_DIR})