// SPDX-License-Identifier: MIT
#include <stdio.h>
#include <sys/socket.h>

#include "API.h"
#include "Message.h"
#include "Setup.h"

#define HAVE_DUMP_BINARY    1
#define HAVE_DUMP_STRUCT    1
#define HAVE_SOURCE_SERVER  1

namespace ESPHome {

#define ESPHOME_API_COUNT           64
#define ESPHOME_API_VERSION_MAJOR   1
#define ESPHOME_API_VERSION_MINOR   14
#define ESPHOME_VERSION             "2026.7.4"
#define ESPHOME_BUFFER_SIZE         1024

extern Callback const API[ESPHOME_API_COUNT];

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
#if HAVE_DUMP_STRUCT
    if constexpr(requires(T t) { t->size(); t->data(); }) {
        printf("%s%s %s = \"%.*s\"\n", tab, type, name, (int)value->size(), value->data());
        return;
    }
    printf(format, tab, type, name, value, padding);
#endif
}

template<class T>
static void DumpStruct(T* payload)
{
#if HAVE_DUMP_STRUCT
    __builtin_dump_struct(payload, DumpType);
#endif
}

static void Message(int fd, void* data, int type, int id, int integer, int upper, std::string_view string)
{
    auto callback = (type < ESPHOME_API_COUNT) ? API[type] : nullptr;
    if (callback) {
        callback(fd, data, type, id, integer, upper, string);
        return;
    }
    if (id == -1) {
        printf("[%d:%d] : END\n", type, id);
    }
    else if (string.data()) {
        printf("[%d:%d] : (%.*s)\n", type, id, (int)string.size(), string.data());
    }
    else {
        printf("[%d:%d] : %d\n", type, id, integer);
    }
}

void Send(int fd, void* data, int type, ...)
{
    char* buffer = (char*)malloc(ESPHOME_BUFFER_SIZE);
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

    char* data = (char*)calloc(ESPHOME_BUFFER_SIZE, 1);
    DecodeMessage(buffer, Message, fd, data);
    free(data);
}

Callback const API[ESPHOME_API_COUNT] =
{
    [HelloRequest::id] = [](int fd, void* data, int type, int id, int integer, int upper, std::string_view string) {
        struct HelloRequest* payload = (struct HelloRequest*)data;
        switch (id) {
        case 1:     payload->client_info = string;                                  break;
        case 2:     payload->api_version_major = integer;                           break;
        case 3:     payload->api_version_minor = integer;                           break;
        case -1:    DumpStruct(payload);
                    Send(fd, data, HelloResponse::id,
                                   Tag(1, VARINT), ESPHOME_API_VERSION_MAJOR,
                                   Tag(2, VARINT), ESPHOME_API_VERSION_MINOR,
                                   Tag(3, LEN), Text(ESPHOME_VERSION),
                                   Tag(4, LEN), Text(Setup::Name),
                                   Tag());
                    break;
        }
    },
#if HAVE_SOURCE_SERVER
    [HelloResponse::id] = [](int fd, void* data, int type, int id, int integer, int upper, std::string_view string) {
        struct HelloResponse* payload = (struct HelloResponse*)data;
        switch (id) {
        case 1:     payload->api_version_major = integer;                           break;
        case 2:     payload->api_version_minor = integer;                           break;
        case 3:     payload->server_info = string;                                  break;
        case 4:     payload->name = string;                                         break;
        case -1:    DumpStruct(payload);                                            break;
        }
    },
#endif
    [AuthenticationRequest::id] = [](int fd, void* data, int type, int id, int integer, int upper, std::string_view string) {
        struct AuthenticationRequest* payload = (struct AuthenticationRequest*)data;
        switch (id) {
        case 1:     payload->password = string;                                     break;
        case -1:    DumpStruct(payload);
                    Send(fd, data, AuthenticationResponse::id,
                                   Tag(1, VARINT), false,
                                   Tag());
                    break;
        }
    },
#if HAVE_SOURCE_SERVER
    [AuthenticationResponse::id] = [](int fd, void* data, int type, int id, int integer, int upper, std::string_view string) {
        struct AuthenticationResponse* payload = (struct AuthenticationResponse*)data;
        switch (id) {
        case 1:     payload->invalid_password = integer;                            break;
        case -1:    DumpStruct(payload);                                            break;
        }
    },
#endif
    [DisconnectRequest::id] = [](int fd, void* data, int type, int id, int integer, int upper, std::string_view string) {
        struct DisconnectRequest* payload = (struct DisconnectRequest*)data;
        switch (id) {
        case 1:     payload->reason = DisconnectReason(integer);                    break;
        case -1:    DumpStruct(payload);
                    Send(fd, data, DisconnectResponse::id,
                                   Tag());
                    break;
        }
    },
#if HAVE_SOURCE_SERVER
    [DisconnectResponse::id] = [](int fd, void* data, int type, int id, int integer, int upper, std::string_view string) {
        struct DisconnectResponse* payload = (struct DisconnectResponse*)data;
        DumpStruct(payload);
    },
#endif
    [PingRequest::id] = [](int fd, void* data, int type, int id, int integer, int upper, std::string_view string) {
        struct PingRequest* payload = (struct PingRequest*)data;
        DumpStruct(payload);
        Send(fd, data, PingResponse::id,
                       Tag());
    },
#if HAVE_SOURCE_SERVER
    [PingResponse::id] = [](int fd, void* data, int type, int id, int integer, int upper, std::string_view string) {
        struct PingResponse* payload = (struct PingResponse*)data;
        DumpStruct(payload);
    },
#endif
    [DeviceInfoRequest::id] = [](int fd, void* data, int type, int id, int integer, int upper, std::string_view string) {
        struct DeviceInfoRequest* payload = (struct DeviceInfoRequest*)data;
        DumpStruct(payload);
        Send(fd, data, DeviceInfoResponse::id,
//                     Tag(1, VARINT), 0,
                       Tag(2, LEN), Text(Setup::Name),
                       Tag(3, LEN), Text(Setup::GetMacAddress()),
                       Tag(4, LEN), Text(ESPHOME_VERSION),
                       Tag(5, LEN), Text(__DATE__),
                       Tag(6, LEN), Text(Setup::Model),
                       Tag(12, LEN), Text(Setup::Manufacturer),
                       Tag(13, LEN), Text(Setup::FriendlyName),
                       Tag());
    },
#if HAVE_SOURCE_SERVER
    [DeviceInfoResponse::id] = [](int fd, void* data, int type, int id, int integer, int upper, std::string_view string) {
        struct DeviceInfoResponse* payload = (struct DeviceInfoResponse*)data;
        switch (id) {
//      case 1:     payload->uses_password = integer;                               break;
        case 2:     payload->name = string;                                         break;
        case 3:     payload->mac_address = string;                                  break;
        case 4:     payload->esphome_version = string;                              break;
        case 5:     payload->compilation_time = string;                             break;
        case 6:     payload->model = string;                                        break;
//      case 7:     payload->has_deep_sleep = integer;                              break;
//      case 8:     payload->project_name = string;                                 break;
//      case 9:     payload->project_version = string;                              break;
//      case 10:    payload->webserver_port = integer;                              break;
//      case 11:    payload->legacy_bluetooth_proxy_version = integer;              break;
//      case 15:    payload->bluetooth_proxy_feature_flags = integer;               break;
        case 12:    payload->manufacturer = string;                                 break;
        case 13:    payload->friendly_name = string;                                break;
//      case 14:    payload->legacy_voice_assistant_version = integer;              break;
//      case 17:    payload->voice_assistant_feature_flags = integer;               break;
//      case 16:    payload->suggested_area = string;                               break;
//      case 18:    payload->bluetooth_mac_address = string;                        break;
//      case 19:    payload->api_encryption_supported = integer;                    break;
//      case 20:    payload->devices = integer;                                     break;
//      case 21:    payload->areas = integer;                                       break;
//      case 22:    payload->area = integer;                                        break;
//      case 23:    payload->zwave_proxy_feature_flags = integer;                   break;
//      case 24:    payload->zwave_home_id = integer;                               break;
//      case 25:    payload->serial_proxies = integer;                              break;
//      case 26:    payload->api_encryption_provisionable = integer;                break;
        case -1:    DumpStruct(payload);                                            break;
        }
    },
#endif
    [ListEntitiesRequest::id] = [](int fd, void* data, int type, int id, int integer, int upper, std::string_view string) {
        struct ListEntitiesRequest* payload = (struct ListEntitiesRequest*)data;
        DumpStruct(payload);
        Send(fd, data, ListEntitiesDoneResponse::id,
                       Tag());
    },
#if HAVE_SOURCE_SERVER
    [ListEntitiesDoneResponse::id] = [](int fd, void* data, int type, int id, int integer, int upper, std::string_view string) {
        struct ListEntitiesDoneResponse* payload = (struct ListEntitiesDoneResponse*)data;
        DumpStruct(payload);
    },
#endif
    [SubscribeStatesRequest::id] = [](int fd, void* data, int type, int id, int integer, int upper, std::string_view string) {
        struct SubscribeStatesRequest* payload = (struct SubscribeStatesRequest*)data;
        DumpStruct(payload);
    },
    // ==================== SENSOR ====================
#if HAVE_SOURCE_SERVER
    [ListEntitiesSensorResponse::id] = [](int fd, void* data, int type, int id, int integer, int upper, std::string_view string) {
        struct ListEntitiesSensorResponse* payload = (struct ListEntitiesSensorResponse*)data;
        switch (id) {
        case 1:     payload->object_id = string;                                    break;
        case 2:     payload->key = integer;                                         break;
        case 3:     payload->name = string;                                         break;
        case 5:     payload->icon = string;                                         break;
        case 6:     payload->unit_of_measurement = string;                          break;
        case 7:     payload->accuracy_decimals = integer;                           break;
        case 8:     payload->force_update = integer;                                break;
        case 9:     payload->device_class = string;                                 break;
        case 10:    payload->state_class = SensorStateClass(integer);               break;
//      case 11:    payload->legacy_last_reset_type = SensorLastResetType(value);   break;
        case 12:    payload->disabled_by_default = integer;                         break;
        case 13:    payload->entity_category = EntityCategory(integer);             break;
//      case 14:    payload->device_id = integer;                                   break;
        case -1:    DumpStruct(payload);                                            break;
        }
    },
    [SensorStateResponse::id] = [](int fd, void* data, int type, int id, int integer, int upper, std::string_view string) {
        struct SensorStateResponse* payload = (struct SensorStateResponse*)data;
        switch (id) {
        case 1:     payload->key = integer;                                         break;
        case 2:     payload->state = CastFloat(integer);                            break;
        case 3:     payload->missing_state = integer;                               break;
//      case 4:     payload->device_id = integer;                                   break;
        case -1:    DumpStruct(payload);                                            break;
        }
    },
    // ==================== SWITCH ====================
    [ListEntitiesSwitchResponse::id] = [](int fd, void* data, int type, int id, int integer, int upper, std::string_view string) {
        struct ListEntitiesSwitchResponse* payload = (struct ListEntitiesSwitchResponse*)data;
        switch (id) {
        case 1:     payload->object_id = string;                                    break;
        case 2:     payload->key = integer;                                         break;
        case 3:     payload->name = string;                                         break;
        case 5:     payload->icon = string;                                         break;
        case 6:     payload->assumed_state = integer;                               break;
        case 7:     payload->disabled_by_default = integer;                         break;
        case 8:     payload->entity_category = EntityCategory(integer);             break;
        case 9:     payload->device_class = string;                                 break;
//      case 10:    payload->device_id = integer;                                   break;
        case -1:    DumpStruct(payload);                                            break;
        }
    },
    [SwitchStateResponse::id] = [](int fd, void* data, int type, int id, int integer, int upper, std::string_view string) {
        struct SwitchStateResponse* payload = (struct SwitchStateResponse*)data;
        switch (id) {
        case 1:     payload->key = integer;                                         break;
        case 2:     payload->state = integer;                                       break;
//      case 3:     payload->device_id = integer;                                   break;
        case -1:    DumpStruct(payload);                                            break;
        }
    },
    [SwitchCommandRequest::id] = [](int fd, void* data, int type, int id, int integer, int upper, std::string_view string) {
        struct SwitchCommandRequest* payload = (struct SwitchCommandRequest*)data;
        switch (id) {
        case 1:     payload->key = integer;                                         break;
        case 2:     payload->state = integer;                                       break;
//      case 3:     payload->device_id = integer;                                   break;
        case -1:    DumpStruct(payload);                                            break;
        }
    },
    // ==================== TEXT SENSOR ====================
    [ListEntitiesTextSensorResponse::id] = [](int fd, void* data, int type, int id, int integer, int upper, std::string_view string) {
        struct ListEntitiesTextSensorResponse* payload = (struct ListEntitiesTextSensorResponse*)data;
        switch (id) {
        case 1:     payload->object_id = string;                                    break;
        case 2:     payload->key = integer;                                         break;
        case 3:     payload->name = string;                                         break;
//      case 4:
        case 5:     payload->icon = string;                                         break;
        case 6:     payload->disabled_by_default = integer;                         break;
        case 7:     payload->entity_category = EntityCategory(integer);             break;
        case 8:     payload->device_class = string;                                 break;
//      case 9:     payload->device_id = integer;                                   break;
        case -1:    DumpStruct(payload);                                            break;
        }
    },
    [TextSensorStateResponse::id] = [](int fd, void* data, int type, int id, int integer, int upper, std::string_view string) {
        struct TextSensorStateResponse* payload = (struct TextSensorStateResponse*)data;
        switch (id) {
        case 1:     payload->key = integer;                                         break;
        case 2:     payload->state = string;                                        break;
        case 3:     payload->missing_state = integer;                               break;
//      case 4:     payload->device_id = integer;                                   break;
        case -1:    DumpStruct(payload);                                            break;
        }
    },
    // ==================== CLIMATE ====================
    [ListEntitiesClimateResponse::id] = [](int fd, void* data, int type, int id, int integer, int upper, std::string_view string) {
        struct ListEntitiesClimateResponse* payload = (struct ListEntitiesClimateResponse*)data;
        switch (id) {
        case 1:     payload->object_id = string;                                    break;
        case 2:     payload->key = integer;                                         break;
        case 3:     payload->name = string;                                         break;
//      case 4:
        case 5:     payload->supports_current_temperature = integer;                break;
        case 6:     payload->supports_two_point_target_temperature = integer;       break;
        case 7:     payload->supported_modes = ClimateMode(integer);                break;
        case 8:     payload->visual_min_temperature = CastFloat(integer);           break;
        case 9:     payload->visual_max_temperature = CastFloat(integer);           break;
        case 10:    payload->visual_target_temperature_step = CastFloat(integer);   break;
//      case 11:    payload->legacy_supports_away = integer;                        break;
//      case 12:    payload->supports_action = integer;                             break;
        case 13:    payload->supported_fan_modes = ClimateFanMode(integer);         break;
        case 14:    payload->supported_swing_modes = ClimateSwingMode(integer);     break;
//      case 15:    payload->supported_custom_fan_modes;                            break;
        case 16:    payload->supported_presets = ClimatePreset(integer);            break;
//      case 17:    payload->supported_custom_presets;                              break;
        case 18:    payload->disabled_by_default = integer;                         break;
        case 19:    payload->icon = string;                                         break;
        case 20:    payload->entity_category = EntityCategory(integer);             break;
        case 21:    payload->visual_current_temperature_step = CastFloat(integer);  break;
//      case 22:    payload->supports_current_humidity = integer;                   break;
//      case 23:    payload->supports_target_humidity = integer;                    break;
        case 24:    payload->visual_min_humidity = CastFloat(integer);              break;
        case 25:    payload->visual_max_humidity = CastFloat(integer);              break;
//      case 26:    payload->device_id = integer;                                   break;
        case 27:    payload->feature_flags = integer;                               break;
        case 28:    payload->temperature_unit = TemperatureUnit(integer);           break;
        case -1:    DumpStruct(payload);                                            break;
        }
    },
    [ClimateStateResponse::id] = [](int fd, void* data, int type, int id, int integer, int upper, std::string_view string) {
        struct ClimateStateResponse* payload = (struct ClimateStateResponse*)data;
        switch (id) {
        case 1:     payload->key = integer;                                         break;
        case 2:     payload->mode = ClimateMode(integer);                           break;
        case 3:     payload->current_temperature = CastFloat(integer);              break;
        case 4:     payload->target_temperature = CastFloat(integer);               break;
        case 5:     payload->target_temperature_low = CastFloat(integer);           break;
        case 6:     payload->target_temperature_high = CastFloat(integer);          break;
//      case 7:     payload->unused_legacy_away = integer;                          break;
        case 8:     payload->action = ClimateAction(integer);                       break;
        case 9:     payload->fan_mode = ClimateFanMode(integer);                    break;
        case 10:    payload->swing_mode = ClimateSwingMode(integer);                break;
        case 11:    payload->custom_fan_mode = string;                              break;
        case 12:    payload->preset = ClimatePreset(integer);                       break;
        case 13:    payload->custom_preset = string;                                break;
        case 14:    payload->current_humidity = CastFloat(integer);                 break;
        case 15:    payload->target_humidity = CastFloat(integer);                  break;
//      case 16:    payload->device_id = integer;                                   break;
        case -1:    DumpStruct(payload);                                            break;
        }
    },
#endif
    [ClimateCommandRequest::id] = [](int fd, void* data, int type, int id, int integer, int upper, std::string_view string) {
        struct ClimateCommandRequest* payload = (struct ClimateCommandRequest*)data;
        switch (id) {
        case 1:     payload->key = integer;                                         break;
        case 2:     payload->has_mode = integer;                                    break;
        case 3:     payload->mode = ClimateMode(integer);                           break;
        case 4:     payload->has_target_temperature = integer;                      break;
        case 5:     payload->target_temperature = CastFloat(integer);               break;
        case 6:     payload->has_target_temperature_low = integer;                  break;
        case 7:     payload->target_temperature_low = CastFloat(integer);           break;
        case 8:     payload->has_target_temperature_high = integer;                 break;
        case 9:     payload->target_temperature_high = CastFloat(integer);          break;
//      case 10:    payload->unused_has_legacy_away = integer;                      break;
//      case 11:    payload->unused_legacy_away = integer;                          break;
        case 12:    payload->has_fan_mode = integer;                                break;
        case 13:    payload->fan_mode = ClimateFanMode(integer);                    break;
        case 14:    payload->has_swing_mode = integer;                              break;
        case 15:    payload->swing_mode = ClimateSwingMode(integer);                break;
        case 16:    payload->has_custom_fan_mode = integer;                         break;
        case 17:    payload->custom_fan_mode = string;                              break;
        case 18:    payload->has_preset = integer;                                  break;
        case 19:    payload->preset = ClimatePreset(integer);                       break;
        case 20:    payload->has_custom_preset = integer;                           break;
        case 21:    payload->custom_preset = string;                                break;
        case 22:    payload->has_target_humidity = integer;                         break;
        case 23:    payload->target_humidity = CastFloat(integer);                  break;
//      case 24:    payload->device_id = integer;                                   break;
        case -1:    DumpStruct(payload);                                            break;
        }
    },
};

};
