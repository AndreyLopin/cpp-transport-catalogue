// Вставьте сюда решение из предыдущего спринта
#include "transport_router.h"

namespace transport_router {

void Router::SetSettings(const RoutingSettings& settings) {
    routing_settings_ = settings;
}

double Router::FormulaForFindTime(double distance, double weight_time, double velocity)
{
    double distance_between_two_stops_in_km = distance / 1000.0;
    double time_in_hour = distance_between_two_stops_in_km / velocity;
    double min_in_hour = 60.0;
    return weight_time += time_in_hour * min_in_hour;
}

void Router::ConstructGraph(graph::DirectedWeightedGraph<double>& graph, double distance, double& weight, double velocity, size_t a, size_t b,
        size_t& span, const std::vector<domain::Stop*>& stops, std::string bus_num) {

    weight = FormulaForFindTime(distance, weight, velocity);
    graph::Edge edge(stops[a]->id, stops[b]->id, span, bus_num, weight);
    graph.AddEdge(edge);
    ++span;
}

void Router::FillGraphs(transport_catalogue::TransportCatalogue& catalogue, graph::DirectedWeightedGraph<double>& graph) {
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
                            ConstructGraph(graph, it->second, weight, routing_settings_.bus_velocity, i, j, span, stops, bus.name_);
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
                                ConstructGraph(graph, it->second, weight, routing_settings_.bus_velocity, x, t - 1, span, stops, bus.name_);
                            }
                        }
                    }
                }
            }
    }
}

};