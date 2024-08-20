#include "transport_router.h"

namespace transport_router {

const int MIN_IN_HOUR = 60;
const int METERS_IN_KM = 1000;

void TransportRouter::FillGraphs(transport_catalogue::TransportCatalogue& catalogue) {
    for(const auto& bus : catalogue.GetBuses()) {
        const auto& stops = bus.stops_;
        double weight = routing_settings_.bus_wait_time * 1.0;
        auto map = catalogue.GetDistances();

        if (stops.size() > 1) {
            for (size_t i = 0; i < stops.size() - 1; ++i) {
                size_t span = 1;
                weight = routing_settings_.bus_wait_time * 1.0;
                for (size_t j = i + 1; j < stops.size(); ++j) {
                    if (stops[i] != stops[j]) {
                        auto it = map.find({ stops[j - 1], stops[j] });
                        if (it == map.end()) {
                            it = map.find({ stops[j], stops[j - 1] });
                        }

                        weight += (it->second * MIN_IN_HOUR) / (METERS_IN_KM * routing_settings_.bus_velocity);
                        graph::Edge edge(stops[i]->id, stops[j]->id, span, bus.name_, weight);
                        graph_.AddEdge(edge);
                        ++span;
                    }
                }
            }
            if (!bus.is_roundtrip_) {
                for (size_t x = stops.size() - 1; x > 0; --x) {
                    weight = routing_settings_.bus_wait_time * 1.0;
                    size_t span = 1;
                    for (size_t t = x; t > 0; --t) {
                        if (stops[x] != stops[t - 1]) {
                            auto it = map.find({ stops[t], stops[t - 1] });
                            if (it == map.end()) {
                                it = map.find({ stops[t - 1], stops[t] });
                            }

                            weight += (it->second * MIN_IN_HOUR) / (METERS_IN_KM * routing_settings_.bus_velocity);
                            graph::Edge edge(stops[x]->id, stops[t - 1]->id, span, bus.name_, weight);
                            graph_.AddEdge(edge);
                            ++span;
                        }
                    }
                }
            }
        }
    }
}

std::optional<RequestRouteInfo> TransportRouter::FindRoute(domain::Stop* from, domain::Stop* to) {
    const auto route_info = router_.graph::Router<double>::BuildRoute(from->id, to->id);
    if(route_info.has_value()) {
        std::vector<RoutePoint> route_points;

        const auto& elem = route_info.value().edges;

        for (const auto& el : elem) {
            const auto& edge = router_.GetGraph().GetEdge(el);
            route_points.emplace_back(RoutePoint{catalogue_.FindStop(catalogue_.GetAllStops()[edge.from].name),
                                            static_cast<int>(edge.span_count),
                                            edge.bus,
                                            edge.weight - GetBusWaitTime()});            
        }
        return RequestRouteInfo{route_info.value().weight, route_points};
    } else {
        return std::nullopt;
    }
}

const graph::DirectedWeightedGraph<double>& TransportRouter::GetGraph() const {
    return graph_;
}

};