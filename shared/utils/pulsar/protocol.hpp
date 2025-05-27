#pragma once

#include <Wire.h>

#define PROTOCOL_VER 1


// assume that versions are not back compatible 
// version 4 bit: 16 possible value


typedef enum : uint8_t {
    MESSAGE_KIND_LOG = 0,
    MESSAGE_KIND_PREPARE = 1, // only from the application -> server -> launchpad core 
    MESSAGE_KIND_PREPARE_STATUS = 2, // only from the launchpad core -> server -> application 
    MESSAGE_KIND_LAUNCH = 3, // only from the application -> server -> launchpad core -> rocket
    MESSAGE_KIND_LAUNCH_STATUS = 3, // only from the rocket -> launchpad core -> server -> application 
    MESSAGE_KIND_ACK = 4, // general ack message, either from the server or the launchpad core
    MESSAGE_REQUEST = 5,


    MESSAGE_KIND_NONE = 0b1111,
} MessageKind;



struct [[packed]] LogMessage 
{
    static constexpr MessageKind kind = MESSAGE_KIND_LOG;
    uint8_t log_level;
    uint8_t log_length;
    char log[16];
};

struct [[packed]] PrepareMessage 
{
    static constexpr MessageKind kind = MESSAGE_KIND_PREPARE;
    uint8_t pressure; // 0-255 -> 0-10bar (float)
    uint8_t water_fill; // 0-255 -> 0-100% (float)
    bool enable_data_logging;
};

struct [[packed]] PrepareStatusMessage 
{
    static constexpr MessageKind kind = MESSAGE_KIND_PREPARE_STATUS;
    uint8_t current_pressure; // 0-255 -> 0-10bar (float)
    uint8_t current_water_fill; // 0-255 -> 0-100% (float)
    bool ready_to_launch;
};

struct [[packed]] LaunchMessage 
{
    static constexpr MessageKind kind = MESSAGE_KIND_LAUNCH;
    uint32_t launch_id; // 0-4294967295
};

struct [[packed]] LaunchStatusMessage 
{
    static constexpr MessageKind kind = MESSAGE_KIND_LAUNCH_STATUS;
    uint8_t altitude; // 0-255 -> 0-1km (float)
    int8_t speed[3]; // 0-255 -> 0-100m/s (float)
    int16_t acceleration[3]; // 0-65556 -> -10g-10g (float)
    int16_t gyro[3]; // 0- -> 0-1000deg/s (float)
    uint8_t battery; // 0-255 -> 0-100% (float)
};

struct [[packed]] NoneMessage 
{
    static constexpr MessageKind kind = MESSAGE_KIND_NONE;
    bool _empty;
};

struct [[packed]] ProtocolMessage 
{
    uint8_t version : 4;
    uint8_t message_kind : 4;
    union {
        LogMessage log_message;
        PrepareMessage prepare_message;
        PrepareStatusMessage prepare_status_message;
        LaunchMessage launch_message;
        LaunchStatusMessage launch_status_message;
    };
};


static inline constexpr uint8_t SERVER_S_ADDRESS = 0x08;
static inline constexpr uint8_t LAUNCHPAD_S_ADDRESS = 0x09;
static inline constexpr uint8_t ROCKET_S_ADDRESS = 0x0A;


static size_t buffered_message = 0;
static ProtocolMessage send_message_buffer[128];

static size_t received_message = 0;
static ProtocolMessage received_message_buffer[128];

static inline bool has_received_message(ProtocolMessage *message) {
    if (received_message == 0) {
        *message = (ProtocolMessage){};
        message->version = PROTOCOL_VER;
        message->message_kind = MESSAGE_KIND_NONE;
        return false;
    }

    *message = received_message_buffer[0];
    for (size_t i = 0; i < received_message - 1; i++) {
        received_message_buffer[i] = received_message_buffer[i + 1];
    }
    received_message--;
    return true;
}

static inline bool add_received_message(ProtocolMessage *message) {
    if (received_message > 128) {
        // led should blink
        Serial.println("Buffer overflow"); 
        return false;
    }

    received_message_buffer[received_message] = *message;
    received_message++;

    return true;
}




static inline bool send_message_to_master(ProtocolMessage *message) {
    if (buffered_message > 128) {
        // led should blink
        Serial.println("Buffer overflow"); 
        return false;
    }

    send_message_buffer[buffered_message] = *message;
    buffered_message++;
    return true;
}

static inline bool request_from_master_callback(ProtocolMessage *message) {
    if(buffered_message == 0) {
        *message = (ProtocolMessage){};
        message->version = PROTOCOL_VER;
        message->message_kind = MESSAGE_KIND_NONE;
        return false;
    }

    *message = send_message_buffer[0];
    for (size_t i = 0; i < buffered_message - 1; i++) {
        send_message_buffer[i] = send_message_buffer[i + 1];
    }
    buffered_message--;
    return true;
}




// as master
static inline bool send_message_to_slave(ProtocolMessage *message, uint8_t address) {
    Wire.beginTransmission(address);
    size_t res = Wire.write((char *)message, sizeof(ProtocolMessage));
    if (res != sizeof(ProtocolMessage)) {
        Serial.println("Error: failed to send message");
        Wire.endTransmission();
        return false;
    }
    Wire.endTransmission();
    return true;
}


static inline bool request_from_slave(ProtocolMessage *message, uint8_t address) {


    
    Wire.requestFrom(address, sizeof(ProtocolMessage));
    if (Wire.available() == sizeof(ProtocolMessage)) {
        Wire.readBytes((char *)message, sizeof(ProtocolMessage));
        return true;
    }
    else if(Wire.available() != 0) {
        Serial.println("Error: failed to receive message");
        return false;
    }
    *message = (ProtocolMessage){};
    message->version = PROTOCOL_VER;
    message->message_kind = MESSAGE_KIND_NONE;
    return false;
}


static inline bool setup_communication_master() {
    Wire.begin();
    Wire.setClock(400000);


    return true;
}

static inline bool setup_communication_slave(int sda, int scl, uint8_t address) {
    //Wire.setClock(20000);
    Wire.onReceive([](size_t num_bytes) {
        if (num_bytes == sizeof(ProtocolMessage)) {
            Serial.println("Received message");
            ProtocolMessage message;
            Wire.readBytes((char *)&message, sizeof(ProtocolMessage));
            add_received_message(&message);
        }
        else {
            Serial.println("Error: received message size mismatch");
        }

    });

    Wire.onRequest([]() {
        ProtocolMessage message;
        if (request_from_master_callback(&message)) {
            Serial.println("Sending message");
            Wire.write((char *)&message, sizeof(ProtocolMessage));
        }
        else {
            Serial.println("Error: no message to send");
            Wire.write((char *)&message, sizeof(ProtocolMessage));
        }
    });
    return true;
}