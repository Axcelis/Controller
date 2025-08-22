#include "Equipment.h"

Equipment::Equipment(const std::string& name) : name(name), enabled(false) {}
Equipment::~Equipment() {}
void Equipment::enable() {
    enabled = true;
    std::cout << name << " enabled." << std::endl;
}
void Equipment::disable() {
    enabled = false;
    std::cout << name << " disabled." << std::endl;
}
bool Equipment::isEnabled() const {
    return enabled;
}
