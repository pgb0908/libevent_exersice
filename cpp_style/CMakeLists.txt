
set(SRC_LIBEVENT
    ./Dispatcher.cpp
    ./Libevent.cpp
        ./FileEvent.cpp
        ./ImplBase.cpp
)

add_executable(cpp_libevent main.cpp ${SRC_LIBEVENT})

target_link_libraries(cpp_libevent PRIVATE libevent::core)