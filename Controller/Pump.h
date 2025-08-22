#pragma once
#include "Equipment.h"

class Pump : public Equipment {
public:
    Pump();
    void setSpeed(int rpm);
    int getSpeed() const;
    void setStatus(PumpStatus status);
    PumpStatus getStatus() const;
private:
    int speed; // Current pump speed in RPM
    PumpStatus status;
};
