// SPDX-License-Identifier: MIT
#pragma once

namespace ESPHome {
namespace Server {

extern void Broadcast(int type, va_list va);
extern void Dispatch(int type, int fd, const void* data);
extern void Poll(void* args = nullptr);
extern bool Start(void(*dispatch)(int type, int fd, const void* data) = Dispatch);
extern void Stop();

};
};
