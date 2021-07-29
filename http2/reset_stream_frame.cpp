#include "reset_stream_frame.h"

uint32_t ResetStreamFrame::payloadLength() {
    return 4;
}

void ResetStreamFrame::writePayload(Transport * transport) {
    transport->write(static_cast<uint32_t>(this->errorCode));
}

