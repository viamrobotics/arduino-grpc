#include "../utils/utils.h"
#include "frame.h"

Frame::Frame(FrameType frameType, uint8_t streamID) : header(FrameHeader(frameType, streamID)) {
}

void Frame::write(Transport * transport) {
    // Header
    uint32_t length = this->payloadLength();
    transport->write(static_cast<uint8_t>((length & 0xff0000) >> 16));
    transport->write(static_cast<uint8_t>((length & 0xff00) >> 8));
    transport->write(static_cast<uint8_t>(length & 0xff));
    transport->write(static_cast<uint8_t>(this->header.type));
    transport->write(this->header.flags);
    transport->write(this->header.streamID);

    this->writePayload(transport);
}