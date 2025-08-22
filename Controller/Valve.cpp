#include "Valve.h"

Valve::Valve() : Equipment("Valve"), status(ValveStatus::Closed) {}

void Valve::setStatus(ValveStatus s) {
    status = s;
}

ValveStatus Valve::getStatus() const {
    return status;
}
