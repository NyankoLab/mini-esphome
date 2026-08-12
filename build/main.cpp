// SPDX-License-Identifier: MIT
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "../Server.h"
#include "../Setup.h"

int main(int argc, const char* argv[])
{
    strcpy(ESPHome::Setup::Name, "name");
    strcpy(ESPHome::Setup::Model, "model");
    strcpy(ESPHome::Setup::Manufacturer, "nyanko");
    strcpy(ESPHome::Setup::FriendlyName, "friendly_name");
    ESPHome::Server::Start();
    ESPHome::Server::Poll();
    ESPHome::Server::Stop();
    return EXIT_SUCCESS;
}
