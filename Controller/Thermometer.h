#pragma once
#include "Equipment.h"

class Thermometer : public Equipment {
public:
    Thermometer();
    void setStatus(PumpStatus status);
    PumpStatus getStatus() const;

    // Temperature member and accessors
    void setTemperature(int temp) { temperature = temp; }
    int getTemperature() const { return temperature; }

private:
    PumpStatus status;
    int temperature = 0;
};
