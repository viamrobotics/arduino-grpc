#include "../utils/result.h"

uint32_t varIntSize(uint8_t n, uint64_t i);
uint32_t writeVarInt(uint8_t n, uint64_t i, uint8_t* buffer);
Result<uint64_t> readVarInt(uint8_t n, uint8_t* p, uint32_t& pIdx, uint32_t& pLenRem);
void writeUint32(uint32_t i, uint8_t* buffer);
uint32_t readUint32(uint8_t* buffer);
