#pragma once

#include <vector>
#include "frame.h"

#define SETTINGS_FLAG_ACK 0x1

enum SettingParameter {
    SETTINGS_HEADER_TABLE_SIZE = 0x1,
    SETTINGS_ENABLE_PUSH = 0x2,
    SETTINGS_MAX_CONCURRENT_STREAMS = 0x3,
    SETTINGS_INITIAL_WINDOW_SIZE = 0x4,
    SETTINGS_MAX_FRAME_SIZE = 0x5,
    SETTINGS_MAX_HEADER_LIST_SIZE = 0x6
};

struct Setting {
    SettingParameter parameter;
    uint32_t value;
};

struct SettingsFrame : Frame {
    ~SettingsFrame() {}
    SettingsFrame(FrameHeader header);
    SettingsFrame(uint8_t streamID = 0);
    static SettingsFrame ack(uint8_t streamID = 0);
    static SettingsFrame* makeAck(uint8_t streamID = 0);
    static SettingsFrame ack(FrameHeader header);
    static SettingsFrame* makeAck(FrameHeader header);
    std::vector<Setting> settings;

    uint32_t payloadLength();
    void writePayload(Transport* transport);
};
