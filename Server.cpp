// SPDX-License-Identifier: MIT
#include <unistd.h>
#include <netinet/in.h>
#include <sys/fcntl.h>
#include <sys/poll.h>
#include <sys/socket.h>

#include "API.h"
#include "Message.h"
#include "Server.h"
#include "Setup.h"

#if defined(__ESP__)
#include <freertos/task.h>
#define ESPHOME_STACK_SIZE 3072
#endif

namespace ESPHome {
namespace Server {

static bool stop = true;
static int server_fd = -1;
static int pollfds_count = 0;
static constexpr int pollfds_capacity = 4;
static struct pollfd pollfds[pollfds_capacity];

void Broadcast(int type, va_list va)
{
    for (int i = 0; i < pollfds_count; ++i) {
        auto& pollfd = pollfds[i];
        if (pollfd.fd == server_fd)
            continue;
        API::Send(pollfd.fd, type, va);
    }
}

void Dispatch(int type, int fd, const void* data)
{
}

void Poll(void* args)
{
    while (stop == false && pollfds_count) {
        int count = poll(pollfds, pollfds_count, INT_MAX);
        if (count == 0) {
            printf("%s : %s" ESPHOME_LF, "poll", "timeout");
            continue;
        }
        for (int i = 0; i < pollfds_count; ++i) {
            auto& pollfd = pollfds[i];
            int revents = pollfd.revents;
            pollfd.revents = 0;
            if (revents & POLLIN) {

                // Server
                if (pollfd.fd == server_fd) {
                    struct sockaddr_storage sockaddr = {};
                    socklen_t length = sizeof(struct sockaddr_storage);
                    int fd = accept(pollfd.fd, (struct sockaddr*)&sockaddr, &length);
                    if (fd >= 0) {
                        if (pollfds_count >= pollfds_capacity) {
                            close(pollfds[1].fd);
                            printf("%d : %s" ESPHOME_LF, pollfds[1].fd, "close");
                            for (int i = 2; i < pollfds_count; ++i)
                                pollfds[i - 1] = pollfds[i];
                            pollfds_count--;
                        }
                        printf("%d : %s" ESPHOME_LF, fd, "accept");
                        pollfds[pollfds_count++] = { fd, POLLIN | POLLERR | POLLHUP | POLLNVAL };
                        break;
                    }
                    continue;
                }

                // Header
                char header[6] = {};
                int count = 0;
                int field = 0;
                for (int i = 0; i < 6; ++i) {
                    char c = 0;
                    if (recv(pollfd.fd, &c, 1, MSG_DONTWAIT) != 1)
                        break;
                    header[i] = c;
                    count++;
                    if ((c & 0x80) == 0)
                        field++;
                    if (field == 3)
                        break;
                }

                // Message
                revents |= POLLHUP;
                if (count >= 3) {
                    int offset = 0;
                    int length = Message::Length(header, nullptr, &offset);
                    if (length > count) {
                        char* buffer = (char*)malloc(length);
                        if (buffer) {
                            memcpy(buffer, header, count);
                            if (recv(pollfd.fd, buffer + count, length - count, MSG_DONTWAIT) == length - count) {
                                API::Recv(pollfd.fd, buffer, length);
                                revents &= ~POLLHUP;
                            }
                            free(buffer);
                        } else {
                            printf("%d : %s" ESPHOME_LF, pollfd.fd, "out of memory");
                        }
                    } else {
                        API::Recv(pollfd.fd, header, count);
                        revents &= ~POLLHUP;
                    }
                }
            }
            if (revents & (POLLERR | POLLHUP | POLLNVAL)) {
                close(pollfd.fd);
                printf("%d : %s" ESPHOME_LF, pollfd.fd, "close");
                pollfds[i] = pollfds[--pollfds_count];
                break;
            }
        }
    }
}

bool Start(void(*dispatch)(int type, int fd, const void* data))
{
    if (stop == false)
        return false;
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    switch (0) default: {
        if (fd < 0) {
            printf("%s : %s (%d)" ESPHOME_LF, "socket", strerror(errno), errno);
            break;
        }
        int value = 1;
        setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &value, sizeof(int));
        struct sockaddr_in sockaddr = {};
        sockaddr.sin_family = PF_INET;
        sockaddr.sin_addr.s_addr = 0;
        sockaddr.sin_port = htons(6053);
        if (bind(fd, (struct sockaddr*)&sockaddr, sizeof(struct sockaddr_in)) != 0) {
            printf("%s : %s (%d)" ESPHOME_LF, "bind", strerror(errno), errno);
            break;
        }
        if (listen(fd, 4) != 0) {
            printf("%s : %s (%d)" ESPHOME_LF, "listen", strerror(errno), errno);
            break;
        }
        stop = false;
        server_fd = fd;
        printf("%d : %s" ESPHOME_LF, fd, "listen");
        pollfds[pollfds_count++] = { fd, POLLIN | POLLERR | POLLHUP | POLLNVAL };
        API::Dispatch = dispatch;
#if defined(__ESP__)
        xTaskCreate(Poll, "esphome", ESPHOME_STACK_SIZE, nullptr, tskIDLE_PRIORITY, nullptr);
#endif
        return true;
    }
    if (fd >= 0) {
        close(fd);
    }
    return false;
}

void Stop()
{
    API::Dispatch = [](int, int, const void*){};
    stop = true;
    server_fd = -1;
    for (int i = 0; i < pollfds_count; ++i)
        close(pollfds[i].fd);
    pollfds_count = 0;
}

};
};
