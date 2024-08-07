#pragma once

#include "router.h"

namespace transport_router {

struct RoutingSettings {
    int bus_wait_time;
    double bus_velocity;
};

class Router {
public:
    Router() = default;

    Router(const RoutingSettings& settings)
        : routing_settings_(settings) {};

    void SetSettings(const RoutingSettings& settings);

    void BuildRoute();

private:
    RoutingSettings routing_settings_;
};

};