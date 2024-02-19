//
// Created by bont on 24. 2. 19.
//

#ifndef LIBEVENT_EXERCISE_LIBEVENT_H
#define LIBEVENT_EXERCISE_LIBEVENT_H

#include <memory>

struct event_base;
extern "C" {
void event_base_free(event_base *);
}

template <class T, void (*deleter)(T*)> class CSmartPtr : public std::unique_ptr<T, void (*)(T*)> {
public:
    CSmartPtr() : std::unique_ptr<T, void (*)(T*)>(nullptr, deleter) {}
    CSmartPtr(T* object) : std::unique_ptr<T, void (*)(T*)>(object, deleter) {}
};


class Global {
public:
    static bool initialized() { return initialized_; }

    /**
     * Initialize the library globally.
     */
    static void initialize();

private:
    // True if initialized() has been called.
    static bool initialized_;
};

using BasePtr = CSmartPtr<event_base, event_base_free>;


class Libevent {

};


#endif //LIBEVENT_EXERCISE_LIBEVENT_H
