#include <iostream>
#include "Equipment.h"
#include "Pump.h"
#include "Thermometer.h"
#include "Valve.h"

// Controller class
enum class ControllerState {
    Startup,
    NormalOperation,
    ThermometerHigh,
    ValveClosed,
    Done
};

class Controller {
public:
    Controller() : pump(), thermometer(), valve() {}
    void enableAll() {
        pump.enable();
        thermometer.enable();
        valve.enable();
        pump.setStatus(PumpStatus::On);
        thermometer.setStatus(PumpStatus::On);
        valve.setStatus(ValveStatus::Open);
    }
    void disableAll() {
        pump.disable();
        thermometer.disable();
        valve.disable();
        pump.setStatus(PumpStatus::Off);
        thermometer.setStatus(PumpStatus::Off);
        valve.setStatus(ValveStatus::Closed);
    }
    void status() {
        std::cout << "Pump: " << (pump.isEnabled() ? "Enabled" : "Disabled") << ", Status: " << (pump.getStatus() == PumpStatus::On ? "On" : "Off") << ", Speed: " << pump.getSpeed() << " RPM" << std::endl;
        std::cout << "Thermometer: " << (thermometer.isEnabled() ? "Enabled" : "Disabled") << ", Status: " << (thermometer.getStatus() == PumpStatus::On ? "On" : "Off") << std::endl;
        std::cout << "Valve: " << (valve.isEnabled() ? "Enabled" : "Disabled") << ", Status: " << (valve.getStatus() == ValveStatus::Open ? "Open" : "Closed") << std::endl;
    }

    void runStateMachine() {
        ControllerState state = ControllerState::Startup;
        bool running = true;
        int tempC = 25;

        while (running) {
            switch (state) {
                case ControllerState::Startup:
                    std::cout << "State: Startup" << std::endl;
                    thermometer.setStatus(PumpStatus::On);
                    valve.setStatus(ValveStatus::Open);
                    pump.setSpeed(1200);
                    status();
                    state = ControllerState::NormalOperation;
                    break;
                case ControllerState::NormalOperation:
                    std::cout << "State: NormalOperation" << std::endl;
                    // Simulate temperature rising
                    tempC = 101;
                    if (tempC > 100) {
                        state = ControllerState::ThermometerHigh;
                    } else if (valve.getStatus() == ValveStatus::Closed) {
                        state = ControllerState::ValveClosed;
                    } else {
                        state = ControllerState::Done;
                    }
                    break;
                case ControllerState::ThermometerHigh:
                    std::cout << "State: ThermometerHigh" << std::endl;
                    pump.setStatus(PumpStatus::Off);
                    pump.disable();
                    std::cout << "ERROR: Thermometer above 100C! Pump turned off." << std::endl;
                    status();
                    state = ControllerState::Done;
                    break;
                case ControllerState::ValveClosed:
                    std::cout << "State: ValveClosed" << std::endl;
                    pump.setStatus(PumpStatus::Off);
                    pump.disable();
                    std::cout << "ERROR: Valve closed! Pump turned off." << std::endl;
                    status();
                    state = ControllerState::Done;
                    break;
                case ControllerState::Done:
                    running = false;
                    break;
            }
        }
    }
private:
    Pump pump;
    Thermometer thermometer;
    Valve valve;
};

int main() {
    std::cout << "Hello, C++ Boilerplate!" << std::endl;
    Controller controller;
    controller.enableAll();
    controller.runStateMachine();
    controller.disableAll();
    controller.status();
    return 0;
}
