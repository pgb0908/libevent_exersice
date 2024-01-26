add_executable(pipe_sample main.cpp)
target_link_libraries(pipe_sample PRIVATE libevent::core)
#target_include_directories(libevent_exercise PRIVATE ${LIBEVENT_INCLUDE_DIR})