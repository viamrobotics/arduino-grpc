#pragma once

#include <stdint.h>
#include "frame_header.h"
#include "transport.h"

struct Frame {
public:
    virtual ~Frame() {}
    Frame(FrameType frameType, uint8_t streamID = 0);
    Frame(FrameHeader header) : header(header) {} ;
    FrameHeader header;
    virtual uint32_t payloadLength() = 0;
    virtual void writePayload(Transport * transport) = 0;
    void write(Transport * transport);
};
