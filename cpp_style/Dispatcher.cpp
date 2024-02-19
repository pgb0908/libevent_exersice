//
// Created by bont on 24. 2. 19.
//

#include <event2/event.h>
#include <iostream>
#include "Dispatcher.h"

Dispatcher::Dispatcher() {
    event_base* event_base = event_base_new();
    libevent_ = BasePtr(event_base);
}


void Dispatcher::dispatch_loop() {
    std::cout  << "event loop~~~" << std::endl;
    event_base_dispatch(libevent_.get());

}

FileEventPtr Dispatcher::createFileEvent(int fd, const FileReadyCb& cb, FileTriggerType trigger, uint32_t events) {
    auto fileEvent = std::make_unique<FileEvent>(*this, fd,
                                                 [this, cb](uint32_t events) {
                                                     cb(events);
                                                 },
                                                 trigger,
                                                 events);

    event_base_dump_events(libevent_.get(), stdout);

    return fileEvent;
}
