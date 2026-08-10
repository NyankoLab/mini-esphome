// SPDX-License-Identifier: MIT
#include <unistd.h>
#include <netinet/in.h>
#include <sys/fcntl.h>
#include <sys/poll.h>
#include <sys/socket.h>
#include <vector>

#include "ESPHomeAPI.h"
#include "ESPHomeMessage.h"
#include "ESPHomeServer.h"

#define TAG "ESPHome Server"

namespace ESPHome {
namespace Server {

static bool stop = false;
static int server_fd = -1;
static std::vector<struct pollfd> pollfds;

bool Start()
{
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
        printf("%s : %d is listen\n", TAG, fd);
        server_fd = fd;
        pollfds.emplace_back(fd, POLLSTANDARD);
        return true;
    }
    if (fd >= 0) {
        close(fd);
    }
    return false;
}

void Stop()
{
    server_fd = -1;
    for (auto& pollfd : pollfds) {
        close(pollfd.fd);
    }
    pollfds = std::vector<struct pollfd>();
}

void Poll()
{
    if (pollfds.size() == 0)
        return;
    while (stop == false) {
        int count = poll(pollfds.data(), (int)pollfds.size(), -1);
        if (count == 0)
            continue;
        for (auto& pollfd : pollfds) {
            int revents = pollfd.revents;
            pollfd.revents = 0;
            if (revents & POLLIN) {
                if (pollfd.fd == server_fd) {
                    struct sockaddr_storage sockaddr = {};
                    socklen_t length = sizeof(struct sockaddr_storage);
                    int fd = accept(pollfd.fd, (struct sockaddr*)&sockaddr, &length);
                    if (fd >= 0) {
//                      fcntl(fd, F_SETFL, O_NONBLOCK | fcntl(fd, F_GETFL, 0));
                        pollfds.emplace_back(fd, POLLSTANDARD);
                        printf("%s : %d is accept\n", TAG, fd);
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

                bool finish = false;
                if (count >= 3) {
                    int offset = 0;
                    int length = ESPHome::LengthMessage(header, nullptr, &offset);
                    if (length) {
                        char* buffer = (char*)malloc(length + count);
                        if (buffer) {
                            memcpy(buffer, header, count);
                            if (recv(pollfd.fd, buffer + count, length - count, 0) == length - count) {
                                ESPHome::Recv(pollfd.fd, buffer, length);
                                finish = true;
                            }
                            free(buffer);
                        }
                    }
                    else {
                        ESPHome::Recv(pollfd.fd, header, count);
                        finish = true;
                    }
                }
                if (finish == false) {
                    revents |= POLLHUP;
                }
            }
            if (revents & POLLOUT) {
                
            }
            if (revents & (POLLERR | POLLHUP)) {
                close(pollfd.fd);
                printf("%s : %d is close\n", TAG, pollfd.fd);
                pollfd.fd = pollfds.back().fd;
                pollfds.pop_back();
                break;
            }
        }
    }
}

};
};
