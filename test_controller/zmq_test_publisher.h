#pragma once
#include <string>
#include <zmq.hpp>

class ZmqTestPublisher {
public:
    ZmqTestPublisher(const std::string& address);
    void send(const std::string& topic, const std::string& value);
private:
    zmq::context_t context;
    zmq::socket_t publisher;
};
