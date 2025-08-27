#pragma once
#include <string>
#include <zmq.hpp>

class ZmqTestSubscriber {
public:
    ZmqTestSubscriber(const std::string& address);
    bool receive(std::string& topic, std::string& value);
private:
    zmq::context_t context;
    zmq::socket_t subscriber;
};
