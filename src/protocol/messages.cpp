#include "messages.hpp"

#include <netinet/in.h>
#include <string>
#include <iostream>
#include <array>
#include <unistd.h>
#include <vector>
#include <bitset>
#include "src/logger/logger.hpp"

namespace message {

    int read_message_type(Socket& stream) {
        return read(stream);
    }

    SerializableMatrix::SerializableMatrix(vector<int> data_in): data(data_in) {

    }

    void SerializableMatrix::read(Socket& in) {
        
        int vec_len = message::read(in);
        auto vec = message::read_vector(in, vec_len);
    };
    void SerializableMatrix::write(Socket& out) {
        int vec_len = this->data.size();
        put(out, vec_len);
        put(out, this->data);
    };


    RegistrationMessage::RegistrationMessage() { };

    RegistrationMessage::RegistrationMessage(string name) : name(name) { }

    RegistrationMessage RegistrationMessage::create(string name) {
        return RegistrationMessage(name);
    }
    
    void RegistrationMessage::write(Socket& stream) {
        put(stream, MessageType::Registration);
        put(stream, this->name.length());
        put(stream, this->name);
    }

    RegistrationMessagePOCO RegistrationMessage::read_poco(Socket& stream) {
        int name_length = message::read(stream);
        if (name_length <= 0) {
            Logger::error() << "tried to read registration message from stream, but name length <= 0. Expected length > 0";
            throw -3;
        }
        auto name = read_vector(stream, name_length);
        return RegistrationMessagePOCO { MessageType::Registration, name_length, name };
    }

    RegistrationMessage RegistrationMessage::read(Socket& stream) {
        auto poco = read_poco(stream);
        std::string name_string = std::string(poco.name.begin(), poco.name.end());
        return RegistrationMessage(name_string);
    }

    void RegistrationMessage::read_into(Socket& stream) {
        auto poco = read_poco(stream);
        this->name = std::string(poco.name.data());
    }

    void put(Socket& stream, string str) {
        put(stream, str.data(), str.length());
    }

    void put(Socket& stream, vector<char> vector) {
        put(stream, vector.data(), vector.size());
    }

    void put(Socket& stream, vector<int> vector) {
        put(stream, vector.data(), vector.size());
    }

    void put(Socket& socket, const char* value, int len) {
        auto& debug = Logger::debug() << "write ";
        for(int i = 0; i < len; i++) {
            int val = value[i];
            debug << std::to_string(val) << " ";
        };
        Logger::debugln();

        socket.send(value, len);
    }

    void put(Socket& stream, const int* value, int len) {
        auto& debug = Logger::debug() << "write ";
        for(int i = 0; i < len; i++) {
            int val = value[i];
            debug << std::to_string(val) << " ";
            put(stream, value[i]);
        };
        Logger::debugln();
    }

    void put(Socket& stream, int n) {
        int network_ordered = htonl(n);
        // "JOHN"
        char byte[4];
        byte[0] = (network_ordered & 0x000000ff); // N
        byte[1] = (network_ordered & 0x0000ff00) >> 8; // H
        byte[2] = (network_ordered & 0x00ff0000) >> 16; // O
        byte[3] = (network_ordered & 0xff000000) >> 24; // J

        put(stream, byte, 4);
    }



    std::string read_string(Socket& stream, int length) {
        auto vec = std::vector<char>(length);
        char* buffer = vec.data();
        read(stream, buffer, length);
        
        return std::string(vec.data());
    }

    std::vector<int> read_int_vector(Socket& stream, int length) {
        auto vec = std::vector<int>(length);
        int* buffer = vec.data();
        read(stream, buffer, length);
        return vec;
    }

    std::vector<char> read_vector(Socket& stream, int length) {
        auto vec = std::vector<char>(length);
        char* buffer = vec.data();
        read(stream, buffer, length);
        auto& debug = Logger::debug() << "byte ";
        for(int i = 0; i < length; i++) {
            int x = buffer[i];
            debug << std::to_string(x) << ", ";
        }
        return vec;
    }

    int read(Socket& socket) {
        int result = 0;
        auto& log = Logger::debug() << "byte ";

        char bytes[4];
        for(int i = 0; i < 4; i++) {
            socket.recv(bytes + i, 1);
            log << std::to_string(bytes[i]) << " ";
        }

        //network ordered bytes;

        result = result | ((int) bytes[0] & 0x000000ff);
        result = result | (((int) bytes[1] << 8) & 0x0000ff00);
        result = result | (((int) bytes[2] << 16) & 0x00ff0000);
        result = result | (((int) bytes[3] << 24) & 0xff000000);

        
        int host_ordered = ntohl(result);
        log << std::to_string(host_ordered);

        std::cout << " " << std::bitset< 32 >( result ) << ' ';
        return host_ordered;
    }

    void read(Socket& stream, int* buffer, int n) {
        for(int i = 0; i < n; i++) {
            buffer[i] = message::read(stream);
        }
    }

    void read(Socket& socket, char* buffer, int n) {
        //TODO: less 'recv' syscalls
        for(int i = 0; i < n; i++) {
            socket.recv(buffer + i, 1);
        }
    }

    InputDataMessage::InputDataMessage() {

    }
    InputDataMessage::InputDataMessage(string name, vector<int> data) :
        name(name),
        data(data)
    {

    }

    InputDataMessagePOCO InputDataMessage::read_poco(Socket& stream) {
        MessageType message_type = static_cast<MessageType>(message::read(stream));
        if (message_type != MessageType::InputData) {
            Logger::log() << "tried to read registration message from stream, but message type is wrong. Expected 'InputData', found " + std::to_string(message_type);
            throw -2;
        }
        int name_length = message::read(stream);
        if (name_length <= 0) {
            Logger::log() << "tried to read registration message from stream, but name length <= 0. Expected length > 0";
            throw -3;
        }

        std::cout << "value length" << name_length << std::endl;
        auto name = read_vector(stream, name_length);
        int data_length = message::read(stream);
        auto data = read_int_vector(stream, data_length);
        return InputDataMessagePOCO { message_type, name_length, name, data_length, data };
    }

    void InputDataMessage::read_into(Socket& stream) {
        auto poco = read_poco(stream);
        this->name = std::string(poco.name.begin(), poco.name.end());
        this->data = poco.data;
    }

    void InputDataMessage::write(Socket& stream) {
        put(stream, MessageType::InputData);
        put(stream, this->name.length());
        put(stream, this->name);
        put(stream, this->data.size());
        put(stream, this->data);
    }

    InputDataMessage InputDataMessage::read(Socket& stream) {
        auto poco = read_poco(stream);
        std::string name_string = std::string(poco.name.begin(), poco.name.end());
        return InputDataMessage(name_string, poco.data);
    }

    

    EventMessage::EventMessage() { }
    EventMessage::EventMessage(int tag, string data): tag(tag), data(data) { }
    EventMessage EventMessage::read(Socket& socket) {
        Logger::log() << "reading event message"; Logger::logln();
        auto poco = read_poco(socket);
        int tag = poco.id;
        string data_string = string(poco.value.begin(), poco.value.end());
        return EventMessage(tag, data_string);
    }

    EventPOCO EventMessage::read_poco(Socket& socket) {
        int id = message::read(socket);
        int value_length = message::read(socket);
        auto value = read_vector(socket, value_length);
        return EventPOCO { MessageType::Event, id, value_length, value };
    }

    void EventMessage::read_into(Socket& socket) {
        auto poco = read_poco(socket);
        string data = string(poco.value.begin(), poco.value.end());
        this->tag = poco.id;
        this->data = data;
    };
    
    void EventMessage::write(Socket& socket) {
        put(socket, MessageType::Event);
        put(socket, this->tag);
        put(socket, this->data.length());
        put(socket, this->data);
    };
} // namespace message
