//
// Created by bont on 24. 2. 19.
//

#include "Libevent.h"


bool Global::initialized_ = false;

void Global::initialize() {

    initialized_ = true;
}