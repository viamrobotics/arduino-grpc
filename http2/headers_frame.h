#pragma once

#include "frame.h"
#include "hpack.h"
#include <vector>

#define HEADERS_FLAG_END_STREAM 0x1
#define HEADERS_FLAG_END_HEADERS 0x4
#define HEADERS_FLAG_PADDED 0x8
#define HEADERS_FLAG_PRIORITY 0x20

struct HeadersFrame : Frame {
    ~HeadersFrame() {}
    HeadersFrame(FrameHeader header, std::vector<HeaderField> headerFields)
     : Frame(header), headerFields(headerFields) {}

    HeadersFrame(uint8_t streamID, std::vector<HeaderField> headerFields)
     : Frame(FrameType::HEADERS, streamID), headerFields(headerFields) {}

    uint32_t payloadLength();
    void writePayload(Transport* transport);

    bool priority = false;
    bool exclusive = false;
    uint32_t streamDependency = 0;
    uint8_t weight = 0;

    uint8_t* padding = nullptr;
    uint32_t paddingLength = 0;

    std::vector<HeaderField> headerFields = std::vector<HeaderField>();
};