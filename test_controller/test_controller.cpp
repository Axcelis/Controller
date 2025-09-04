
#include "../Controller/Equipment.h"
#include "../Controller/Pump.h"
#include "../Controller/Thermometer.h"
#include "../Controller/Valve.h"

#include <cassert>
#include <iostream>
#include <thread>
#include <chrono>
#include <map>
#include "zmq_test_publisher.h"
#include "zmq_test_subscriber.h"

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


void testZmqMessaging() {
    ZmqTestPublisher publisher("tcp://*:5556");
    ZmqTestSubscriber subscriber("tcp://localhost:5555");

    // Give time for sockets to connect
    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    // Publish test messages
    publisher.send("valve_status", "open");
    publisher.send("pump_status", "on");
    publisher.send("pump_speed", "1500");
    publisher.send("thermometer_status", "on");
    publisher.send("thermometer_temperature", "42");

    // Wait 2 seconds to allow controller to process and publish messages
    std::this_thread::sleep_for(std::chrono::seconds(2));

    // Expected messages from controller
    std::map<std::string, std::string> expected = {
        {"controller_state", "NormalOperation"},
        {"valve_status", "open"},
        {"pump_status", "on"},
        {"pump_speed", "1500"},
        {"thermometer_status", "on"},
        {"thermometer_temperature", "42"}
    };
    std::map<std::string, std::string> received;

    // Keep reading messages until all expected values are received
    int tries = 0;
    while (received.size() < expected.size() && tries < 100) {
        std::string topic, value;
        if (subscriber.receive(topic, value)) 
        {
            std::cout << "Received from controller: " << topic << " = " << value << std::endl;
            if (expected.count(topic) && value == expected[topic]) 
            {
                received[topic] = value;
            }
            ++tries;
        } 
        else 
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
            ++tries;
        }
    }

    // Assert all expected messages were received and match
    for (const auto& kv : expected) {
        assert(received.count(kv.first) && received[kv.first] == kv.second);
    }
}

int main() {
    // Argument parsing
    // Usage: test_controller.exe [unit|zmq]
    // If no argument, run both
    bool ran = false;
    if (__argc > 1) {
        std::string arg = __argv[1];
        if (arg == "unit") {
            testPump();
            testThermometer();
            testValve();
            testStateMachine();
            std::cout << "Unit tests passed!" << std::endl;
            ran = true;
        } else if (arg == "zmq") {
            testZmqMessaging();
            std::cout << "ZMQ messaging test passed!" << std::endl;
            ran = true;
        } else {
            std::cout << "Unknown argument: " << arg << std::endl;
        }
    }
    if (!ran) {
        std::cout << "Running all tests (unit + zmq)..." << std::endl;
        testPump();
        testThermometer();
        testValve();
        testStateMachine();
        testZmqMessaging();
        std::cout << "All tests passed!" << std::endl;
    }
    return 0;
}