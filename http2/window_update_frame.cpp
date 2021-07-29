#include "window_update_frame.h"

uint32_t WindowUpdateFrame::payloadLength() {
    return 4;
}

void WindowUpdateFrame::writePayload(Transport * transport) {
    transport->write(this->windowSizeIncrement);
}
