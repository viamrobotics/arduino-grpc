#pragma once

#include "../utils/result.h"
#include <memory>

using namespace std;

Result<shared_ptr<char>> huffmanDecode(uint32_t maxLen, uint8_t* v, uint32_t vLen, uint32_t& decodedLen);
uint64_t HuffmanEncodeLength(uint8_t* s, uint32_t sLen);
shared_ptr<uint8_t> huffmanEncode(uint8_t* s, uint32_t sLen, uint32_t& encodedLen);

