#pragma once

#include "frame.h"

struct PushPromiseFrame : Frame {
    PushPromiseFrame(uint8_t streamID = 0)
     : Frame(FrameType::PUSH_PROMISE, streamID) {}

    uint32_t payloadLength();
    void writePayload(Transport* transport);
};
