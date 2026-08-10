// SPDX-License-Identifier: MIT
#pragma once

#include <string_view>

namespace ESPHome {

enum WireType {
    VARINT = 0, // 0 int32, int64, uint32, uint64, sint32, sint64, bool, enum
    I64,        // 1 fixed64, sfixed64, double
    LEN,        // 2 string, bytes, embedded messages, packed repeated fields
    SGROUP,     // 3 group start (deprecated)
    EGROUP,     // 4 group end (deprecated)
    I32,        // 5 fixed32, sfixed32, float
};

#define CastFloat(number) (*(float*)&number)

inline int Tag(int field = 0, int wire = 7)
{
    return (field << 3) | wire;
}

#define Text(text) strlen(text), text

typedef void (*Callback)(int fd, void* data, int type, int id, int integer, int upper, std::string_view string);

bool DecodeProtobuf(const char* buffer, const char* end, int type, Callback callback, int fd, void* data);
void EncodeProtobuf(char*& buffer, va_list va);

int DecodeMessage(const char* buffer, Callback callback, int fd, void* data);
int EncodeMessage(char* buffer, int type, va_list va);
int LengthMessage(const char* buffer, int* type = nullptr, int* offset = nullptr);

};
