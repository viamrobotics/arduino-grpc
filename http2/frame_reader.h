#pragma once

#include <stdlib.h>
#include <stdint.h>
#include "frame.h"
#include "frame_header_reader.h"
#include "../utils/result.h"

struct FrameReader {
    ~FrameReader() {
        if (this->framePayload != nullptr) {
            delete[] this->framePayload;
        }
        if (this->_frame != nullptr) {
            delete this->_frame;
        }
    }
    Frame* _frame = nullptr;
    FrameHeaderReader frameHeaderReader = FrameHeaderReader();
    Result<bool> pump(uint8_t byte);
    Result<Frame*> frame();

    bool readingHeader = true;
    bool readingPayload = false;
    uint8_t framePayloadBytesLeft = 0;
    uint8_t* framePayload = nullptr;
};
