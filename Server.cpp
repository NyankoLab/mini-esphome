// SPDX-License-Identifier: MIT
#include <unistd.h>
#include <netinet/in.h>
#include <sys/fcntl.h>
#include <sys/poll.h>
#include <sys/socket.h>

#include "API.h"
#include "Message.h"
#include "Server.h"

#define TAG "ESPHome"

namespace ESPHome {
namespace Server {

static bool stop = true;
static int server_fd = -1;
static int pollfds_count = 0;
static constexpr int pollfds_capacity = 4;
static struct pollfd pollfds[pollfds_capacity];

void Broadcast(int type, ...)
{
    for (int i = 0; i < pollfds_count; ++i) {
        auto& pollfd = pollfds[i];
        if (pollfd.fd == server_fd)
            continue;
        va_list va;
        va_start(va, type);
        ESPHome::SendV(pollfd.fd, type, va);
        va_end(va);
    }
}

void Dispatch(int type, int fd, const void* data)
{
    switch (type) {
    case ESPHome::ListEntitiesRequest::id:
        ESPHome::Send(fd, ListEntitiesClimateResponse::id,
                          Tag(1, LEN), Text("0"),
                          Tag(2, I32), 0,
                          Tag(3, LEN), Text("Climate"),
                          Tag(7, VARINT), ESPHome::CLIMATE_MODE_OFF,
                          Tag(7, VARINT), ESPHome::CLIMATE_MODE_COOL,
                          Tag(7, VARINT), ESPHome::CLIMATE_MODE_HEAT,
                          Tag(7, VARINT), ESPHome::CLIMATE_MODE_FAN_ONLY,
                          Tag(7, VARINT), ESPHome::CLIMATE_MODE_DRY,
                          Tag(7, VARINT), ESPHome::CLIMATE_MODE_AUTO,
                          Tag(8, I32), ESPHome::CastInt(16.0f),
                          Tag(9, I32), ESPHome::CastInt(32.0f),
                          Tag(10, I32), ESPHome::CastInt(1.0f),
                          Tag(21, I32), ESPHome::CastInt(1.0f),
                          Tag());
        ESPHome::Send(fd, ListEntitiesDoneResponse::id,
                          Tag());
        break;
    case ESPHome::SubscribeStatesRequest::id:
        ESPHome::Send(fd, ClimateStateResponse::id,
                          Tag(1, I32), 0,
                          Tag(2, VARINT), ESPHome::CLIMATE_MODE_OFF,
                          Tag(3, I32), ESPHome::CastInt(28.0f),
                          Tag(4, I32), ESPHome::CastInt(28.0f),
                          Tag());
        break;
    default:
        break;
    }
};

bool Start(void(*dispatch)(int type, int fd, const void* data))
{
    if (stop == false)
        return false;
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    switch (0) default: {
        if (fd < 0) {
            printf("%s : %s (%d)\n", "socket", strerror(errno), errno);
            break;
        }
        int value = 1;
        setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &value, sizeof(int));
        struct sockaddr_in sockaddr = {};
        sockaddr.sin_family = PF_INET;
        sockaddr.sin_addr.s_addr = 0;
        sockaddr.sin_port = htons(6053);
        if (bind(fd, (struct sockaddr*)&sockaddr, sizeof(struct sockaddr_in)) != 0) {
            printf("%s : %s (%d)\n", "bind", strerror(errno), errno);
            break;
        }
        if (listen(fd, 4) != 0) {
            printf("%s : %s (%d)\n", "listen", strerror(errno), errno);
            break;
        }
        stop = false;
        server_fd = fd;
        printf("%s : %d is listen\n", TAG, fd);
        fcntl(fd, F_SETFL, O_NONBLOCK | fcntl(fd, F_GETFL, 0));
        pollfds[pollfds_count++] = { fd, POLLSTANDARD & ~POLLOUT };
        ESPHome::Dispatch = dispatch;
        return true;
    }
    if (fd >= 0) {
        close(fd);
    }
    return false;
}

void Stop()
{
    ESPHome::Dispatch = [](int, int, const void*){};
    stop = true;
    server_fd = -1;
    for (int i = 0; i < pollfds_count; ++i)
        close(pollfds[i].fd);
    pollfds_count = 0;
}

void Poll()
{
    while (stop == false && pollfds_count) {
        int count = poll(pollfds, pollfds_count, -1);
        if (count == 0)
            continue;
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
                            printf("%s : %d is close\n", TAG, pollfds[1].fd);
                            for (int i = 2; i < pollfds_count; ++i)
                                pollfds[i - 1] = pollfds[i];
                            pollfds_count--;
                        }
                        printf("%s : %d is accept\n", TAG, fd);
                        fcntl(fd, F_SETFL, O_NONBLOCK | fcntl(fd, F_GETFL, 0));
                        pollfds[pollfds_count++] = { fd, POLLSTANDARD & ~POLLOUT };
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
                    if (recv(pollfd.fd, &c, 1, 0) != 1)
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
                    int length = ESPHome::LengthMessage(header, nullptr, &offset);
                    if (length) {
                        char* buffer = (char*)malloc(length);
                        if (buffer) {
                            memcpy(buffer, header, count);
                            if (recv(pollfd.fd, buffer + count, length - count, 0) == length - count) {
                                ESPHome::Recv(pollfd.fd, buffer, length);
                                revents &= ~POLLHUP;
                            }
                            free(buffer);
                        }
                    } else {
                        ESPHome::Recv(pollfd.fd, header, count);
                        revents &= ~POLLHUP;
                    }
                }
            }
            if (revents & (POLLERR | POLLHUP | POLLNVAL)) {
                close(pollfd.fd);
                printf("%s : %d is close\n", TAG, pollfd.fd);
                pollfds[i] = pollfds[--pollfds_count];
                break;
            }
        }
    }
}

};
};
