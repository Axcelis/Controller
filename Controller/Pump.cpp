#include "Pump.h"

Pump::Pump() : Equipment("Pump"), speed(0), status(PumpStatus::Off) {}
void Pump::setStatus(PumpStatus s) {
    status = s;
}

PumpStatus Pump::getStatus() const {
    return status;
}

void Pump::setSpeed(int rpm) {
    speed = rpm;
}

int Pump::getSpeed() const {
    return speed;
}
