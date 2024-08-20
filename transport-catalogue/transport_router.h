#pragma once

#include "router.h"
#include "transport_catalogue.h"

namespace transport_router {

struct RoutingSettings {
    int bus_wait_time;
    double bus_velocity;
};

struct RoutePoint {
    domain::Stop* from;
    int span_count;
    std::string bus;
    double wait_time;
};

struct RequestRouteInfo {
    double duration;

    std::vector<RoutePoint> route_points;
};

class TransportRouter {
public:
    TransportRouter() = default;

    TransportRouter(const RoutingSettings settings, transport_catalogue::TransportCatalogue& catalogue)
        : routing_settings_(settings)
        , graph_(graph::DirectedWeightedGraph<double>(catalogue.GetStopsCount()))
        , router_(graph_)
        , catalogue_(catalogue) {
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

    std::optional<transport_router::RequestRouteInfo> FindRoute(domain::Stop* from, domain::Stop* to);

private:
    RoutingSettings routing_settings_;
    graph::DirectedWeightedGraph<double> graph_;
    graph::Router<double> router_;
    transport_catalogue::TransportCatalogue& catalogue_;

    void FillGraphs(transport_catalogue::TransportCatalogue& catalogue);
    const graph::DirectedWeightedGraph<double>& GetGraph() const;
};

};