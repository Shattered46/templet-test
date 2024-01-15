#include <chrono>
#include <thread>
#include <memory>
#include <utility>
#include <vector>

#include "protocol/messages.hpp"
#include "src/logger/logger.hpp"
#include "src/socket/socket.hpp"
#include "src/error_parser.hpp"
#include "src/engine/engine.hpp"

using string = std::string;

template <class T1, class T2>
using pair = std::pair<T1, T2>;
template <class T>
using vec = std::vector<T>;
using event = std::pair<Tag, string>;
using InputMessage = message::InputDataMessage;
using EventMessage = message::EventMessage;


#define N 5


pair<InputMessage, InputMessage> handshake(Socket& socket, string name) {
    auto message = message::RegistrationMessage(name);
    message.write(socket);
    socket.flush();

    auto input_message = message::InputDataMessage::read(socket);
    Logger::debug() << "received input data message";
    Logger::debug() << "input name: " << input_message.name;
    Logger::debug() << "vector: ";
    
    print(std::cout, input_message.data);
    Logger::debugln();
    auto input_message2 = message::InputDataMessage::read(socket);
    Logger::debug() << "received input data message";
    Logger::debug() << "input name: " << input_message2.name;
    Logger::debug() << "vector: ";

    return std::make_pair(input_message, input_message2);
}

void generate_tasks(vec<mult_task>& tasks, engine& eng, vec<int>& A, vec<int>& B) {
    for(int i = 0; i < N; i++) {
        mult_task t(A, B);
        t.cur_i = i;
        tasks.push_back(t);
    }

    for(int i = 0; i < N; i++) {
        auto task_p = tasks.data() + i;
        eng.submit(task_p);
    }
}

vec<event> collect_updates(Socket& socket) {
    vec<event> events;
    while (socket.can_read()) {
        message::MessageType msg_type = static_cast<message::MessageType>(message::read_message_type(socket));
        if (msg_type != message::MessageType::Event) {
            Logger::error() << "funny story just happened";
        }
        auto msg = EventMessage::read(socket);
        event e = std::make_pair(Tag { msg.tag }, msg.data);
        events.push_back(e);
    }
    return events;
}

int main() {
    Socket client_socket = Socket();
    bool success = client_socket.connect("127.0.0.1", "11037");

    if (success) {
        auto [inputA, inputB] = handshake(client_socket, "client_name");
        engine engine;
        auto tasks = vec<mult_task>();

        generate_tasks(tasks, engine, inputA.data, inputB.data);
        
        while(true) {
            auto events = collect_updates(client_socket);
            engine.append(events);
            engine.catch_up();
            auto event = engine.execute_random_task();
            if (event.has_value()) {
                auto [tag, data] = *event;
                auto msg = EventMessage(tag.val, data);
                msg.write(client_socket);
            } else {
                break;
            }
        }
        print(std::cout, inputA.data);
        Logger::debugln();
        std::this_thread::sleep_for(std::chrono::milliseconds(10000));
    } else {
        Logger::error("couldn't connect");
        print_error();
    }

    return 0;
}