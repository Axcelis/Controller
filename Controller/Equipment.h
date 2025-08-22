#pragma once
#include <string>
#include <iostream>

enum class ValveStatus {
    Open,
    Closed
};

enum class PumpStatus {
    On,
    Off
};

class Equipment {
public:
    Equipment(const std::string& name);
    virtual ~Equipment();
    virtual void enable();
    virtual void disable();
    bool isEnabled() const;
protected:
    std::string name;
    bool enabled;
};
