#pragma once

#include "frame.h"

struct WindowUpdateFrame : Frame {
  WindowUpdateFrame(FrameHeader header, uint32_t windowSizeIncrement)
     : Frame(header),
       windowSizeIncrement(windowSizeIncrement) {}

    WindowUpdateFrame(uint32_t streamID, uint32_t windowSizeIncrement)
     : Frame(FrameType::WINDOW_UPDATE, streamID),
       windowSizeIncrement(windowSizeIncrement) {}

    uint32_t payloadLength();
    void writePayload(Transport* transport);

    uint32_t windowSizeIncrement = 0;
};
