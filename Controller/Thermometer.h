#pragma once
#include "Equipment.h"

class Thermometer : public Equipment {
public:
    Thermometer();
    void setStatus(PumpStatus status);
    PumpStatus getStatus() const;
private:
    PumpStatus status;
};
