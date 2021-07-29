#include "ping_frame.h"

uint32_t PingFrame::payloadLength() {
    return 8;
}

void PingFrame::writePayload(Transport * transport) {
    for (uint8_t i = 0; i < 8; i++) {
        transport->write(this->opaque[i]);
    }
}

PingFrame PingFrame::ack(uint8_t* data) {
    PingFrame frame = PingFrame(data);
    frame.header.flags = PING_FLAG_ACK;
    return frame;
}

PingFrame* PingFrame::makeAck(uint8_t* data) {
    PingFrame* frame = new PingFrame(data);
    frame->header.flags = PING_FLAG_ACK;
    return frame;
}

PingFrame PingFrame::ack(FrameHeader header, uint8_t* data) {
    PingFrame frame = PingFrame(header, data);
    frame.header.flags = PING_FLAG_ACK;
    return frame;
}

PingFrame* PingFrame::makeAck(FrameHeader header, uint8_t* data) {
    PingFrame* frame = new PingFrame(header, data);
    frame->header.flags = PING_FLAG_ACK;
    return frame;
}
