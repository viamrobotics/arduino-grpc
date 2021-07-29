#pragma once

#include "frame.h"
#include "error_code.h"

struct ResetStreamFrame : Frame {
    ~ResetStreamFrame() {}
    ResetStreamFrame(FrameHeader header, ErrorCode errorCode = NO_ERROR)
     : Frame(header), errorCode(errorCode) {}

    ResetStreamFrame(uint8_t streamID = 0, ErrorCode errorCode = NO_ERROR)
     : Frame(FrameType::RST_STREAM, streamID), errorCode(errorCode) {}
    ErrorCode errorCode;

    uint32_t payloadLength();
    void writePayload(Transport* transport);
};
