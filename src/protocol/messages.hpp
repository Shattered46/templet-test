#pragma once

#include <istream>
#include <vector>
#include <string>
#include <iostream>
#include "src/socket/socket.hpp"

namespace message {
    template<class T>
    using vector = std::vector<T>;
    using string = std::string;

    int read_message_type(Socket& istream);

    enum MessageType {
        Registration,
        Event,
        InputData
    };

    struct RegistrationMessagePOCO {
        MessageType messageType;
        int name_length;
        std::vector<char> name;
    };

    struct InputDataMessagePOCO {
        MessageType messageType;
        int name_length;
        std::vector<char> name;
        int data_length;
        std::vector<int> data;
    };

    class Message {
    public:
        virtual void read_into(Socket&) = 0;
        virtual void write(Socket&) = 0;
        virtual ~Message() { }
    };

    class RegistrationMessage: Message {
    private:     
    public: 
        std::string name;

        RegistrationMessage();
        RegistrationMessage(std::string name);
        static RegistrationMessage create(std::string name);
        static RegistrationMessage read(Socket&);
        static RegistrationMessagePOCO read_poco(Socket&);
        void read_into(Socket&) override;
        void write(Socket&) override;
    };

    class SerializableMatrix {
    public:
        SerializableMatrix(std::vector<int>);
        std::vector<int> data;
        static void read(Socket&);
        void write(Socket&);
    };

    class InputDataMessage: Message {
    public:
        std::string name;
        std::vector<int> data;
        InputDataMessage();
        InputDataMessage(std::string name, std::vector<int> data);
        static InputDataMessage read(Socket&);
        static InputDataMessagePOCO read_poco(Socket&);
        void read_into(Socket&) override;
        void write(Socket&) override;
    };

    struct EventPOCO {
        MessageType messageType;
        int id;
        int value_length;
        std::vector<char> value;
    };

    class EventMessage: public Message {
    public:
        int tag;
        std::string data;
        EventMessage();
        EventMessage(int tag, std::string data);
        static EventMessage read(Socket&);
        static EventPOCO read_poco(Socket&);
        void read_into(Socket&) override;
        void write(Socket&) override;
    };


    void put(Socket&, std::vector<int>);
    void put(Socket&, const int* value, int len);
    void put(Socket&, const char* value, int len);
    void put(Socket&, int);
    void put(Socket&, std::string);


    void read(Socket& stream, int* buffer, int n);
    void read(Socket& stream, char* buffer, int n);
    int read(Socket& stream);
    std::vector<int> read_int_vector(Socket&, int len);
    std::vector<char> read_vector(Socket&, int len);
    std::string read_string(Socket&, int len);
}