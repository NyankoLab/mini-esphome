// SPDX-License-Identifier: MIT
#include <stdio.h>
#include <sys/socket.h>

#include "ESPHomeAPI.h"
#include "ESPHomeMessage.h"

#define HAVE_DUMP_BINARY    1
#define HAVE_DUMP_STRUCT    1
#define HAVE_SOURCE_SERVER  1

namespace ESPHome {

extern Callback const API[16];

static void DumpBinary(int fd, const char* buffer, int length)
{
#if HAVE_DUMP_BINARY
    printf("%d : ", fd);
    for (int i = 0; i < length; ++i) {
        printf("%s%02X", i == 0 ? "" : ".", buffer[i]);
    }
    printf("\n");
#endif
}

template<typename T = void*, typename U = void*>
static void DumpType(char const* format,
                     char const* tab = nullptr,
                     char const* type = nullptr,
                     char const* name = nullptr,
                     T const& value = 0,
                     U const& padding = 0)
{
    if constexpr(requires(T t) { t->size(); t->data(); }) {
        printf("%s%s %s = \"%.*s\"\n", tab, type, name, (int)value->size(), value->data());
        return;
    }
    printf(format, tab, type, name, value, padding);
}

template<class T>
static void DumpStruct(T* payload)
{
#if HAVE_DUMP_STRUCT
    __builtin_dump_struct(payload, DumpType);
#endif
}

static void Message(int fd, void* data, int type, int id, int value, int high, const char* array)
{
    auto callback = (type < 16) ? API[type] : nullptr;
    if (callback) {
        callback(fd, data, type, id, value, high, array);
        return;
    }
    if (id == -1) {
        printf("[%d:%d] : END\n", type, id);
    }
    else if (array) {
        printf("[%d:%d] : (%.*s)\n", type, id, value, array);
    }
    else {
        printf("[%d:%d] : %d\n", type, id, value);
    }
}

void Send(int fd, void* data, int type, ...)
{
    char* buffer = (char*)malloc(1024);
    if (buffer == nullptr)
        return;
    va_list va;
    va_start(va, type);
    int length = EncodeMessage(buffer, type, va);
    va_end(va);
    DumpBinary(fd, buffer, length);
    DecodeMessage(buffer, API[type], fd, data);
    send(fd, buffer, length, 0);
    free(buffer);
}

void Recv(int fd, const char* buffer, int length)
{
    DumpBinary(fd, buffer, length);

    char* data = (char*)malloc(1024);
    DecodeMessage(buffer, Message, fd, data);
    free(data);
}

Callback const API[16] =
{
    [HelloRequest::id] = [](int fd, void* data, int type, int id, int value, int high, const char* array) {
        struct HelloRequest* payload = (struct HelloRequest*)data;
        switch (id) {
        case 1:     payload->client_info = std::string_view(array, array + value);  break;
        case 2:     payload->api_version_major = value;                             break;
        case 3:     payload->api_version_minor = value;                             break;
        case -1:    DumpStruct(payload);
                    Send(fd, data, HelloResponse::id,
                                   Tag(1, VARINT), 1,
                                   Tag(2, VARINT), 14,
                                   Tag(3, LEN), Text("nyanko"),
                                   Tag(4, LEN), Text("name"),
                                   Tag());
                    break;
        }
    },
#if HAVE_SOURCE_SERVER
    [HelloResponse::id] = [](int fd, void* data, int type, int id, int value, int high, const char* array) {
        struct HelloResponse* payload = (struct HelloResponse*)data;
        switch (id) {
        case 1:     payload->api_version_major = value;                             break;
        case 2:     payload->api_version_minor = value;                             break;
        case 3:     payload->server_info = std::string_view(array, array + value);  break;
        case 4:     payload->name = std::string_view(array, array + value);         break;
        case -1:    DumpStruct(payload);                                            break;
        }
    },
#endif
    [AuthenticationRequest::id] = [](int fd, void* data, int type, int id, int value, int high, const char* array) {
        struct AuthenticationRequest* payload = (struct AuthenticationRequest*)data;
        switch (id) {
        case 1:     payload->password = std::string_view(array, array + value);     break;
        case -1:    DumpStruct(payload);
                    Send(fd, data, AuthenticationResponse::id,
                                   Tag(1, VARINT), 0,
                                   Tag());
                    break;
        }
    },
#if HAVE_SOURCE_SERVER
    [AuthenticationResponse::id] = [](int fd, void* data, int type, int id, int value, int high, const char* array) {
        struct AuthenticationResponse* payload = (struct AuthenticationResponse*)data;
        switch (id) {
        case 1:     payload->invalid_password = value;                              break;
        case -1:    DumpStruct(payload);                                            break;
        }
    },
#endif
    [DisconnectRequest::id] = [](int fd, void* data, int type, int id, int value, int high, const char* array) {
        struct DisconnectRequest* payload = (struct DisconnectRequest*)data;
        switch (id) {
        case 1:     payload->reason = DisconnectReason(value);                      break;
        case -1:    DumpStruct(payload);
                    Send(fd, data, DisconnectResponse::id,
                                   Tag());
                    break;
        }
    },
#if HAVE_SOURCE_SERVER
    [DisconnectResponse::id] = [](int fd, void* data, int type, int id, int value, int high, const char* array) {
        struct DisconnectResponse* payload = (struct DisconnectResponse*)data;
        DumpStruct(payload);
    },
#endif
    [PingRequest::id] = [](int fd, void* data, int type, int id, int value, int high, const char* array) {
        struct PingRequest* payload = (struct PingRequest*)data;
        DumpStruct(payload);
        Send(fd, data, PingResponse::id,
                       Tag());
    },
#if HAVE_SOURCE_SERVER
    [PingResponse::id] = [](int fd, void* data, int type, int id, int value, int high, const char* array) {
        struct PingResponse* payload = (struct PingResponse*)data;
        DumpStruct(payload);
    },
#endif
    [DeviceInfoRequest::id] = [](int fd, void* data, int type, int id, int value, int high, const char* array) {
        struct DeviceInfoRequest* payload = (struct DeviceInfoRequest*)data;
        DumpStruct(payload);
        Send(fd, data, DeviceInfoResponse::id,
//                     Tag(1, VARINT), 0,
                       Tag(2, LEN), Text("name"),
                       Tag(3, LEN), Text("00:00:00:00:00:00"),
                       Tag(4, LEN), Text("1.14.0"),
                       Tag(5, LEN), Text(__DATE__),
                       Tag(6, LEN), Text("model"),
                       Tag(12, LEN), Text("nyanko"),
                       Tag(13, LEN), Text("friendly_name"),
//                     Tag(19, VARINT), 0,
                       Tag());
    },
#if HAVE_SOURCE_SERVER
    [DeviceInfoResponse::id] = [](int fd, void* data, int type, int id, int value, int high, const char* array) {
        struct DeviceInfoResponse* payload = (struct DeviceInfoResponse*)data;
        switch (id) {
//      case 1:     payload->uses_password = value;                                     break;
        case 2:     payload->name = std::string_view(array, array + value);             break;
        case 3:     payload->mac_address = std::string_view(array, array + value);      break;
        case 4:     payload->esphome_version = std::string_view(array, array + value);  break;
        case 5:     payload->compilation_time = std::string_view(array, array + value); break;
        case 6:     payload->model = std::string_view(array, array + value);            break;
        case 12:    payload->manufacturer = std::string_view(array, array + value);     break;
        case 13:    payload->friendly_name = std::string_view(array, array + value);    break;
//      case 19:    payload->api_encryption_supported = value;                          break;
        case -1:    DumpStruct(payload);                                                break;
        }
    },
#endif
    [ListEntitiesRequest::id] = [](int fd, void* data, int type, int id, int value, int high, const char* array) {
        struct ListEntitiesRequest* payload = (struct ListEntitiesRequest*)data;
        DumpStruct(payload);
        Send(fd, data, ListEntitiesDoneResponse::id,
                       Tag());
    },
#if HAVE_SOURCE_SERVER
    [ListEntitiesDoneResponse::id] = [](int fd, void* data, int type, int id, int value, int high, const char* array) {
        struct ListEntitiesDoneResponse* payload = (struct ListEntitiesDoneResponse*)data;
        DumpStruct(payload);
    },
#endif
};

};
