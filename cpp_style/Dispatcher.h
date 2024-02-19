//
// Created by bont on 24. 2. 19.
//

#ifndef LIBEVENT_EXERCISE_DISPATCHER_H
#define LIBEVENT_EXERCISE_DISPATCHER_H

#include "Libevent.h"
#include "FileEvent.h"

class Dispatcher {
public:

    Dispatcher();
    FileEventPtr createFileEvent(int fd,  const FileReadyCb& cb, FileTriggerType trigger, uint32_t events);

    void dispatch_loop();
    event_base& base() { return *libevent_; }

private:
    BasePtr libevent_;


};


#endif //LIBEVENT_EXERCISE_DISPATCHER_H
