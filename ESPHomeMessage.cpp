// SPDX-License-Identifier: MIT
#include <stdio.h>

#include "ESPHomeAPI.h"
#include "ESPHomeMessage.h"

namespace ESPHome {

int Read16BE(const char*& buffer)
{
    int value = 0;
    value |= uint8_t(*buffer++) << 8;
    value |= uint8_t(*buffer++);
    return value;
}

int Read16LE(const char*& buffer)
{
    int value = 0;
    value |= uint8_t(*buffer++);
    value |= uint8_t(*buffer++) << 8;
    return value;
}

int Read32BE(const char*& buffer)
{
    int value = 0;
    value |= uint8_t(*buffer++) << 24;
    value |= uint8_t(*buffer++) << 16;
    value |= uint8_t(*buffer++) << 8;
    value |= uint8_t(*buffer++);
    return value;
}

int Read32LE(const char*& buffer)
{
    int value = 0;
    value |= uint8_t(*buffer++);
    value |= uint8_t(*buffer++) << 8;
    value |= uint8_t(*buffer++) << 16;
    value |= uint8_t(*buffer++) << 24;
    return value;
}

void Write16BE(char*& buffer, int value)
{
    (*buffer++) = value >> 8;
    (*buffer++) = value;
}

void Write16LE(char*& buffer, int value)
{
    (*buffer++) = value;
    (*buffer++) = value >> 8;
}

void Write32BE(char*& buffer, int value)
{
    (*buffer++) = value >> 24;
    (*buffer++) = value >> 16;
    (*buffer++) = value >> 8;
    (*buffer++) = value;
}

void Write32LE(char*& buffer, int value)
{
    (*buffer++) = value;
    (*buffer++) = value >> 8;
    (*buffer++) = value >> 16;
    (*buffer++) = value >> 24;
}

int DecodeBase128(const char*& buffer)
{
    uint8_t byte = 0;
    int shift = 0;
    int value = 0;
    do {
        byte = (*buffer++);
        value |= uint32_t(byte & 0x7F) << shift;
        shift += 7;
    } while (byte & 0x80);
    return value;
}

void EncoderBase128(char*& buffer, int value)
{
    uint8_t byte = 0;
    do {
        byte = uint8_t(value & 0x7F);
        value >>= 7;
        if (value)
            byte |= 0x80;
        (*buffer++) = byte;
    } while (value);
}

bool DecodeProtobuf(const char* buffer, const char* end, int type, Callback callback, int fd, void* data)
{
    uint32_t high = 0;
    uint32_t value = 0;
    while (buffer < end) {
        int tag = DecodeBase128(buffer);
        int id = (tag >> 3);
        int wire = (tag & 0x07);
        high = 0;
        value = 0;
        switch (wire) {
        case VARINT:
        case LEN:
            value = DecodeBase128(buffer);
            if (wire == LEN) {
                callback(fd, data, type, id, value, 0, buffer);
                buffer += value;
            }
            else {
                callback(fd, data, type, id, value, 0, nullptr);
            }
            break;
        case I64:
        case I32:
            value = Read32LE(buffer);
            if (wire == I64) {
                high = Read32LE(buffer);
            }
            callback(fd, data, type, id, value, high, nullptr);
            break;
        default:
            return false;
        }
    }
    return true;
}

void EncodeProtobuf(char*& buffer, va_list va)
{
    uint32_t high = 0;
    uint32_t value = 0;
    bool end = false;
    while (end == false) {
        int tag = va_arg(va, int);
        int wire = (tag & 0x07);
        high = 0;
        value = 0;
        EncoderBase128(buffer, tag);
        switch (wire) {
        case VARINT:
        case LEN:
            value = va_arg(va, int);
            EncoderBase128(buffer, value);
            if (wire == LEN) {
                void* array = va_arg(va, void*);
                memcpy(buffer, array, value);
                buffer += value;
            }
            break;
        case I64:
        case I32:
            Write32LE(buffer, value = va_arg(va, int));
            if (wire == I64) {
                Write32LE(buffer, high = va_arg(va, int));
            }
            break;
        default:
            buffer--;
            end = true;
            break;
        }
    }
}

int DecodeMessage(const char* buffer, Callback callback, int fd, void* data)
{
    const char* start = buffer;
    int type = 0;
    int offset = 0;
    int size = LengthMessage(buffer, &type, &offset);

    if (callback) {
        if (DecodeProtobuf(start + offset, start + size, type, callback, fd, data)) {
            callback(fd, data, type, -1, 0, 0, nullptr);
        }
    }
    return size;
}

int EncodeMessage(char* buffer, int type, va_list va)
{
    char* start = buffer;
    (*buffer++) = 0x00;
    (*buffer++) = 0x00;
    (*buffer++) = 0x00;
    EncodeProtobuf(buffer, va);

    int offset = 3;
    int size = int(buffer - start) - 3;
    if (size >= 16384)
        offset++;
    if (size >= 128)
        offset++;
    if (type >= 128)
        offset++;
    if (offset > 3) {
        memmove(buffer + offset, buffer + 3, size);
    }
    char* header = start;
    (*header++) = 0x00;
    EncoderBase128(header, size);
    EncoderBase128(header, type);
    return size + offset;
}

int LengthMessage(const char* buffer, int* type, int* offset)
{
    const char* start = buffer;
    int values[3] = {};
    values[0] = (*buffer++);
    values[1] = DecodeBase128(buffer);
    values[2] = DecodeBase128(buffer);
    if (type) {
        (*type) = values[2];
    }
    if (offset) {
        (*offset) = int(buffer - start);
    }
    return values[1] + int(buffer - start);
}

};
