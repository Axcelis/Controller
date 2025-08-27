#include "zmq_subscriber.h"

ZmqSubscriber::ZmqSubscriber(const std::string& address)
    : context(1), subscriber(context, ZMQ_SUB)
{
    subscriber.connect(address);
    subscriber.setsockopt(ZMQ_SUBSCRIBE, "", 0); // Subscribe to all topics
}

bool ZmqSubscriber::receive(std::string& topic, std::string& value) {
    zmq::message_t topic_msg;
    zmq::message_t value_msg;
    // Use DONTWAIT flag for non-blocking receive
    if (!subscriber.recv(topic_msg, zmq::recv_flags::dontwait)) return false;
    if (!subscriber.recv(value_msg, zmq::recv_flags::dontwait)) return false;
    topic = std::string(static_cast<char*>(topic_msg.data()), topic_msg.size());
    value = std::string(static_cast<char*>(value_msg.data()), value_msg.size());
    return true;
}
