#include "Equipment.h"
#include "Pump.h"
#include "Thermometer.h"
#include "Valve.h"
#include <cassert>
#include <iostream>

void testPump() {
    Pump pump;
    pump.setSpeed(1500);
    assert(pump.getSpeed() == 1500);
    pump.setStatus(PumpStatus::On);
    assert(pump.getStatus() == PumpStatus::On);
    pump.setStatus(PumpStatus::Off);
    assert(pump.getStatus() == PumpStatus::Off);
    pump.enable();
    assert(pump.isEnabled());
    pump.disable();
    assert(!pump.isEnabled());
}

void testThermometer() {
    Thermometer thermometer;
    thermometer.setStatus(PumpStatus::On);
    assert(thermometer.getStatus() == PumpStatus::On);
    thermometer.setStatus(PumpStatus::Off);
    assert(thermometer.getStatus() == PumpStatus::Off);
    thermometer.enable();
    assert(thermometer.isEnabled());
    thermometer.disable();
    assert(!thermometer.isEnabled());
}

void testValve() {
    Valve valve;
    valve.setStatus(ValveStatus::Open);
    assert(valve.getStatus() == ValveStatus::Open);
    valve.setStatus(ValveStatus::Closed);
    assert(valve.getStatus() == ValveStatus::Closed);
    valve.enable();
    assert(valve.isEnabled());
    valve.disable();
    assert(!valve.isEnabled());
}

void testStateMachine() {
    // Simulate state transitions
    Pump pump;
    Thermometer thermometer;
    Valve valve;
    int tempC = 25;
    // Startup
    thermometer.setStatus(PumpStatus::On);
    valve.setStatus(ValveStatus::Open);
    pump.setSpeed(1200);
    pump.setStatus(PumpStatus::On);
    assert(pump.getStatus() == PumpStatus::On);
    assert(thermometer.getStatus() == PumpStatus::On);
    assert(valve.getStatus() == ValveStatus::Open);
    // NormalOperation
    tempC = 101;
    if (tempC > 100) {
        pump.setStatus(PumpStatus::Off);
        assert(pump.getStatus() == PumpStatus::Off);
        std::cout << "Thermometer above 100C! Pump turned off." << std::endl;
    }
    valve.setStatus(ValveStatus::Closed);
    if (valve.getStatus() == ValveStatus::Closed) {
        pump.setStatus(PumpStatus::Off);
        assert(pump.getStatus() == PumpStatus::Off);
        std::cout << "Valve closed! Pump turned off." << std::endl;
    }
}

int main() {
    testPump();
    testThermometer();
    testValve();
    testStateMachine();
    std::cout << "All tests passed!" << std::endl;
    return 0;
}
