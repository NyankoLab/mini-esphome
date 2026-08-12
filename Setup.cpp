// SPDX-License-Identifier: MIT
#include "Setup.h"

namespace ESPHome {
namespace Setup {

char Name[31];
char Model[127];
char Manufacturer[20];
char FriendlyName[120];

const char* GetMacAddress()
{
    return "00:00:00:00:00:00";
}

};
};
