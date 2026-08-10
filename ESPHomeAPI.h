// SPDX-License-Identifier: MIT
#pragma once

#include <stdarg.h>
#include <stdint.h>
#include <string.h>
#include <string_view>

namespace ESPHome {

enum DisconnectReason
{
    DISCONNECT_REASON_UNSPECIFIED = 0,
    DISCONNECT_REASON_PROVISIONING_CLOSED,
};

enum WireType
{
    VARINT = 0, // 0 int32, int64, uint32, uint64, sint32, sint64, bool, enum
    I64,        // 1 fixed64, sfixed64, double
    LEN,        // 2 string, bytes, embedded messages, packed repeated fields
    SGROUP,     // 3 group start (deprecated)
    EGROUP,     // 4 group end (deprecated)
    I32,        // 5 fixed32, sfixed32, float
};
static_assert(I32 == 5);

struct HelloRequest
{
    std::string_view client_info;
    uint32_t api_version_major;
    uint32_t api_version_minor;

    static constexpr int id = 1;
};

struct HelloResponse
{
    uint32_t api_version_major;
    uint32_t api_version_minor;
    std::string_view server_info;
    std::string_view name;

    static constexpr int id = 2;
};

struct AuthenticationRequest
{
    std::string_view password;

    static constexpr int id = 3;
};

struct AuthenticationResponse
{
    bool invalid_password;

    static constexpr int id = 4;
};

struct DisconnectRequest
{
    DisconnectReason reason;

    static constexpr int id = 5;
};

struct DisconnectResponse
{
    static constexpr int id = 6;
};

struct PingRequest
{
    static constexpr int id = 7;
};

struct PingResponse
{
    static constexpr int id = 8;
};

struct DeviceInfoRequest
{
    static constexpr int id = 9;
};

struct DeviceInfoResponse
{
//  bool uses_password;
    std::string_view name;
    std::string_view mac_address;
    std::string_view esphome_version;
    std::string_view compilation_time;
    std::string_view model;
    std::string_view manufacturer;
    std::string_view friendly_name;
//  bool api_encryption_supported;

    static constexpr int id = 10;
};

struct ListEntitiesRequest
{
    static constexpr int id = 11;
};

struct ListEntitiesDoneResponse
{
    static constexpr int id = 12;
};

void Send(int fd, void* data, int type, ...);
void Recv(int fd, const char* buffer, int length);

};
