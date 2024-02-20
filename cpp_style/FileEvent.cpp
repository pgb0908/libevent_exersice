//
// Created by bont on 24. 2. 19.
//

#include <event2/event.h>
#include <cassert>
#include <utility>
#include <iostream>
#include <event2/event_compat.h>
#include <arpa/inet.h>
#include <cstring>
#include <unistd.h>
#include "FileEvent.h"
#include "Dispatcher.h"


FileEvent::FileEvent(Dispatcher& dispatcher, int fd,  FileReadyCb cb,  FileTriggerType trigger, uint32_t events)
    : dispatcher_(dispatcher), fd_(fd), cb_(std::move(cb)), enabled_events_(events), trigger_(trigger){

    assert(&dispatcher.base() != nullptr);

    assignEvents(events, &dispatcher.base());
    event_add(&raw_event_, nullptr);

}

void FileEvent::assignEvents(uint32_t events, event_base *base) {
    assert(base != nullptr);

    enabled_events_ = events;
    event_assign(
            &raw_event_, base, fd_,
            EV_PERSIST | (trigger_ == FileTriggerType::Edge ? EV_ET : 0) |
            (events & FileReadyType::Read ? EV_READ : 0) |
            (events & FileReadyType::Write ? EV_WRITE : 0) |
            (events & FileReadyType::Closed ? EV_CLOSED : 0),
            [](evutil_socket_t, short what, void* arg) -> void {
                auto* event = static_cast<FileEvent*>(arg);
                uint32_t events = 0;
                if (what & EV_READ) {
                    events |= FileReadyType::Read;
                }

                if (what & EV_WRITE) {
                    events |= FileReadyType::Write;
                }

                if (what & EV_CLOSED) {
                    events |= FileReadyType::Closed;
                }

                assert(events != 0);
                event->mergeInjectedEventsAndRunCb(events);
            },
            this);
}

void FileEvent::setEnabled(uint32_t events) {
    updateEvents(events);
}

void FileEvent::updateEvents(uint32_t events) {
    if (events == enabled_events_ && trigger_ != FileTriggerType::Edge) {
        return;
    }
    auto* base = event_get_base(&raw_event_);
    event_del(&raw_event_);
    assignEvents(events, base);
    event_add(&raw_event_, nullptr);
}

void FileEvent::mergeInjectedEventsAndRunCb(uint32_t events) {
    // TODO(davinci26): This can be optimized further in (w)epoll backends using the `EPOLLONESHOT`
    // flag. With this flag `EPOLLIN`/`EPOLLOUT` are automatically disabled when the event is
    // activated.
/*    if constexpr (PlatformDefaultTriggerType == FileTriggerType::EmulatedEdge) {
        if (trigger_ == FileTriggerType::EmulatedEdge) {
            unregisterEventIfEmulatedEdge(events &
                                          (Event::FileReadyType::Write | Event::FileReadyType::Read));
        }
    }*/

    cb_(events);
}

void FileEvent::cancelEvent() {
    event_del(&raw_event_);
}


