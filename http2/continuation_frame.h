#pragma once

#include "frame.h"

struct ContinuationFrame : Frame {
    ~ContinuationFrame() {}
    ContinuationFrame(uint32_t streamID)
     : Frame(FrameType::CONTINUATION, streamID) {}

    ContinuationFrame(FrameHeader header)
     : Frame(header) {}

    uint32_t payloadLength();
    void writePayload(Transport* transport);
};
