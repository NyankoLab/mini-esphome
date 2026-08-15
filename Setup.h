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
#define recv lwip_recv
#define send lwip_send
#define setsockopt lwip_setsockopt
#define socket lwip_socket
#define println(format, ...) ESP_LOGI("esphome", format, __VA_ARGS__)
#else
#define println(format, ...) printf(format "\n", __VA_ARGS__)
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
