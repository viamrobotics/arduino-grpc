#pragma once

#include <stdint.h>

struct UnaryMethodHandler {
	const char* name;
    uint8_t (*handler)(uint8_t* data, uint32_t dataLen, uint8_t** response, uint32_t* responseLen);
};
