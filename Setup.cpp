// SPDX-License-Identifier: MIT
#include <stdint.h>
#include <stdio.h>
#include "Setup.h"

#if defined(__ESP__)
#include <esp_wifi.h>
#endif

namespace ESPHome {
namespace Setup {

char Name[31];
char Model[127];
char Manufacturer[20];
char FriendlyName[120];

const char* GetMacAddress()
{
#if defined(__ESP__)
    static char MacAddress[18];
    uint8_t macaddr[6] = {};
    esp_wifi_get_mac(WIFI_IF_STA, macaddr);
    snprintf(MacAddress, 18, "%02X:%02X:%02X:%02X:%02X:%02X", macaddr[0], macaddr[1], macaddr[2], macaddr[3], macaddr[4], macaddr[5]);
    return MacAddress;
#else
    return "00:00:00:00:00:00";
#endif
}

};
};
