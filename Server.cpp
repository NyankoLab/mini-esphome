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
#if HAVE_MPOLL
typedef int (*mpoll_callback)(int, int);
extern "C" void mpoll_ctl(int fd, mpoll_callback callback);
extern "C" void mpoll_wait(int timeout);
#else
#define ESPHOME_STACK_SIZE 3072
#endif
#endif

namespace ESPHome {
namespace Server {

static bool stop = true;
static int server_fd = -1;
static int pollfds_count = 0;
static constexpr int pollfds_capacity = 4;
static struct pollfd pollfds[pollfds_capacity];

int Accept(int fd, int revents)
{
    if (revents & POLLIN)
    {
        // Limit connections
        if (pollfds_count >= pollfds_capacity) {
            close(pollfds[1].fd);
            println("%d : %s", pollfds[1].fd, "close");
#if HAVE_MPOLL
            mpoll_ctl(pollfds[1].fd, nullptr);
#endif
            for (int i = 2; i < pollfds_count; ++i)
                pollfds[i - 1] = pollfds[i];
            pollfds_count--;
        }

        struct sockaddr_storage sockaddr = {};
        socklen_t length = sizeof(struct sockaddr_storage);
        int accpet_fd = accept(fd, (struct sockaddr*)&sockaddr, &length);
        if (accpet_fd >= 0) {
            println("%d : %s", accpet_fd, "accept");
            pollfds[pollfds_count++] = { accpet_fd, POLLIN | POLLERR | POLLHUP | POLLNVAL };
#if HAVE_MPOLL
            mpoll_ctl(accpet_fd, Recv);
#endif
        }
    }
    return revents;
}

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

#if HAVE_MPOLL == 0
void Poll(void* args)
{
    while (stop == false && pollfds_count) {
        int count = poll(pollfds, pollfds_count, INT_MAX);
        if (count == 0) {
            println("%s : %s", "poll", "timeout");
            continue;
        }
        for (int i = 0; i < pollfds_count; ++i) {
            auto& pollfd = pollfds[i];
            int revents = pollfd.revents;
            pollfd.revents = 0;
            if (revents & POLLIN) {
                if (pollfd.fd == server_fd) {
                    revents = Accept(pollfd.fd, revents);
                    break;
                } else {
                    revents = Recv(pollfd.fd, revents);
                }
            }
            if (revents & (POLLERR | POLLHUP | POLLNVAL)) {
                close(pollfd.fd);
                println("%d : %s", pollfd.fd, "close");
                pollfds[i] = pollfds[--pollfds_count];
                break;
            }
        }
    }
}
#endif

int Recv(int fd, int revents)
{
    if (revents & POLLIN)
    {
        // Header
        char header[6] = {};
        int count = 0;
        int field = 0;
        for (int i = 0; i < 6; ++i) {
            char c = 0;
            if (recv(fd, &c, 1, MSG_DONTWAIT) != 1)
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
                    if (recv(fd, buffer + count, length - count, MSG_DONTWAIT) == length - count) {
                        API::Recv(fd, buffer, length);
                        revents &= ~POLLHUP;
                    }
                    free(buffer);
                } else {
                    println("%d : %s", fd, "out of memory");
                }
            } else {
                API::Recv(fd, header, count);
                revents &= ~POLLHUP;
            }
        }
    }
    return revents;
}

int Start(void(*dispatch)(int type, int fd, const void* data))
{
    if (stop == false)
        return -1;
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    switch (0) default: {
        if (fd < 0) {
            println("%s : %s (%d)", "socket", strerror(errno), errno);
            break;
        }
        int value = 1;
        setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &value, sizeof(int));
        struct sockaddr_in sockaddr = {};
        sockaddr.sin_len = sizeof(struct sockaddr_in);
        sockaddr.sin_family = PF_INET;
        sockaddr.sin_addr.s_addr = 0;
        sockaddr.sin_port = htons(6053);
        if (bind(fd, (struct sockaddr*)&sockaddr, sizeof(struct sockaddr_in)) != 0) {
            println("%s : %s (%d)", "bind", strerror(errno), errno);
            break;
        }
        if (listen(fd, 4) != 0) {
            println("%s : %s (%d)", "listen", strerror(errno), errno);
            break;
        }
        API::Dispatch = dispatch;
        stop = false;
        server_fd = fd;
        println("%d : %s", fd, "listen");
        pollfds[pollfds_count++] = { fd, POLLIN | POLLERR | POLLHUP | POLLNVAL };
#if HAVE_MPOLL
        mpoll_ctl(fd, Accept);
#elif defined(__ESP__)
        xTaskCreate(Poll, "esphome", ESPHOME_STACK_SIZE, nullptr, tskIDLE_PRIORITY, nullptr);
#endif
        return fd;
    }
    if (fd >= 0) {
        close(fd);
    }
    return -1;
}

void Stop()
{
    API::Dispatch = [](int, int, const void*){};
    stop = true;
    server_fd = -1;
    for (int i = 0; i < pollfds_count; ++i) {
        close(pollfds[i].fd);
#if HAVE_MPOLL
        mpoll_ctl(pollfds[1].fd, nullptr);
#endif
    }
    pollfds_count = 0;
}

};
};
