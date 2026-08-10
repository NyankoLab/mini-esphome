// SPDX-License-Identifier: MIT
#include <stdlib.h>
#include <stdio.h>

#include "ESPHomeServer.h"

int main(int argc, const char* argv[])
{
    ESPHome::Server::Start();
    ESPHome::Server::Poll();
    ESPHome::Server::Stop();
    return EXIT_SUCCESS;
}
