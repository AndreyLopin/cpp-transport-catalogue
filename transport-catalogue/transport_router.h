#pragma once

#include "router.h"
#include "transport_catalogue.h"

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

    int GetBusWaitTime() const {
        return routing_settings_.bus_wait_time;
    }

    double GetBusVelocity() const {
        return routing_settings_.bus_velocity;
    }
 
    void FillGraphs(transport_catalogue::TransportCatalogue& catalogue, graph::DirectedWeightedGraph<double>& graph);

    void BuildRoute();

private:
    RoutingSettings routing_settings_;
};

};