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

    //Отсюда должно вызываться построение графа, которое должно быть частью роутера. 
    //FillGraphs должен быть приватным методом
    TransportRouter(const RoutingSettings& settings)
        : routing_settings_(settings) {};

    void SetSettings(const RoutingSettings& settings);

    int GetBusWaitTime() const {
        return routing_settings_.bus_wait_time;
    }

    double GetBusVelocity() const {
        return routing_settings_.bus_velocity;
    }
 
    void FillGraphs(transport_catalogue::TransportCatalogue& catalogue, graph::DirectedWeightedGraph<double>& graph);

    //Данный метод должен осуществлять поиск на графе, используя исходный роутер и принимать на входе две остановки
    //и возвращать данные пути. Ну и это скорее FindRoute, впрочем  - у вас он даже не реализован
    void FindRoute();

private:
    RoutingSettings routing_settings_;
};

};