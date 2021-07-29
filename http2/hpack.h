#pragma once
#include "../utils/utils.h"
#include <memory>
#include <stdint.h>
#include <string.h>
#include <vector>
#include "../utils/result.h" 

using namespace std;

struct HeaderField {
    HeaderField() : HeaderField("", "") {}
    HeaderField(const char* name) : HeaderField(name, "") {}
    HeaderField(const char* name, const char* value) {
        if (name != nullptr) {
            size_t n = strlen(name) + 1;
            shared_ptr<char> ptr(new char[n], default_delete<char[]>());
            this->name = ptr;
            memcpy(this->name.get(), name, n);
        }
        if (value != nullptr) {
            size_t n = strlen(value) + 1;
            shared_ptr<char> ptr(new char[n], default_delete<char[]>());
            this->value = ptr;
            memcpy(this->value.get(), value, n);
        }
    }

    shared_ptr<char> name;
    shared_ptr<char> value;

    bool sensitive = false;
};

Result<vector<HeaderField>> decodeHeaderFields(uint8_t* data, uint32_t dataLength);
uint32_t encodeHeaderFieldsLen(const vector<HeaderField> & fields);
uint8_t* encodeHeaderFields(const vector<HeaderField> & fields, uint32_t &outLen);
