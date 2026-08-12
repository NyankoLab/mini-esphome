// SPDX-License-Identifier: MIT
#pragma once

namespace ESPHome {
namespace Setup {

extern char Name[31];
extern char Model[127];
extern char Manufacturer[20];
extern char FriendlyName[120];

const char* GetMacAddress();

};
};
