#include "Thermometer.h"

Thermometer::Thermometer() : Equipment("Thermometer"), status(PumpStatus::Off) {}

void Thermometer::setStatus(PumpStatus s) {
    status = s;
}

PumpStatus Thermometer::getStatus() const {
    return status;
}
