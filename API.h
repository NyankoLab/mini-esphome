// SPDX-License-Identifier: MIT
#pragma once

#include <stdarg.h>
#include <stdint.h>
#include <string.h>
#include <string_view>
#include <vector>

namespace ESPHome {

enum DisconnectReason {
    DISCONNECT_REASON_UNSPECIFIED = 0,
    DISCONNECT_REASON_PROVISIONING_CLOSED,
};

struct HelloRequest { static constexpr int id = 1;
/* 1  */std::string_view client_info;
/* 2  */uint32_t api_version_major;
/* 3  */uint32_t api_version_minor;
};

struct HelloResponse { static constexpr int id = 2;
/* 1  */uint32_t api_version_major;
/* 2  */uint32_t api_version_minor;
/* 3  */std::string_view server_info;
/* 4  */std::string_view name;
};

struct AuthenticationRequest { static constexpr int id = 3;
/* 1  */std::string_view password;
};

struct AuthenticationResponse { static constexpr int id = 4;
/* 1  */bool invalid_password;
};

struct DisconnectRequest { static constexpr int id = 5;
/* 1  */DisconnectReason reason;
};

struct DisconnectResponse { static constexpr int id = 6;
};

struct PingRequest { static constexpr int id = 7;
};

struct PingResponse { static constexpr int id = 8;
};

struct DeviceInfoRequest { static constexpr int id = 9;
};

struct DeviceInfoResponse { static constexpr int id = 10;
// 1  */bool uses_password;
/* 2  */std::string_view name;
/* 3  */std::string_view mac_address;
/* 4  */std::string_view esphome_version;
/* 5  */std::string_view compilation_time;
/* 6  */std::string_view model;
// 7  */bool has_deep_sleep;
// 8  */std::string_view project_name;
// 9  */std::string_view project_version;
// 10 */uint32_t webserver_port;
// 11 */uint32_t legacy_bluetooth_proxy_version;
// 15 */uint32_t bluetooth_proxy_feature_flags;
/* 12 */std::string_view manufacturer;
/* 13 */std::string_view friendly_name;
// 14 */uint32_t legacy_voice_assistant_version;
// 17 */uint32_t voice_assistant_feature_flags;
// 16 */std::string_view suggested_area;
// 18 */std::string_view bluetooth_mac_address;
// 19 */bool api_encryption_supported;
// 20 */std::vector<DeviceInfo> devices;
// 21 */std::vector<AreaInfo> areas;
// 22 */AreaInfo area;
// 23 */uint32_t zwave_proxy_feature_flags;
// 24 */uint32_t zwave_home_id;
// 25 */std::vector<SerialProxyInfo> serial_proxies;
// 26 */bool api_encryption_provisionable;
};

struct ListEntitiesRequest { static constexpr int id = 11;
};

struct ListEntitiesDoneResponse { static constexpr int id = 19;
};

struct SubscribeStatesRequest { static constexpr int id = 20;
};

// ==================== COMMON =====================
enum EntityCategory {
    ENTITY_CATEGORY_NONE = 0,
    ENTITY_CATEGORY_CONFIG,
    ENTITY_CATEGORY_DIAGNOSTIC,
};

// ==================== BINARY SENSOR ====================
// ==================== COVER ====================
// ==================== FAN ====================
// ==================== LIGHT ====================
// ==================== SENSOR ====================
enum SensorStateClass {
    STATE_CLASS_NONE = 0,
    STATE_CLASS_MEASUREMENT,
    STATE_CLASS_TOTAL_INCREASING,
    STATE_CLASS_TOTAL,
    STATE_CLASS_MEASUREMENT_ANGLE,
};

struct ListEntitiesSensorResponse { static constexpr int id = 16;
/* 1  */std::string_view object_id;
/* 2  */uint32_t key;
/* 3  */std::string_view name;
// 4  */reserved
/* 5  */std::string_view icon;
/* 6  */std::string_view unit_of_measurement;
/* 7  */int32_t accuracy_decimals;
/* 8  */bool force_update;
/* 9  */std::string_view device_class;
/* 10 */SensorStateClass state_class;
// 11 */SensorLastResetType legacy_last_reset_type;
/* 12 */bool disabled_by_default;
/* 13 */EntityCategory entity_category;
// 14 */uint32_t device_id;
};

struct SensorStateResponse { static constexpr int id = 25;
/* 1  */uint32_t key;
/* 2  */float state;
/* 3  */bool missing_state;
// 4  */uint32_t device_id;
};

// ==================== SWITCH ====================
struct ListEntitiesSwitchResponse { static constexpr int id = 17;
/* 1  */std::string_view object_id;
/* 2  */uint32_t key;
/* 3  */std::string_view name;
// 4  */reserved
/* 5  */std::string_view icon;
/* 6  */bool assumed_state;
/* 7  */bool disabled_by_default;
/* 8  */EntityCategory entity_category;
/* 9  */std::string_view device_class;
// 10 */uint32_t device_id;
};

struct SwitchStateResponse { static constexpr int id = 26;
/* 1  */uint32_t key;
/* 2  */bool state;
// 3  */uint32_t device_id;
};

struct SwitchCommandRequest { static constexpr int id = 33;
/* 1  */uint32_t key;
/* 2  */bool state;
// 3  */uint32_t device_id;
};

// ==================== TEXT SENSOR ====================
struct ListEntitiesTextSensorResponse { static constexpr int id = 18;
/* 1  */std::string_view object_id;
/* 2  */uint32_t key;
/* 3  */std::string_view name;
// 4  */reserved
/* 5  */std::string_view icon;
/* 6  */bool disabled_by_default;
/* 7  */EntityCategory entity_category;
/* 8  */std::string_view device_class;
// 9  */uint32_t device_id;
};

struct TextSensorStateResponse { static constexpr int id = 27;
/* 1  */uint32_t key;
/* 2  */std::string_view state;
/* 3  */bool missing_state;
// 4  */uint32_t device_id;
};

// ==================== CAMERA ====================
// ==================== TEMPERATURE UNIT ====================
enum TemperatureUnit {
    TEMPERATURE_UNIT_CELSIUS = 0,
    TEMPERATURE_UNIT_FAHRENHEIT,
    TEMPERATURE_UNIT_KELVIN,
};

// ==================== CLIMATE ====================
enum ClimateMode {
    CLIMATE_MODE_OFF = 0,
    CLIMATE_MODE_HEAT_COOL,
    CLIMATE_MODE_COOL,
    CLIMATE_MODE_HEAT,
    CLIMATE_MODE_FAN_ONLY,
    CLIMATE_MODE_DRY,
    CLIMATE_MODE_AUTO,
};

enum ClimateFanMode {
    CLIMATE_FAN_ON = 0,
    CLIMATE_FAN_OFF,
    CLIMATE_FAN_AUTO,
    CLIMATE_FAN_LOW,
    CLIMATE_FAN_MEDIUM,
    CLIMATE_FAN_HIGH,
    CLIMATE_FAN_MIDDLE,
    CLIMATE_FAN_FOCUS,
    CLIMATE_FAN_DIFFUSE,
    CLIMATE_FAN_QUIET,
};

enum ClimateSwingMode {
    CLIMATE_SWING_OFF = 0,
    CLIMATE_SWING_BOTH,
    CLIMATE_SWING_VERTICAL,
    CLIMATE_SWING_HORIZONTAL,
};

enum ClimateAction {
    CLIMATE_ACTION_OFF = 0,
    // values same as mode for readability
    CLIMATE_ACTION_COOLING = 2,
    CLIMATE_ACTION_HEATING,
    CLIMATE_ACTION_IDLE,
    CLIMATE_ACTION_DRYING,
    CLIMATE_ACTION_FAN,
    CLIMATE_ACTION_DEFROSTING,
};

enum ClimatePreset {
    CLIMATE_PRESET_NONE = 0,
    CLIMATE_PRESET_HOME,
    CLIMATE_PRESET_AWAY,
    CLIMATE_PRESET_BOOST,
    CLIMATE_PRESET_COMFORT,
    CLIMATE_PRESET_ECO,
    CLIMATE_PRESET_SLEEP,
    CLIMATE_PRESET_ACTIVITY,
};

struct ListEntitiesClimateResponse { static constexpr int id = 46;
/* 1  */std::string_view object_id;
/* 2  */uint32_t key;
/* 3  */std::string_view name;
// 4  */reserved
// 5  */bool supports_current_temperature;
// 6  */bool supports_two_point_target_temperature;
/* 7  */std::vector<ClimateMode> supported_modes;
/* 8  */float visual_min_temperature;
/* 9  */float visual_max_temperature;
/* 10 */float visual_target_temperature_step;
// 11 */bool legacy_supports_away;
// 12 */bool supports_action;
/* 13 */std::vector<ClimateFanMode> supported_fan_modes;
/* 14 */std::vector<ClimateSwingMode> supported_swing_modes;
// 15 */std::vector<std::string_view> supported_custom_fan_modes;
/* 16 */std::vector<ClimatePreset> supported_presets;
// 17 */std::vector<std::string_view> supported_custom_presets;
/* 18 */bool disabled_by_default;
/* 19 */std::string_view icon;
/* 20 */EntityCategory entity_category;
/* 21 */float visual_current_temperature_step;
// 22 */bool supports_current_humidity;
// 23 */bool supports_target_humidity;
/* 24 */float visual_min_humidity;
/* 25 */float visual_max_humidity;
// 26 */uint32_t device_id;
/* 27 */uint32_t feature_flags;
/* 28 */TemperatureUnit temperature_unit;
};

struct ClimateStateResponse { static constexpr int id = 47;
/* 1  */uint32_t key;
/* 2  */ClimateMode mode;
/* 3  */float current_temperature;
/* 4  */float target_temperature;
/* 5  */float target_temperature_low;
/* 6  */float target_temperature_high;
// 7  */bool unused_legacy_away;
/* 8  */ClimateAction action;
/* 9  */ClimateFanMode fan_mode;
/* 10 */ClimateSwingMode swing_mode;
/* 11 */std::string_view custom_fan_mode;
/* 12 */ClimatePreset preset;
/* 13 */std::string_view custom_preset;
/* 14 */float current_humidity;
/* 15 */float target_humidity;
// 16 */uint32_t device_id;
};

struct ClimateCommandRequest { static constexpr int id = 48;
/* 1  */uint32_t key;
/* 2  */bool has_mode;
/* 3  */ClimateMode mode;
/* 4  */bool has_target_temperature;
/* 5  */float target_temperature;
/* 6  */bool has_target_temperature_low;
/* 7  */float target_temperature_low;
/* 8  */bool has_target_temperature_high;
/* 9  */float target_temperature_high;
// 10 */bool unused_has_legacy_away;
// 11 */bool unused_legacy_away;
/* 12 */bool has_fan_mode;
/* 13 */ClimateFanMode fan_mode;
/* 14 */bool has_swing_mode;
/* 15 */ClimateSwingMode swing_mode;
/* 16 */bool has_custom_fan_mode;
/* 17 */std::string_view custom_fan_mode;
/* 18 */bool has_preset;
/* 19 */ClimatePreset preset;
/* 20 */bool has_custom_preset;
/* 21 */std::string_view custom_preset;
/* 22 */bool has_target_humidity;
/* 23 */float target_humidity;
// 24 */uint32_t device_id;
};

// ==================== WATER_HEATER ====================
// ==================== NUMBER ====================
// ==================== SELECT ====================
// ==================== SIREN ====================
// ==================== LOCK ====================
// ==================== BUTTON ====================
// ==================== MEDIA PLAYER ====================
// ==================== BLUETOOTH ====================
// ==================== VOICE ASSISTANT ====================
// ==================== ALARM CONTROL PANEL ====================
// ===================== TEXT =====================
// ==================== DATETIME DATE ====================
// ==================== DATETIME TIME ====================
// ==================== EVENT ====================
// ==================== VALVE ====================
// ==================== DATETIME DATETIME ====================
// ==================== UPDATE ====================
// ==================== Z-WAVE ====================
// ==================== INFRARED ====================
// ==================== RADIO FREQUENCY ====================
// ==================== SERIAL PROXY ====================
// ==================== BLUETOOTH CONNECTION PARAMS ====================

extern void(*Dispatch)(int fd, int type, const void* data);
extern void Send(int fd, int type, ...);
extern void SendV(int fd, int type, va_list va);
extern void Recv(int fd, const char* buffer, int length);

};
