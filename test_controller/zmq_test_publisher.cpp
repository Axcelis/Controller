#include "zmq_test_publisher.h"

ZmqTestPublisher::ZmqTestPublisher(const std::string& address)
    : context(1), publisher(context, ZMQ_PUB)
{
    publisher.bind(address);
}

void ZmqTestPublisher::send(const std::string& topic, const std::string& value) {
    zmq::message_t topic_msg(topic.c_str(), topic.size());
    zmq::message_t value_msg(value.c_str(), value.size());
    publisher.send(topic_msg, zmq::send_flags::sndmore);
    publisher.send(value_msg, zmq::send_flags::none);
}
