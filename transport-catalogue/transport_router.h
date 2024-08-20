#pragma once

#include "router.h"
#include "transport_catalogue.h"

namespace transport_router {

struct RoutingSettings {
    int bus_wait_time;
    double bus_velocity;
};

class TransportRouter {
public:
    TransportRouter() = default;

    TransportRouter(const RoutingSettings settings, transport_catalogue::TransportCatalogue& catalogue)
        : routing_settings_(settings)
        , graph_(graph::DirectedWeightedGraph<double>(catalogue.GetStopsCount()))
        , router_(graph_) {
            FillGraphs(catalogue);
            router_.Initialize();
    }

    void SetSettings(const RoutingSettings& settings) {
        routing_settings_ = settings;
    }

    int GetBusWaitTime() const {
        return routing_settings_.bus_wait_time;
    }

    double GetBusVelocity() const {
        return routing_settings_.bus_velocity;
    }
 
    void FillGraphs(transport_catalogue::TransportCatalogue& catalogue);

    std::optional<graph::Router<double>::RouteInfo> FindRoute(graph::VertexId from, graph::VertexId to);

    const graph::DirectedWeightedGraph<double>& GetGraph() const;

private:
    RoutingSettings routing_settings_;
    graph::DirectedWeightedGraph<double> graph_;
    graph::Router<double> router_;
};

};