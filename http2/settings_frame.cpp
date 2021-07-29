#include "settings_frame.h"

SettingsFrame::SettingsFrame(uint8_t streamID) : Frame(FrameType::SETTINGS, streamID) {
    this->settings = std::vector<Setting>();
}

SettingsFrame::SettingsFrame(FrameHeader header) : Frame(header) {
    this->settings = std::vector<Setting>();
}

uint32_t SettingsFrame::payloadLength() {
    return 6 * this->settings.size();
}

void SettingsFrame::writePayload(Transport * transport) {
    for (Setting setting : this->settings) {
        transport->write(static_cast<uint8_t>(0x00));
        transport->write(static_cast<uint8_t>(setting.parameter));
        transport->write(setting.value);
    }
}

SettingsFrame SettingsFrame::ack(uint8_t streamID) {
    SettingsFrame frame(streamID);
    frame.header.flags = SETTINGS_FLAG_ACK;
    return frame;
}

SettingsFrame* SettingsFrame::makeAck(uint8_t streamID) {
    SettingsFrame* frame = new SettingsFrame(streamID);
    frame->header.flags = SETTINGS_FLAG_ACK;
    return frame;
}

SettingsFrame SettingsFrame::ack(FrameHeader header) {
    SettingsFrame frame(header);
    frame.header.flags = SETTINGS_FLAG_ACK;
    return frame;
}

SettingsFrame* SettingsFrame::makeAck(FrameHeader header) {
    SettingsFrame* frame = new SettingsFrame(header);
    frame->header.flags = SETTINGS_FLAG_ACK;
    return frame;
}

