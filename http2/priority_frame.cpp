#include "priority_frame.h"

uint32_t PriorityFrame::payloadLength() {
    return 5;
}

void PriorityFrame::writePayload(Transport * transport) {
    uint32_t streamDependency = this->streamDependency;
    if (this->exclusive) {
        streamDependency |= 1 << 31;    
    }
    transport->write(streamDependency);
    transport->write(this->weight);
}
