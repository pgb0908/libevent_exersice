//
// Created by bont on 24. 2. 19.
//

#ifndef LIBEVENT_EXERCISE_IMPLBASE_H
#define LIBEVENT_EXERCISE_IMPLBASE_H

#include "event2/event_struct.h"

class ImplBase {
protected:
    ~ImplBase();

    event raw_event_;
};


#endif //LIBEVENT_EXERCISE_IMPLBASE_H
