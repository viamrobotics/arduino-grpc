#pragma once

#include "frame.h"

#define DATA_FLAG_END_STREAM 0x1
#define DATA_FLAG_PADDED 0x8

struct DataFrame : Frame {
    ~DataFrame() {}
    DataFrame(uint8_t streamID = 0)
     : Frame(FrameType::DATA, streamID) {}

    DataFrame(uint8_t streamID,
              uint8_t* data,
              uint32_t dataLength)
     : Frame(FrameType::DATA, streamID),
       data(data),
       dataLength(dataLength) {}

    DataFrame(FrameHeader header,
              uint8_t* data,
              uint32_t dataLength)
     : Frame(header),
       data(data),
       dataLength(dataLength) {}

    DataFrame(uint8_t streamID,
              uint8_t* data,
              uint32_t dataLength,
              uint8_t* padding,
              uint32_t paddingLength)
     : Frame(FrameType::DATA, streamID),
       data(data),
       dataLength(dataLength),
       padding(padding),
       paddingLength(paddingLength) {
        this->header.flags |= DATA_FLAG_PADDED;
    }

    uint32_t payloadLength();
    void writePayload(Transport* transport);

    uint8_t* data = nullptr;
    uint32_t dataLength = 0;
    uint8_t* padding = nullptr;
    uint32_t paddingLength = 0;
};
