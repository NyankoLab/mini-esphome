// SPDX-License-Identifier: MIT
#pragma once

namespace ESPHome {
namespace Server {

extern int Accept(int fd, int revents);
extern void Broadcast(int type, va_list va);
extern void Dispatch(int type, int fd, const void* data);
extern void Poll(void* args = nullptr);
extern int Recv(int fd, int revents);
extern int Start(void(*dispatch)(int type, int fd, const void* data) = Dispatch);
extern void Stop();

};
};
