#include <iostream>
#include <thread>
#include <chrono>
#include "Equipment.h"
#include "Pump.h"
#include "Thermometer.h"
#include "Valve.h"
#include "zmq_publisher.h"
#include "zmq_subscriber.h"

// Controller class
enum class ControllerState {
    Startup,
    NormalOperation,
    ThermometerHigh,
    ValveClosed,
    Done,
    Idle
};

class Controller {
public:
    Controller(ZmqPublisher& publisher)
        : pump(), thermometer(), valve(), publisher(publisher) {}

    // Message handler to update components
    void handleMessage(const std::string& key, const std::string& value) {
        if (key == "valve_status") {
            if (value == "open") valve.setStatus(ValveStatus::Open);
            else if (value == "closed") valve.setStatus(ValveStatus::Closed);
        } else if (key == "pump_status") {
            if (value == "on") pump.setStatus(PumpStatus::On);
            else if (value == "off") pump.setStatus(PumpStatus::Off);
        } else if (key == "pump_speed") {
            try {
                int speed = std::stoi(value);
                pump.setSpeed(speed);
            } catch (...) {}
        } else if (key == "thermometer_status") {
            if (value == "on") thermometer.setStatus(PumpStatus::On);
            else if (value == "off") thermometer.setStatus(PumpStatus::Off);
        } else if (key == "thermometer_temperature") {
            try {
                int temp = std::stoi(value);
                thermometer.setTemperature(temp);
            } catch (...) {}
        }
    }
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
    void status(ControllerState state) {
        std::cout << "Pump: " << (pump.isEnabled() ? "Enabled" : "Disabled") << ", Status: " << (pump.getStatus() == PumpStatus::On ? "On" : "Off") << ", Speed: " << pump.getSpeed() << " RPM" << std::endl;
        std::cout << "Thermometer: " << (thermometer.isEnabled() ? "Enabled" : "Disabled") << ", Status: " << (thermometer.getStatus() == PumpStatus::On ? "On" : "Off") << std::endl;
        std::cout << "Valve: " << (valve.isEnabled() ? "Enabled" : "Disabled") << ", Status: " << (valve.getStatus() == ValveStatus::Open ? "Open" : "Closed") << std::endl;

        // Send updates via ZeroMQ
        publisher.send("pump_speed", std::to_string(pump.getSpeed()));
        publisher.send("temperature", std::to_string(thermometer.getStatus() == PumpStatus::On ? 25 : 101)); // Example temperature
        publisher.send("valve_position", valve.getStatus() == ValveStatus::Open ? "open" : "closed");
        std::string state_str;
        switch (state) {
            case ControllerState::Startup: state_str = "Startup"; break;
            case ControllerState::NormalOperation: state_str = "NormalOperation"; break;
            case ControllerState::ThermometerHigh: state_str = "ThermometerHigh"; break;
            case ControllerState::ValveClosed: state_str = "ValveClosed"; break;
            case ControllerState::Done: state_str = "Done"; break;
        }
        publisher.send("controller_state", state_str);
    }

    void runStateMachine() {
        ControllerState state = ControllerState::Startup;
        bool running = true;
        int tempC = 25;

        ZmqSubscriber subscriber("tcp://localhost:5556"); // Example address
        while (running) {
            // Receive and process all available messages before each state execution
            for (int msgCount = 0; msgCount < 10; ++msgCount) { // Example: process up to 10 messages per cycle
                std::string topic, value;
                if (subscriber.receive(topic, value)) {
                    std::cout << "Received message: " << topic << " = " << value << std::endl;
                    handleMessage(topic, value);
                } else {
                    break; // No more messages available
                }
            }
            switch (state) {
                case ControllerState::Startup:
                    std::cout << "State: Startup" << std::endl;
                    thermometer.setStatus(PumpStatus::On);
                    thermometer.setTemperature(99);
                    valve.setStatus(ValveStatus::Open);
                    pump.setSpeed(1200);
                    state = ControllerState::NormalOperation;
                    break;
                case ControllerState::NormalOperation:
                    std::cout << "State: NormalOperation" << std::endl;
                    std::cout << "Valve status: " << (valve.getStatus() == ValveStatus::Open ? "Open" : "Closed") << std::endl;
                    std::cout << "Pump status: " << (pump.getStatus() == PumpStatus::On ? "On" : "Off") << std::endl;
                    std::cout << "Thermometer status: " << (thermometer.getStatus() == PumpStatus::On ? "On" : "Off") << std::endl;
                    std::cout << "Thermometer temperature: " << thermometer.getTemperature() << std::endl;
                    std::cout << "Pump speed: " << pump.getSpeed() << std::endl;
                    if (thermometer.getTemperature() > 100) {
                        state = ControllerState::ThermometerHigh;
                    } else if (valve.getStatus() == ValveStatus::Closed) {
                        state = ControllerState::ValveClosed;
                    } else {
                        std::this_thread::sleep_for(std::chrono::seconds(1));
                    }
                    break;
                case ControllerState::ThermometerHigh:
                    std::cout << "State: ThermometerHigh" << std::endl;
                    pump.setStatus(PumpStatus::Off);
                    pump.disable();
                    std::cout << "ERROR: Thermometer above 100C! Pump turned off." << std::endl;
                    state = ControllerState::Done;
                    break;
                case ControllerState::ValveClosed:
                    std::cout << "State: ValveClosed" << std::endl;
                    pump.setStatus(PumpStatus::Off);
                    pump.disable();
                    std::cout << "ERROR: Valve closed! Pump turned off." << std::endl;
                    state = ControllerState::Done;
                    break;
                case ControllerState::Done:
                    std::cout << "State: Done" << std::endl;
                    state = ControllerState::NormalOperation;
                    break;
            }
            // Send status messages for GUI updates at the end of each loop
            std::string state_str;
            switch (state) {
                case ControllerState::Startup: state_str = "Startup"; break;
                case ControllerState::NormalOperation: state_str = "NormalOperation"; break;
                case ControllerState::ThermometerHigh: state_str = "ThermometerHigh"; break;
                case ControllerState::ValveClosed: state_str = "ValveClosed"; break;
                case ControllerState::Done: state_str = "Done"; break;
                case ControllerState::Idle: state_str = "Idle"; break;
            }
            //publisher.send("controller_state", state_str);
            publisher.send("valve_status", valve.getStatus() == ValveStatus::Open ? "open" : "closed");
            publisher.send("pump_status", pump.getStatus() == PumpStatus::On ? "on" : "off");
            publisher.send("pump_speed", std::to_string(pump.getSpeed()));
            publisher.send("thermometer_status", thermometer.getStatus() == PumpStatus::On ? "on" : "off");
            publisher.send("thermometer_temperature", std::to_string(thermometer.getTemperature()));
        }
    }
private:
    Pump pump;
    Thermometer thermometer;
    Valve valve;
    ZmqPublisher& publisher;
};

int main() {
    std::cout << "Hello, C++ Boilerplate!" << std::endl;
    ZmqPublisher publisher("tcp://*:5555");
    Controller controller(publisher);
    controller.enableAll();
    controller.runStateMachine();
    controller.disableAll();
    controller.status(ControllerState::Done);
    return 0;
}
