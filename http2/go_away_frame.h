#pragma once

#include "error_code.h"
#include "frame.h"

struct GoAwayFrame : Frame {
    ~GoAwayFrame() {}
    GoAwayFrame(uint32_t lastStreamID, ErrorCode errorCode)
     : Frame(FrameType::GOAWAY, 0),
       lastStreamID(lastStreamID),
       errorCode(errorCode) {}

    GoAwayFrame(uint32_t lastStreamID, ErrorCode errorCode, uint8_t* debugData, uint32_t debugDataLength)
     : Frame(FrameType::GOAWAY, 0),
       lastStreamID(lastStreamID),
       errorCode(errorCode),
       debugData(debugData),
       debugDataLength(debugDataLength) {}

    uint32_t payloadLength();
    void writePayload(Transport* transport);

    uint32_t lastStreamID = 0;
    ErrorCode errorCode = NO_ERROR;
    uint8_t* debugData;
    uint32_t debugDataLength;
};

