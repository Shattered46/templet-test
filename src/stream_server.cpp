#include <algorithm>
#include <chrono>
#include <memory>
#include <ostream>
#include <thread>
#include <string>
#include <random>

#include "src/../libs/httplib.h"
#include "src/../libs/json.hpp"

#include "src/engine/engine.hpp"
#include "src/state/state.hpp"
#include "protocol/messages.hpp"
#include "socket/socket_pool.hpp"
#include "src/event_log/event_log.hpp"
#include "src/socket/socket.hpp"
#include "src/logger/logger.hpp"

using namespace pool;

using string = std::string;
template <typename T>
using rc = std::shared_ptr<T>;
using EventMessage = message::EventMessage;

static std::random_device rd;
static std::mt19937 gen(rd());
static std::uniform_int_distribution<> distrib(-255, 255);

template<typename T>
void fill_random(std::vector<T>& vector) {
    for(auto it = vector.begin(); it != vector.end(); ++it) {
        *it = distrib(gen);
    }
}

template<typename T>
void print(std::ostream& stream, std::vector<T>& vector) {
    for(auto it = vector.begin(); it != vector.end(); ++it) {
        stream << std::to_string(*it) << " ";
    };
}

void listener_ready(Socket* socket, SocketPool& pool) {
    Logger::log("new client connected");
    Socket* new_client = socket->accept();
    if (new_client == nullptr) {
        Logger::error("accepted client is null");
    } else {
        pool.add_socket(new_client);
    }
}

void socket_ready(Socket* _client, SocketPool& pool) {
    if (_client == nullptr) {
        Logger::error() << "Reported that socket is ready, but received null pointer";
        return;
    }

    Socket& client = *_client;
    Logger::log() << "serving client #" << client.socket_descriptor();
    
    try {
        int message_type = message::read_message_type(client);

        switch (message_type) {
        case message::MessageType::Registration: {
            auto message = message::RegistrationMessage::read(client);
            auto client_name = string(message.name.begin(), message.name.end());

            Logger::debug() << "message length " << message.name.length();
            Logger::log() << "client with name "+ client_name + " registered";  

            Logger::debug() << "sending initial matrix A";
            auto data_message_A = message::InputDataMessage("A", pool.state->A);
            data_message_A.write(client);
            client.flush();
            auto data_message_B = message::InputDataMessage("B", pool.state->B);
            data_message_B.write(client);
            break;
        }
        case message::MessageType::Event: {
            auto message = EventMessage::read(client);
            auto [tag, data] = message;
            auto state = pool.state;
            
            state->log_mutex.lock();
            state->log.add_event(Tag { tag }, data);
            state->log_mutex.unlock();

            pool.broadcast(message);
            // broadcast
            break;
        }
        default:
            Logger::error() << "unknown message type " << message_type;
            Logger::errorln();
            throw -1;
            break;
        }
    } catch (int error) {
        Logger::error() << "client message reading failed";
        pool.remove(_client);
    }
    

}

void socket_server_thread(State state);

void http_server_thread(State state);

int main() {
    SharedState shared_state(5);
    fill_random(shared_state.A);
    fill_random(shared_state.B);

    std::shared_ptr<SharedState> state (&shared_state);

    std::thread socket_thread(socket_server_thread, state);
    std::thread http_thread(http_server_thread, state);

    socket_thread.join();
    http_thread.join();
    return 0;
}

void socket_server_thread(State state) {
    Socket server_socket = Socket();
    Logger::log() << "started socket";

    bool success = server_socket.bind("127.0.0.1", "11037");
     
    if (success) {
        SocketPool pool(10, state);
        pool.add_socket(&server_socket);
        pool.on_listener_ready(listener_ready)
            ->on_socket_ready(socket_ready);

        Logger::log() << "accepting clients";
        Logger::logln();
        while (true) {
            pool.poll(15000);
        }
    } else {
        Logger::error() << "failed to bind socket";
    }
}

void http_server_thread(State state) {
    using namespace httplib;
    using namespace nlohmann;

    ::Logger::log() << "http server thread started"; ::Logger::logln();
    httplib::Server server;

    server.Get("/hi", [](const Request& req, Response& res) {
        auto header = req.response_handler;
        res.set_content("hello", "text/plain");
    });


    server.Get("/events", [state](const Request& req, Response& res) {
        auto header = req.response_handler;

        json response = json::array();

        state->log_mutex.lock();
        uint size = state->log.size();
        for(uint i = 0; i < size; i++) {
            auto event = state->log.get_event(Ord { i });
            auto [tag, data] = event.value();

            json element = { {"tag", tag.val }, {"data", data} };
            response.push_back(element);
        }
        state->log_mutex.unlock();

        string response_text = response.dump();
        res.set_content(response_text, "application/json");
    });

    server.Delete("/event/:id", [state](const Request& req, Response& res) {
        string user_id = req.path_params.at("id");
        int event_id = std::stoi(user_id);
        
        state->log_mutex.lock();
        state->log.remove_event(Tag { event_id });
        
        state->log_mutex.unlock();

        string response_text = "OK";
        res.set_content(response_text, "text/plain");
    });

    server.Get("/state", [state](const Request& req, Response& res) {
        auto _ = req.response_handler;

        state->log_mutex.lock();
        std::vector<int>& A = state->A;
        std::vector<int>& B = state->B;
        std::vector<int>& C = state->C;
        

        mult_task task(A, B);

        uint size = state->log.size();
        for(uint i = 0; i < size; i++) {
            auto event = state->log.get_event(Ord { i });

            auto [tag, data] = event.value();

            std::istringstream stre(data);
            task.on_load(stre);

            std::copy(task.c, task.c + N, C.begin() + tag.val * N);
        }
        
        json json = { { "A", A}, {"B", B}, {"C", C}};
        string response = json.dump();
        state->log_mutex.unlock();

        res.set_content(response, "application/json");
    });

    server.set_mount_point("/", "../static/");

    server.listen("0.0.0.0", 11038);
}
