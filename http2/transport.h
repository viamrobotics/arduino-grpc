#pragma once

#include <stdint.h>
#include <stddef.h>

class Transport {
    public:
        virtual size_t write(uint8_t byte) = 0;
        virtual int available() = 0 ;
        virtual int read() = 0;
        virtual bool connected() = 0;
        size_t write(uint32_t value);
};
