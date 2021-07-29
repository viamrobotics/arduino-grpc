#pragma once

#include <stdint.h>

enum FrameType {
    DATA = 0x0,
    HEADERS = 0x1,
    PRIORITY = 0x2,
    RST_STREAM = 0x3,
    SETTINGS = 0x4,
    PUSH_PROMISE = 0x5,
    PING = 0x6,
    GOAWAY = 0x7,
    WINDOW_UPDATE = 0x8,
    CONTINUATION = 0x9
};

struct FrameHeader {
    FrameHeader();
    FrameHeader(FrameType type, uint8_t streamID = 0);
    uint32_t length;
    FrameType type;
    uint8_t flags;
    uint32_t streamID;
};

