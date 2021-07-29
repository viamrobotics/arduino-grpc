#pragma once

#include "frame.h"

struct PriorityFrame : Frame {
    PriorityFrame(FrameHeader header,
                  bool exclusive,
                  uint32_t streamDependency,
                  uint8_t weight)
     : Frame(header),
       exclusive(exclusive),
       streamDependency(streamDependency),
       weight(weight) {}

    PriorityFrame(uint8_t streamID,
                  bool exclusive,
                  uint32_t streamDependency,
                  uint8_t weight)
     : Frame(FrameType::PRIORITY, streamID),
       exclusive(exclusive),
       streamDependency(streamDependency),
       weight(weight) {}

    uint32_t payloadLength();
    void writePayload(Transport* transport);

    bool exclusive = false;
    uint32_t streamDependency = 0;
    uint8_t weight = 0;
};
