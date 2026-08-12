// SPDX-License-Identifier: MIT
#pragma once

#if defined(__ESP__)
#include <stdio.h>
#include <esp_log.h>
#define accept lwip_accept
#define bind lwip_bind
#define close lwip_close
#define fcntl lwip_fcntl
#define listen lwip_listen
#define poll lwip_poll
#define printf(...) ESP_LOGI("esphome", __VA_ARGS__)
#define recv lwip_recv
#define send lwip_send
#define setsockopt lwip_setsockopt
#define socket lwip_socket
#define ESPHOME_LF ""
#else
#define ESPHOME_LF "\n"
#endif

namespace ESPHome {
namespace Setup {

extern char Name[31];
extern char Model[127];
extern char Manufacturer[20];
extern char FriendlyName[120];

const char* GetMacAddress();

};
};
