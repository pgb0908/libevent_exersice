#include <event2/bufferevent.h>
#include <event2/buffer.h>
#include <event2/listener.h>
#include <event2/util.h>
#include <event2/event.h>

void event_callback(evutil_socket_t socket, short dd, void* data){

}


int main(){
    int socket;
    auto base = event_base_new();
    auto eventNew = event_new(base, socket, EV_READ|EV_PERSIST, event_callback,
                              event_self_cbarg());



    return 0;
}