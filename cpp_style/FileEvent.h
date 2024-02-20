//
// Created by bont on 24. 2. 19.
//

#ifndef LIBEVENT_EXERCISE_FILEEVENT_H
#define LIBEVENT_EXERCISE_FILEEVENT_H


#include "ImplBase.h"

#include <memory>
#include <functional>

struct FileReadyType {
    // File is ready for reading.
    static constexpr uint32_t Read = 0x1;
    // File is ready for writing.
    static constexpr uint32_t Write = 0x2;
    // File has been remote closed.
    static constexpr uint32_t Closed = 0x4;
};


enum class FileTriggerType {
    // See @man 7 epoll(7)
    // They are used on all platforms for DNS and TCP listeners.
    Level,
    // See @man 7 epoll(7)
    // They are used on all platforms that support Edge triggering as the default trigger type.
    Edge,
    // These are synthetic edge events managed by Envoy. They are based on level events and when they
    // are activated they are immediately disabled. This makes them behave like Edge events. Then it
    // is is the responsibility of the consumer of the event to reactivate the event
    // when the socket operation would block.
    //
    // Their main application in Envoy is for Win32 which does not support edge-triggered events. They
    // should be used in Win32 instead of level events. They can only be used in platforms where
    // `PlatformDefaultTriggerType` is `FileTriggerType::EmulatedEdge`.
    EmulatedEdge
};

class Dispatcher;

using FileReadyCb = std::function<void(uint32_t events)>;

class FileEvent : public ImplBase {
public:
    FileEvent(Dispatcher& dispatcher, int fd,  FileReadyCb cb, FileTriggerType trigger, uint32_t events);

    void setEnabled(uint32_t events);
    void cancelEvent();

private:
    void assignEvents(uint32_t events, event_base* base);

    Dispatcher& dispatcher_;
    FileReadyCb cb_;
    int fd_;
    FileTriggerType trigger_;
    // Enabled events for this fd.
    uint32_t enabled_events_;

    struct event ev_accept_;

    void updateEvents(uint32_t events);

    void mergeInjectedEventsAndRunCb(uint32_t i);
};

using FileEventPtr = std::unique_ptr<FileEvent>;


#endif //LIBEVENT_EXERCISE_FILEEVENT_H
