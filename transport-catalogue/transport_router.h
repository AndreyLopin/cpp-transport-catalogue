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

    TransportRouter(transport_catalogue::TransportCatalogue& catalogue, const RoutingSettings routing_settings)
        : catalogue_(catalogue)
        , routing_settings_(routing_settings)
        , transport_graph_(graph::DirectedWeightedGraph<double>(catalogue_.GetStopsCount()))
        , transport_router_(transport_graph_) {
    };

    void SetSettings(const RoutingSettings& settings) {
        routing_settings_ = settings;
        //transport_graph_ = graph::DirectedWeightedGraph<double>(catalogue_.GetStopsCount());
        //FillGraphs(catalogue_, transport_graph_);
        //transport_router_ = graph::Router<double>(transport_graph_);
    }

    int GetBusWaitTime() const {
        return routing_settings_.bus_wait_time;
    }

    double GetBusVelocity() const {
        return routing_settings_.bus_velocity;
    }   

    std::optional<graph::Router<double>::RouteInfo> FindRoute(graph::VertexId from, graph::VertexId to);

private:
    void FillGraphs(transport_catalogue::TransportCatalogue& catalogue, graph::DirectedWeightedGraph<double>& graph);

    transport_catalogue::TransportCatalogue& catalogue_;
    RoutingSettings routing_settings_;
    graph::DirectedWeightedGraph<double> transport_graph_;
    graph::Router<double> transport_router_;
};

};