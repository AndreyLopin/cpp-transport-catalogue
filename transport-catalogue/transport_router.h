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

    double FormulaForFindTime(double distance, double weight_time, double velocity);

    void ConstructGraph(graph::DirectedWeightedGraph<double>& graph, double distance, double& weight, double velocity,
                        size_t a, size_t b, size_t& span, const std::vector<domain::Stop*>& stops, std::string bus_num);
 
    void FillGraphs(transport_catalogue::TransportCatalogue& catalogue, graph::DirectedWeightedGraph<double>& graph);

    void BuildRoute();

private:
    RoutingSettings routing_settings_;
};

};