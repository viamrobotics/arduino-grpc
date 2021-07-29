#pragma once

#include <stdint.h>
#include "frame_header.h"
#include "../utils/result.h"

struct FrameHeaderReader {
    FrameHeaderReader();
    FrameHeader _header;
    uint8_t pos;
    Result<bool> pump(uint8_t byte);
    FrameHeader header();
};
