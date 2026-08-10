// SPDX-License-Identifier: MIT
#pragma once

namespace ESPHome {

inline int Tag(int field = 0, int wire = 7)
{
    return (field << 3) | wire;
}

#define Text(text) strlen(text), text

typedef void (*Callback)(int fd, void* data, int type, int id, int value, int high, const char* array);

bool DecodeProtobuf(const char* buffer, const char* end, int type, Callback callback, int fd, void* data);
void EncodeProtobuf(char*& buffer, va_list va);

int DecodeMessage(const char* buffer, Callback callback, int fd, void* data);
int EncodeMessage(char* buffer, int type, va_list va);
int LengthMessage(const char* buffer, int* type = nullptr, int* offset = nullptr);

};
