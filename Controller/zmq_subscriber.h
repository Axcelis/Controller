#pragma once
#include <string>
#include <zmq.hpp>

class ZmqSubscriber {
public:
    ZmqSubscriber(const std::string& address);
    // Receives a message (topic, value) pair
    bool receive(std::string& topic, std::string& value);
private:
    zmq::context_t context;
    zmq::socket_t subscriber;
};
