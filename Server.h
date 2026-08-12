// SPDX-License-Identifier: MIT
#pragma once

namespace ESPHome {
namespace Server {

extern void Broadcast(int type, ...);
extern void Dispatch(int type, int fd, const void* data);

extern bool Start(void(*callback)(int type, int fd, const void* data) = Dispatch);
extern void Stop();
extern void Poll();

};
};
