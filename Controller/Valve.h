#pragma once
#include "Equipment.h"

class Valve : public Equipment {
public:
    Valve();
    void setStatus(ValveStatus status);
    ValveStatus getStatus() const;
private:
    ValveStatus status;
};
