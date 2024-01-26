#include <iostream>
#include <string.h>
#include <errno.h>
#include <stdio.h>
#include <signal.h>
#include <sys/socket.h>

#include <event2/bufferevent.h>
#include <event2/buffer.h>
#include <event2/listener.h>
#include <event2/util.h>
#include <event2/event.h>

// https://libevent.org/libevent-book/

#define EVENT_LOG_DEBUG 0
#define EVENT_LOG_MSG   1
#define EVENT_LOG_WARN  2
#define EVENT_LOG_ERR   3

typedef void (*event_log_cb)(int severity, const char *msg);

void event_set_log_callback(event_log_cb cb);

// 사용자 정의 로깅 콜백 함수
void custom_log_callback(int severity, const char* msg) {
    // severity에 따라 로그 메시지를 다르게 처리할 수 있습니다.
    // 예를 들어, severity가 EVENT_LOG_ERR일 때는 에러 메시지를 출력하고,
    // 그 외의 경우에는 일반 메시지를 출력할 수 있습니다.
    if (severity == EVENT_LOG_DEBUG) {
        std::cerr << "Error: " << msg << std::endl;
    } else {
        std::cout << "Info: " << msg << std::endl;
    }
}



static const char MESSAGE[] = "Hello, World!\n";
static const unsigned short PORT = 9995;

static void
conn_writecb(struct bufferevent *bev, void *user_data)
{
    struct evbuffer *output = bufferevent_get_output(bev);
    if (evbuffer_get_length(output) == 0) {
        printf("flushed answer\n");
        bufferevent_free(bev);
    }
}

static void
conn_eventcb(struct bufferevent *bev, short events, void *user_data)
{
    if (events & BEV_EVENT_EOF) {
        printf("Connection closed.\n");
    } else if (events & BEV_EVENT_ERROR) {
        printf("Got an error on the connection: %s\n",
               strerror(errno));/*XXX win32*/
    }
    /* None of the other events can happen here, since we haven't enabled
     * timeouts */
    bufferevent_free(bev);
}

static void listener_cb(struct evconnlistener *listener, evutil_socket_t fd,
            struct sockaddr *sa, int socklen, void *user_data)
{
    struct event_base *base = static_cast<event_base *>(user_data);
    struct bufferevent *bev;

    bev = bufferevent_socket_new(base, fd, BEV_OPT_CLOSE_ON_FREE);
    if (!bev) {
        fprintf(stderr, "Error constructing bufferevent!");
        event_base_loopbreak(base);
        return;
    }
    bufferevent_setcb(bev, NULL, conn_writecb, conn_eventcb, NULL);
    bufferevent_enable(bev, EV_WRITE);
    bufferevent_disable(bev, EV_READ);

    bufferevent_write(bev, MESSAGE, strlen(MESSAGE));
}



static void
signal_cb(evutil_socket_t sig, short events, void *user_data)
{
    struct event_base *base = static_cast<event_base *>(user_data);
    struct timeval delay = { 2, 0 };

    printf("Caught an interrupt signal; exiting cleanly in two seconds.\n");

    event_base_loopexit(base, &delay);
}

int main() {
    struct event_base *base;  // event_base: event들을 관리하기 위한 구조체
    struct evconnlistener *listener;
    struct event *signal_event; // event: 특정 조건에서 발생되는 사건


    // 로그 설정
    event_set_log_callback(custom_log_callback);

    base = event_base_new();
    if (!base) {
        fprintf(stderr, "Could not initialize libevent!\n");
        return 1;
    }

    struct sockaddr_in sin = {0};
    sin.sin_family = AF_INET;
    sin.sin_port = htons(PORT);

    listener = evconnlistener_new_bind(base, listener_cb, (void *)base,
                                       LEV_OPT_REUSEABLE|LEV_OPT_CLOSE_ON_FREE, -1,
                                       (struct sockaddr*)&sin,
                                       sizeof(sin));

    if (!listener) {
        fprintf(stderr, "Could not create a listener!\n");
        return 1;
    }

    signal_event = evsignal_new(base, SIGINT, signal_cb, (void *)base);

    if (!signal_event || event_add(signal_event, NULL)<0) {
        fprintf(stderr, "Could not create/add a signal event!\n");
        return 1;
    }

    event_base_dispatch(base);

    evconnlistener_free(listener);
    event_free(signal_event);
    event_base_free(base);

    printf("done\n");

    return 0;
}
