// SPDX-License-Identifier: MIT
#pragma once

#include <stdint.h>
#include <string_view>

namespace ESPHome {
namespace Message {

typedef void (*Callback)(int fd, void* data, int type, int id, int integer, int upper, std::string_view string);

inline double CastDouble(int integer, int upper) {
    uint64_t v = 0;
    v |= uint32_t(integer);
    v |= uint64_t(upper) << 32;
    return *(double*)&v;
}

inline float CastFloat(int integer) {
    return *(float*)&integer;
}

inline int CastInt(float number) {
    return *(int*)&number;
}

inline int DecodeInt(int integer) {
    return (integer >> 1) ^ -(integer & 1);
}

inline int EncodeInt(int integer) {
    return (integer << 1) ^ (integer >> 31);
}

inline int Tag(int field = 0, int wire = 7) {
    return (field << 3) | wire;
}

#define Text(text) strlen(text), text

enum WireType {
    VARINT = 0, // 0 int32, int64, uint32, uint64, sint32, sint64, bool, enum
    I64,        // 1 fixed64, sfixed64, double
    LEN,        // 2 string, bytes, embedded messages, packed repeated fields
    SGROUP,     // 3 group start (deprecated)
    EGROUP,     // 4 group end (deprecated)
    I32,        // 5 fixed32, sfixed32, float
};

int Decode(const char* buffer, Callback callback, int fd, void* data);
int Encode(char* buffer, int type, va_list va);
int Length(const char* buffer, int* type = nullptr, int* offset = nullptr);

};
};
