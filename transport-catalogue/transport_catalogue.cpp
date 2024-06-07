#include "transport_catalogue.h"

namespace transport_catalogue {
    void TransportCatalogue::AddStop(const std::string& name, const distance::Coordinates& coordinates,
                                    const std::vector<std::pair<std::string_view, double>>& distances) {
        if (ptr_stops_.find(name) != ptr_stops_.end()) {
            //ptr_stops_.find(name)->second->coordinates.lat = coordinates.lat;
            //ptr_stops_.find(name)->second->coordinates.lng = coordinates.lng;
            ptr_stops_.find(name)->second->coordinates = std::move(coordinates);
        } else {
            stops_.push_back(std::move(Stop{name, std::move(coordinates)}));
            ptr_stops_.emplace(std::string_view(stops_.back().name), &stops_.back());
        }

        /*for (const auto el : distances) {
            distances_.emplace({{}, el.second});
        }*/
    }

    const Stop* TransportCatalogue::FindStop(const std::string_view& name) const {
        auto result = ptr_stops_.find(name);
        return result == ptr_stops_.end() ? nullptr : result->second;
    }

    void TransportCatalogue::AddBus(const std::string& name, const std::vector<std::string_view>& stops) {
        Bus bus;
        bus.name = std::move(name);
        for (const std::string_view& stop : stops) {
            bus.stops.push_back(ptr_stops_.find(stop)->second);
        }
        buses_.push_back(std::move(bus));
        ptr_buses_.emplace(std::string_view(buses_.back().name), &buses_.back());

        for (const std::string_view& stop : stops) {
            buses_for_stop_[ptr_stops_.find(stop)->first].insert(&buses_.back());
        }
    }

    const Bus* TransportCatalogue::FindBus(const std::string_view& name) const {
        auto result = ptr_buses_.find(name);
        return result == ptr_buses_.end() ? nullptr : result->second;
    }

    BusInfo TransportCatalogue::GetBusInfo(const std::string_view& name) const {
        BusInfo result;

        if (ptr_buses_.find(name) != ptr_buses_.end()) {
            result.name = name;
            result.count_all_stops = ptr_buses_.at(name)->stops.size();
            std::unordered_set<std::string_view> unique_stops;
            for (const auto stop : ptr_buses_.at(name)->stops) {
                unique_stops.insert(stop->name);
            }
            result.count_unique_stops = unique_stops.size();
            result.route_length = GetBusRouteLength(name);
        }

        return result;
    }

    StopInfo TransportCatalogue::GetStopInfo(const std::string_view& name) const {
        StopInfo result;

        if (ptr_stops_.find(name) != ptr_stops_.end()) {
            result.name = name;
            if (buses_for_stop_.find(ptr_stops_.find(name)->first) != buses_for_stop_.end()) {
                for (const Bus* bus : buses_for_stop_.find(ptr_stops_.find(name)->first)->second) {
                    result.buses.insert(std::string_view(bus->name));
                }
            }
        }

        return result;
    }

    double TransportCatalogue::GetBusRouteLength(const std::string_view& name) const {
        double route_length = 0.0;
        distance::Coordinates from {-1.0, -1.0}, to {-1.0, -1.0};
        for (const auto stop : ptr_buses_.at(name)->stops) {
            if (from == distance::Coordinates {-1.0, -1.0}) {
                from = stop->coordinates;
                continue;
            }
            if (to == distance::Coordinates {-1.0, -1.0}) {
                to = stop->coordinates;
                route_length += ComputeDistance(from, to);
                continue;
            }
            
            from = to;
            to = stop->coordinates;
            route_length += ComputeDistance(from, to);
        }

        return route_length;
    }
};