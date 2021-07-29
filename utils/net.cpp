#include "net.h"

Result<uint64_t> readVarInt(uint8_t n, uint8_t* p, uint32_t& pIdx, uint32_t& pLenRem) {
    if (n < 1 || n > 8) {
        return Result<uint64_t>::ofError("bad n");
    }
    if (pLenRem == 0) {
        return Result<uint64_t>::ofError("errNeedMore");
    }
    uint64_t i = static_cast<uint64_t>(p[pIdx]);
    if (n < 8) {
        i &= (1 << static_cast<uint64_t>(n)) - 1;
    }
    if (i < (1<<static_cast<uint64_t>(n))-1) {
    	pIdx++;
    	pLenRem--;    
        return Result<uint64_t>::ofValue(i);
    }

    uint8_t* origP = p;
    pIdx++;
    pLenRem--;
    uint64_t m = 0;
    while (pLenRem > 0) {
        uint8_t b = origP[pIdx];
        pIdx++;
        pLenRem--;
        i += static_cast<uint64_t>(b&127) << m;
        if ((b&128) == 0) {
            return Result<uint64_t>::ofValue(i);
        }
        m += 7;
        if (m >= 63) { // TODO: proper overflow check. making this up.
            return Result<uint64_t>::ofError("errVarintOverflow");
        }
    }
    return Result<uint64_t>::ofError("errNeedMore");
}

uint32_t readUint32(uint8_t* buffer) {
    return (buffer[0] << 24) + (buffer[1] << 16) + (buffer[2] << 8) + buffer[3];
}

void writeUint32(uint32_t i, uint8_t* buffer) {
    buffer[0] = static_cast<uint8_t>((i & 0xff000000) >> 24);
    buffer[1] = static_cast<uint8_t>((i & 0xff0000) >> 16);
    buffer[2] = static_cast<uint8_t>((i & 0xff00) >> 8);
    buffer[3] = static_cast<uint8_t>(i & 0xff);
}

uint32_t varIntSize(uint8_t n, uint64_t i) {
    uint64_t k = ((1 << n) - 1);
    if (i < k) {
        return 1;
    }
    uint32_t size = 1;
    i -= k;
    for (; i >= 128; i >>= 7) {
        size++;
    }
    size++;
    return size;
}

uint32_t writeVarInt(uint8_t n, uint64_t i, uint8_t* buffer) {
    uint32_t bufferIndex = 0;
    uint64_t k = ((1 << n) - 1);
    uint32_t size = 1;
    if (i < k) {
        buffer[bufferIndex] = static_cast<uint8_t>(i);
        return size;
    }
    buffer[bufferIndex++] = static_cast<uint8_t>(k);
    i -= k;
    for (; i >= 128; i >>= 7) {
        buffer[bufferIndex++] = static_cast<uint8_t>(0x80|(i&0x7f));
        size++;
    }
    size++;
    buffer[bufferIndex] = static_cast<uint8_t>(i);
    return size;
}
