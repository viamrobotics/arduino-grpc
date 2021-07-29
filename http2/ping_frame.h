#pragma once

#include "frame.h"
#include <string.h>

#define PING_FLAG_ACK 0x1

struct PingFrame : Frame {
   PingFrame(uint8_t* data)
     : Frame(FrameType::PING, 0) {
        memcpy(this->opaque, data, 8);
     }

    PingFrame(FrameHeader header, uint8_t* data)
     : Frame(header) {
        memcpy(this->opaque, data, 8);
     }

    static PingFrame ack(uint8_t* data);
    static PingFrame* makeAck(uint8_t* data);
    static PingFrame ack(FrameHeader header, uint8_t* data);
    static PingFrame* makeAck(FrameHeader header, uint8_t* data);

    uint32_t payloadLength();
    void writePayload(Transport* transport);

    uint8_t opaque[8] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
};
