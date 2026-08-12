// SPDX-License-Identifier: MIT
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "../API.h"
#include "../Server.h"
#include "../Setup.h"

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

int main(int argc, const char* argv[])
{
    strcpy(ESPHome::Setup::Name, "name");
    strcpy(ESPHome::Setup::Model, "model");
    strcpy(ESPHome::Setup::Manufacturer, "nyanko");
    strcpy(ESPHome::Setup::FriendlyName, "friendly_name");
    ESPHome::Server::Start(Dispatch);
    ESPHome::Server::Poll();
    ESPHome::Server::Stop();
    return EXIT_SUCCESS;
}
